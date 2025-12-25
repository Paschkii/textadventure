#!/usr/bin/env zsh
set -euo pipefail

APP_NAME="Glandular"
TARGET="Glandular"
ARCH="${ARCH_BUILD:-arm64}"
BUILD_DIR="build-release-${ARCH}"
ICLOUD_ROOT="$HOME/Library/Mobile Documents/com~apple~CloudDocs/GlandularBuilds"
DIST_DIR_ROOT="${DIST_DIR_OVERRIDE:-dist}"
DIST_DIR="${DIST_DIR_ROOT}/mac/arm64"
VERSION="${1:-0.1.0}"

mkdir -p "$DIST_DIR"

BREW_PREFIX="$(brew --prefix)"

APP_BUNDLE="$DIST_DIR/${APP_NAME}.app"
EXEC_PATH="$APP_BUNDLE/Contents/MacOS/$TARGET"
FW_DIR="$APP_BUNDLE/Contents/Frameworks"
RES_DIR="$APP_BUNDLE/Contents/Resources"

echo "== Clean =="
rm -rf "$BUILD_DIR" "$DIST_DIR"
mkdir -p "$DIST_DIR"

echo "== Configure (Release, ${ARCH}) =="
cmake -S . -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="${ARCH}" \
  -DCMAKE_PREFIX_PATH="$(brew --prefix sfml)"

echo "== Build =="
cmake --build "$BUILD_DIR" -j

echo "== Create .app bundle =="
mkdir -p "$APP_BUNDLE/Contents/MacOS" "$FW_DIR" "$RES_DIR"

cp "$BUILD_DIR/$TARGET" "$EXEC_PATH"
ln -s "../Resources/assets" "$APP_BUNDLE/Contents/MacOS/assets" 2>/dev/null || true
cp -R "assets" "$RES_DIR/assets"

# Icon (optional)
if [[ -f "assets/icon/Glandular.icns" ]]; then
  cp "assets/icon/Glandular.icns" "$RES_DIR/Glandular.icns"
fi

cat > "$APP_BUNDLE/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleName</key><string>${APP_NAME}</string>
  <key>CFBundleIdentifier</key><string>com.pascal.${APP_NAME:l}</string>
  <key>CFBundleVersion</key><string>${VERSION}</string>
  <key>CFBundleShortVersionString</key><string>${VERSION}</string>
  <key>CFBundleExecutable</key><string>${TARGET}</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>CFBundleIconFile</key><string>Glandular</string>
</dict>
</plist>
PLIST

echo "== Copy SFML dylibs (from otool) =="

# 1) Alle direkt benötigten SFML libs aus dem Binary auslesen (absolute Pfade)
SFML_ABS=("${(@f)$(otool -L "$EXEC_PATH" | awk '{print $1}' | grep "^/opt/homebrew/opt/sfml/lib/libsfml-.*\\.dylib$" || true)}")

if [[ ${#SFML_ABS[@]} -eq 0 ]]; then
  echo "ERROR: No SFML dylibs detected via otool."
  echo "Check: otool -L \"$EXEC_PATH\""
  exit 1
fi

SFML_FILES=()
for abs in "${SFML_ABS[@]}"; do
  base="${abs:t}"
  echo "  -> $base"
  cp "$abs" "$FW_DIR/$base"
  SFML_FILES+=("$base")
done

echo "== Fix rpath + relink SFML dylibs =="
install_name_tool -add_rpath "@executable_path/../Frameworks" "$EXEC_PATH" 2>/dev/null || true

for abs in "${SFML_ABS[@]}"; do
  base="${abs:t}"
  install_name_tool -change "$abs" "@rpath/$base" "$EXEC_PATH" 2>/dev/null || true
done

echo "== Bundle additional Homebrew dylib deps (auto) =="
bundle_one() {
  local bin="$1"
  local deps
  deps=$(otool -L "$bin" | awk '{print $1}' | tail -n +2 | grep "^$BREW_PREFIX" || true)

  for dep in ${(f)deps}; do
    local base="${dep:t}"
    if [[ ! -f "$FW_DIR/$base" ]]; then
      cp "$dep" "$FW_DIR/$base"
      chmod 644 "$FW_DIR/$base"
    fi
    install_name_tool -change "$dep" "@rpath/$base" "$bin" 2>/dev/null || true
  done
}

for i in {1..10}; do
  bundle_one "$EXEC_PATH" || true
  local any=0
  for f in "$FW_DIR"/*.dylib(.N); do
    bundle_one "$f" && any=1 || true
    install_name_tool -add_rpath "@loader_path" "$f" 2>/dev/null || true
  done
  [[ $any -eq 0 ]] && break
done

echo "== Ad-hoc codesign =="
codesign --force --deep --sign - "$APP_BUNDLE" >/dev/null 2>&1 || true

echo "== Zip for distribution =="
ZIP_NAME="${APP_NAME}-macOS-${ARCH}-${VERSION}.zip"
ditto -c -k --sequesterRsrc --keepParent "$APP_BUNDLE" "$DIST_DIR/$ZIP_NAME"

echo "DONE: $DIST_DIR/$ZIP_NAME"
echo "Test: open \"$APP_BUNDLE\""

echo "== Create DMG =="
DMG_NAME="${APP_NAME}-macOS-${ARCH}-${VERSION}.dmg"
STAGE_DIR="$DIST_DIR/dmg-stage"

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR"

cp -R "$APP_BUNDLE" "$STAGE_DIR/"

# Optional: Applications-Link (klassischer macOS-Install-Flow)
ln -s /Applications "$STAGE_DIR/Applications"

hdiutil create -volname "$APP_NAME" \
  -srcfolder "$STAGE_DIR" \
  -ov -format UDZO \
  "$DIST_DIR/$DMG_NAME"

rm -rf "$STAGE_DIR"

echo "DONE: $DIST_DIR/$DMG_NAME"

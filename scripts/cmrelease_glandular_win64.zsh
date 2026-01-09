#!/usr/bin/env zsh
set -euo pipefail

APP_NAME="Glandular"
TARGET="Glandular"
ARCH="win64"
BUILD_DIR="build-release-${ARCH}"

SCRIPT_DIR="$(cd -- "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"

ICLOUD_ROOT="$HOME/Library/Mobile Documents/com~apple~CloudDocs/GlandularBuilds"
DIST_DIR_ROOT="${DIST_DIR_OVERRIDE:-$ICLOUD_ROOT}"
DIST_DIR="${DIST_DIR_ROOT}/windows/x86_64"
VERSION="${1:-1.0.0}"

MINGW_TRIPLE="${MINGW_TRIPLE:-x86_64-w64-mingw32}"
MINGW_TOOLCHAIN_ROOT="${MINGW_TOOLCHAIN_ROOT:-/opt/homebrew/opt/mingw-w64/toolchain-x86_64}"
MINGW_SYSROOT="${MINGW_SYSROOT:-${MINGW_TOOLCHAIN_ROOT}/${MINGW_TRIPLE}}"
MINGW_BIN_DIR="${MINGW_BIN_DIR:-${MINGW_TOOLCHAIN_ROOT}/bin}"

if [[ ! -d "$MINGW_BIN_DIR" && -d "$MINGW_SYSROOT/bin" ]]; then
  MINGW_BIN_DIR="$MINGW_SYSROOT/bin"
fi

SFML_WIN_PREFIX_DEFAULT="/Users/pascalscholz/Documents/Coding/Github/C++/sfmltest/textadventure/third_party/SFML-3.0.2"
SFML_WIN_PREFIX="${SFML_WIN_PREFIX:-$SFML_WIN_PREFIX_DEFAULT}"
SFML_DIR="${SFML_WIN_PREFIX}/lib/cmake/SFML"

if [[ ! -d "$SFML_WIN_PREFIX" ]]; then
  echo "ERROR: SFML_WIN_PREFIX not found: $SFML_WIN_PREFIX"
  exit 1
fi

for dylib in "$SFML_WIN_PREFIX"/lib/libsfml-*.dylib(N); do
  echo "ERROR: SFML_WIN_PREFIX points to a macOS SFML install: $SFML_WIN_PREFIX"
  echo "       For Windows cross-compile, use a Windows SFML prefix with bin/sfml-*.dll and lib/*.a."
  exit 1
done

if [[ ! -f "$SFML_DIR/SFMLConfig.cmake" ]]; then
  echo "ERROR: SFMLConfig.cmake not found: $SFML_DIR/SFMLConfig.cmake"
  exit 1
fi

SFML_WIN_BIN="${SFML_WIN_PREFIX}/bin"

mkdir -p "$DIST_DIR"

echo "== Clean =="
rm -rf "$BUILD_DIR" "$DIST_DIR"
mkdir -p "$DIST_DIR"

export MINGW_TRIPLE
export MINGW_SYSROOT

echo "== Configure (Release, Windows x86_64) =="
cd "$PROJECT_ROOT"
cmake -S . -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$SCRIPT_DIR/toolchain-mingw64.cmake" \
  -DCMAKE_PREFIX_PATH="$SFML_WIN_PREFIX" \
  -DSFML_DIR="$SFML_DIR"

echo "== Build =="
cmake --build "$BUILD_DIR" -j

echo "== Stage =="
STAGE_DIR="$DIST_DIR/${APP_NAME}-windows-${ARCH}-${VERSION}"
rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR"

cp "$BUILD_DIR/$TARGET.exe" "$STAGE_DIR/"

cmake -DSOURCE_DIR="$PROJECT_ROOT" -DDEST_DIR="$STAGE_DIR" -P "$SCRIPT_DIR/copy_assets_with_rankings.cmake"

echo "== Copy SFML DLLs =="
if [[ -d "$SFML_WIN_BIN" ]]; then
  SFML_DLLS=("$SFML_WIN_BIN"/sfml-*.dll(N))
  if [[ ${#SFML_DLLS[@]} -eq 0 ]]; then
    echo "ERROR: No SFML DLLs found in $SFML_WIN_BIN"
    exit 1
  fi
  cp "$SFML_WIN_BIN"/sfml-*.dll "$STAGE_DIR/"
  for extra in openal32.dll freetype.dll ogg.dll vorbis.dll vorbisfile.dll; do
    if [[ -f "$SFML_WIN_BIN/$extra" ]]; then
      cp "$SFML_WIN_BIN/$extra" "$STAGE_DIR/"
    fi
  done
else
  echo "ERROR: SFML bin folder not found: $SFML_WIN_BIN"
  exit 1
fi

echo "== Copy MinGW runtime DLLs (if found) =="
if [[ -d "$MINGW_BIN_DIR" ]]; then
  missing=()
  for dll in libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll; do
    if [[ -f "$MINGW_BIN_DIR/$dll" ]]; then
      cp "$MINGW_BIN_DIR/$dll" "$STAGE_DIR/"
    else
      missing+=("$dll")
    fi
  done
  if [[ ${#missing[@]} -gt 0 ]]; then
    echo "WARN: Missing MinGW runtime DLLs in $MINGW_BIN_DIR: ${missing[*]}"
  fi
else
  echo "WARN: MINGW_BIN_DIR not found: $MINGW_BIN_DIR"
fi

echo "== Zip for distribution =="
ZIP_NAME="${APP_NAME}-Windows-x86_64-${VERSION}.zip"
if command -v zip >/dev/null; then
  (cd "$DIST_DIR" && zip -r -X "$ZIP_NAME" "$(basename "$STAGE_DIR")")
else
  ditto -c -k --keepParent "$STAGE_DIR" "$DIST_DIR/$ZIP_NAME"
fi
echo "DONE: $DIST_DIR/$ZIP_NAME"

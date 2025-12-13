# PROJECT OVERVIEW

This document explains the high-level structure of the `textadventure` project, what each major module does, and where important functions are implemented. Use this as a reference for navigation or for adding inline comments later.

---

## Quick start
- Build & run (from workspace root):

```bash
cmrun textadventure
```

- Core language: C++ (C++20), uses SFML for graphics & audio.
- Build system: CMake (project root contains CMakeLists.txt).

---

## Top-level folders (short)
- `assets/` — art, fonts, audio, textures used by the game.
- `src/` — main source code split into subfolders:
  - `core/` — central game state and main loop.
  - `ui/` — UI drawing and input handling.
  - `rendering/` — helpers for effects, layout and rendering utilities.
  - `story/` — dialogue data, input handling for dialogue logic and text styles.
  - `resources/` — loading textures and sound buffers.
- `build/` — CMake build output.

---

## Important modules and purpose

### `core/`
- `game.hpp` / `game.cpp`
  - Purpose: central `Game` struct (holds `sf::RenderWindow`, `Resources`, UI rectangles, audio `sf::Sound`, dialogue state, clocks, etc.), layout and run loop.
  - Key functions:
    - `Game::Game()` — constructor, resource loading and initialization (implemented in `src/core/game.cpp`).
    - `Game::updateLayout()` — recomputes positions/sizes for UI elements when the window size changes (implemented in `src/core/game.cpp`).
    - `Game::run()` — main event loop and render cycle (implemented in `src/core/game.cpp`).
    - `startTypingSound()` / `stopTypingSound()` — manage typewriter sound (implemented in `src/core/game.cpp`).

### `ui/`
This folder contains the user interface drawing and input code.

- `dialogUI.cpp` / `dialogUI.hpp`
  - Purpose: draws the main dialogue UI including name/text boxes, weapon/dragon showcase and location box; handles typewriter updates for visible text.
  - Key functions (implemented in `src/ui/dialogUI.cpp`):
    - `drawDialogueUI(Game&, sf::RenderTarget&)` — draws name box, text box, and manages drawing order for dragon showcase and location box.
    - `drawDragonShowcase(Game&, sf::RenderTarget&, float uiAlphaFactor, float glowElapsedSeconds)` — draws the dragon portraits, labels and the weaponPanel frame.
    - `drawLocationBox(...)` — draws the currently selected location name.

- `dialogDrawElements.cpp` / `dialogDrawElements.hpp`
  - Purpose: small helpers to render parts of the dialogue UI: frames, name text, typed text segments, return prompt, name input cursor.
  - Key functions (implemented in `src/ui/dialogDrawElements.cpp`):
    - `dialogDraw::drawDialogueFrames(...)`
    - `dialogDraw::drawSpeakerName(...)`
    - `dialogDraw::drawDialogueText(...)` and `drawNameInput(...)`

- `confirmationUI.cpp` / `confirmationUI.hpp`
  - Purpose: show Yes/No confirmation prompts and handle input (mouse-only by recent change).
  - Key functions (implemented in `src/ui/confirmationUI.cpp`): `showConfirmationPrompt`, `drawConfirmationPrompt`, `handleConfirmationEvent`.

- `mapSelectionUI.cpp` / `mapSelectionUI.hpp`
  - Purpose: renders the map background and location icons (Lacrimere, Gonad, Blyathyroid, Aerobronchi, Cladrenal). Handles hover states and sepia↔color swap.
  - Key functions (implemented in `src/ui/mapSelectionUI.cpp`): `drawMapSelectionUI(Game&, sf::RenderTarget&)` (returns hover popup data) and `drawMapSelectionPopup(Game&, sf::RenderTarget&, const MapPopupRenderData&)`.

- `weaponSelectionUI.cpp` / `weaponSelectionUI.hpp`
  - Purpose: weapon selection screen and drawing the weapon/portrait panel frame (separate from dialogue UI). Also contains logic to start dragon dialogue.
  - Key functions: `drawWeaponSelectionUI(...)`, `handleWeaponSelectionEvent(...)` (implemented in `src/ui/weaponSelectionUI.cpp`).

- `introTitle.cpp` / `introTitle.hpp`
  - Purpose: shows the splash/title drop animation and the "Press Enter to Continue." prompt; controls intro -> UI fade transitions.
  - Key functions (implemented in `src/ui/introTitle.cpp`): `drawIntroTitle(Game&, sf::RenderTarget&)`, `triggerIntroTitleExit(Game&)`.

- `uiEffects.*`
  - Purpose: helper functions for glow frames, color computations and other visual effects. Implementations in `src/ui/uiEffects.*`.

- `uiVisibility.*`
  - Purpose: compute whether UI elements (name/text boxes, intro title, weapon panel, etc.) should be visible and their alpha fade factors. Implemented in `src/ui/uiVisibility.cpp`.


### `story/`
- `storyIntro.hpp` / `storyIntro.cpp` (dialogue data)
  - Purpose: contains static vectors of `DialogueLine` (intro, dragon, destination, etc.) used by the game.
- `dialogueLine.hpp`
  - Purpose: `DialogueLine` struct: `speaker` (name/SpeakerId), `text`, and flags.
- `dialogInput.hpp` / `dialogInput.cpp`
  - Purpose: logic for advancing dialogue, handling Enter key, name input and transitions (e.g., from dragon dialogue to map selection). Implemented in `src/story/dialogInput.hpp`/`.cpp`.
- `textStyles.hpp` / `textStyles.cpp`
  - Purpose: mapping from speaker names to `SpeakerId` and `SpeakerStyle` (display name + color). Implemented in `src/story/textStyles.cpp`.

### `rendering/`
- `dialogRender.hpp` / `dialogRender.cpp`
  - Purpose: high-level rendering dispatcher; picks which UI/state to render based on `GameState`.
- `textLayout.*`, `colorHelper.*`, `locations.*`
  - Purpose: helpers for layout, color palettes and location definitions.

### `resources/`
- `resources.cpp` / `resources.hpp`
  - Purpose: load textures, fonts and sound buffers from `assets/`. Provides `Resources` object referenced by `Game::resources`.

---

## Notable global types / state
- `Game` (in `src/core/game.hpp`) — central struct holding window, resources, dialogue state, UI boxes (`nameBox`, `textBox`, `weaponPanel`, `locationBox`), sounds (`enterSound`, `confirmSound`, `rejectSound`, etc.), clocks and other flags.
- `DialogueLine` (in `src/story/dialogueLine.hpp`) — represents a single line of the story: `speaker`, `text`, optional flags.
- `Resources` (in `src/resources/resources.hpp`) — central place to get textures/fonts/sounds like `fireDragon`, `enterKey`, `introBackground`.

---

## Where to add inline comments
If you want inline comments added to source files I recommend a phased approach:
1. Produce and review this `docs/PROJECT_OVERVIEW.md` (non-invasive).
2. Add inline module headers to the top of each major source/header file (1–3 lines) describing purpose and where the key functions are implemented.
3. Add function-level brief doc-comments for public functions and for complex private functions (single-line description + implementation file location if the declaration is in a header).

I can start by adding comments to a single module (e.g., `core/game.hpp` + `core/game.cpp`) and iterate, or apply the same pattern project-wide.

---

## Example function mapping (quick reference)
- `Game::run()` — implemented: `src/core/game.cpp`
- `Game::updateLayout()` — implemented: `src/core/game.cpp`
- `drawDialogueUI(...)` — implemented: `src/ui/dialogUI.cpp`
- `drawDragonShowcase(...)` — implemented: `src/ui/dialogUI.cpp`
- `drawMapSelectionUI(...)` / `drawMapSelectionPopup(...)` — implemented: `src/ui/mapSelectionUI.cpp`
- `drawWeaponSelectionUI(...)` — implemented: `src/ui/weaponSelectionUI.cpp`
- `drawIntroTitle(...)` — implemented: `src/ui/introTitle.cpp`
- `speakerStyle(...)` — implemented: `src/story/textStyles.cpp`
- `computeUiVisibility(...)` — implemented: `src/ui/uiVisibility.cpp`
- `showConfirmationPrompt(...)` / `handleConfirmationEvent(...)` — implemented: `src/ui/confirmationUI.cpp`
- Resource loading — `src/resources/resources.cpp`

---

<!-- Copilot instructions for FlippeRPG repository -->
# Copilot / AI Agent Instructions (concise)

Purpose: Give an AI agent the minimum context to be productive in FlippeRPG (Flipper Zero app) with build steps, layout, and patterns.

- **Project overview**: FlippeRPG is a signal-driven RPG for Flipper Zero; players act as Signal Mages absorbing real-world signals (IR, NFC, SubGHz, GPIO) for power. The Codex tracks progress, shrines unlock techniques, and Flipper-to-Flipper encounters happen at the Campfire. Single-player shrine progression + multiplayer campfire hub with dueling, echo exchange, and auto-handshake XP (MVP complete).

- **Core features (MVP)**:
  - Signal absorption maps IR/NFC/SubGHz/GPIO to elements; XP uses dedup (unique signals grant more XP).
  - Shrines unlock techniques (e.g., Pulse Open, Flame Reach, Bind Whisper) and track progress. 70% success chance on rituals.
  - Duels with separate duel XP (60% win = +15 XP, loss = +5 XP), win/loss tracking.
  - Echoes track signal fragments; exchange available at campfire.
  - **Campfire Hub**: Visual N/S/E/W scene, auto-handshake +2 XP, duel/echo exchange per player.
  - Save/load locally via filesystem (NFC/SubGHz integration pending).

- **Project layout (app)**: `applications_user/FlippeRPG/`
  - `FlippeRPG.c` main entry + all UI views (signal, shrine list/detail, campfire, profile, status, codex).
  - `ASSET_CREATION.md` guide for sprite design in Aseprite (fire animation, player icons, pop-in/fade, selection glow).
  - `codex/` (state, XP, signal history, encounters), `signal/` (engine, fusion), `shrine/` (logic, definitions), `xp/`, `duel/`, `echo/`, `save/`, `core/`, `techniques/`, `narrative/`, `data/`, `assets/`, `tests/`.
  - Encounter log pre-populated in `init_codex()` with test data (3 example players for demo).

- **Current UI Views** (8 screens, ViewDispatcher managed):
  - VIEW_MENU: Main menu (Absorb Signals, Shrines, Techniques, Multiplayer, Status, Exit).
  - VIEW_SIGNAL: Signal absorption screen (OK to scan + 5-10 XP).
  - VIEW_SHRINE_LIST: Browse 5 shrines with completion % visible.
  - VIEW_SHRINE_DETAIL: Individual shrine with ritual description and attempt button.
  - VIEW_TECHNIQUE: Unlocked techniques and mastery tracking.
  - VIEW_CODEX: Player stats (signal_xp, duel_xp, level, auras, shrines completed).
  - VIEW_CAMPFIRE: Multiplayer hub—N/S/E/W positions, fire center, arrow keys to navigate, OK to view player.
  - VIEW_CAMPFIRE_PROFILE: Player card showing name, aura, +2 handshake XP, L key to duel (60% win), R key to echo.

- **Build system**: SCons via `SConstruct`; helper wrappers `fbt`/`fbt.cmd`.
  - Build this app (fap): `./fbt fap_flipperpg` (macOS/Linux) or `.\fbt.cmd fap_flipperpg` (Windows).
  - Deploy to device: `.fbt.cmd fap_deploy` or `scons fap_deploy`.

- **SCons conventions**:
  - Each component has a `SConscript`; keep build logic there so `SConstruct` sees targets.
  - Use existing aliases (`flash`, `fap_dist`, `lint`, `format`) instead of ad-hoc scripts.

- **Patterns & code style**:
  - C apps follow `applications_user/<app>/` layout with per-app metadata.
  - View pattern: static views + callbacks for draw/input, registered with ViewDispatcher.
  - Scene state stored in statics: `campfire_slots[MAX_CAMP_SLOTS]`, `selected_camp_slot`, etc.
  - Game state in global `player_codex` struct (initialized via `init_codex()`, saved via `save_codex()`).
  - Encounters pre-populated from `encounter_log[]` on campfire entry for demo realism.

- **Integration & deployment**:
  - `fap_deploy` handles app packaging/push to device.
  - Compiled FAP placed in `build/f7-firmware-D/.extapps/`.

- **Asset system (Sprite-Ready)**:
  - Currently: ASCII/text-based rendering (fire = `[*]`, players = text names).
  - Code prepared for PNG sprite swaps in `campfire_draw_callback()`.
  - Assets folder: `applications_user/FlippeRPG/assets/` ready for `campfire_fire.png`, player icons, etc.
  - Animation frame cycling ready (sprite sheet or individual PNGs per frame).
  - **See `ASSET_CREATION.md` for full Aseprite workflow, frame specs, and code integration.**

- **Where to start when changing behavior**:
  - **Add UI screen**: Edit `FlippeRPG.c`, create view + callbacks (draw/input), register with dispatcher.
  - **Design sprites**: Follow `ASSET_CREATION.md` step-by-step in Aseprite, export PNG to `assets/`, minimal code changes to load/render.
  - **Modify game systems**: Edit relevant subdirs (`shrine/`, `signal/`, `xp/`, `duel/`, `echo/`, `codex/`) and hook calls in `FlippeRPG.c`.
  - **Test locally**: Use `.fbt.cmd fap_flipperpg` to rebuild, deploy with `.fbt.cmd fap_deploy`, validate on device.

- **Helpful files**: `FlippeRPG.c` (all views/callbacks), `codex/codex.h` (Codex struct, encounter_log), `shrine/shrine.h` (shrine definitions), `core/constants.h` (enums: ShrineID, SignalType, Aura), `ASSET_CREATION.md` (sprite design guide), `README.md` (feature overview & gameplay flows).

If anything is unclear (sprite integration, new shrine types, timeout mechanics, SubGHz scanning), say so and I will expand guidance.

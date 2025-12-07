<!-- Copilot instructions for FlippeRPG repository -->
# Copilot / AI Agent Instructions (concise)

Purpose: Give an AI agent the minimum context to be productive in FlippeRPG (Flipper Zero app) with build steps, layout, and patterns.

- **Project overview**: FlippeRPG is a signal-driven RPG for Flipper Zero; players act as Signal Mages absorbing real-world signals (IR, NFC, SubGHz, GPIO) for power. The Codex tracks progress, shrines unlock techniques, and Flipper-to-Flipper encounters create echoes.

- **Core features**:
  - Signal absorption maps IR/NFC/SubGHz/GPIO to elements; XP uses dedup (unique signals grant more XP).
  - Shrines unlock techniques (e.g., Pulse Open, Flame Reach, Bind Whisper) and track progress.
  - Duels with separate duel XP, win/loss tracking, and synergy bonuses.
  - Echoes share memory fragments between devices via SubGHz/NFC.
  - Save/load locally, via NFC tag (Codex Totem), or SubGHz sync.

- **Project layout (app)**: `applications_user/FlippeRPG/`
  - `FlippeRPG.c` entry point; `manifest.json` / `application.fam` define metadata.
  - `codex/` (state, XP, signal history), `signal/` (signal engine, hashing, encounters), `shrine/` (logic, definitions), `xp/`, `duel/`, `echo/`, `save/`, `core/` (constants/utils), `techniques/`, `narrative/`, `data/`, `assets/`, `tests/`.

- **Build system**: SCons via `SConstruct`; helper wrappers `fbt`/`fbt.cmd`.
  - Build firmware: `scons` (repo root); flash: `scons flash`; distributable: `scons fw_dist`.
  - Build this app (fap): `./fbt fap_flipperpg` (macOS/Linux) or `.bt.cmd fap_flipperpg` (Windows).
  - Deploy compiled apps to device: `scons fap_deploy` (uses `scripts/storage.py`).

- **SCons conventions**:
  - Each component has a `SConscript`; keep build logic there so `SConstruct` sees targets.
  - Use existing aliases (`flash`, `fap_dist`, `lint`, `format`) instead of ad-hoc scripts.

- **Patterns & code style**:
  - C apps follow `applications[_user]/<app>/` layout with per-app `SConscript`/metadata.
  - Python tooling uses `black`; lints via `scons lint_py` / `scons format_py`.
  - Debug helpers: `scons debug`, `scons openocd`, `scons blackmagic` (see `SConstruct`).

- **Integration & deployment**:
  - `fap_dist` and `fap_deploy` handle external app packaging/push; see `SConstruct` and `scripts/storage.py`.
  - Updater/self-update flows live in `dist` targets guarded by `fullenv`.

- **Where to start when changing behavior**:
  - Build/tooling: `SConstruct`, `site_scons/environ.scons`, `site_scons/cc.scons`.
  - App logic: `applications_user/FlippeRPG/` subdirs listed above; entry `FlippeRPG.c`.
  - Core runtime/platform: `furi/`, `lib/` and their `SConscript` files.

- **Helpful files to read**: `SConstruct`; `site_scons/environ.scons`; `site_scons/commandline.scons`; `applications_user/FlippeRPG/`; `scripts/` (e.g., `flash.py`, `storage.py`, `lint.py`); `.github/workflows/*` for CI steps.

If anything is unclear or missing (e.g., preferred `fbt` flags, adding a new shrine/technique flow, or a sample `SConscript`), say so and I will tighten the guide.

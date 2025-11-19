<!-- Copilot instructions for FlippeRPG repository -->
# Copilot / AI Agent Instructions (concise)

Purpose: Help an AI coding agent be productive in this repository by explaining the build system, key directories, conventions, and common developer workflows with short examples.

- **Build system**: The repo uses SCons via `SConstruct` at the repo root. Common entry points:
  - Build firmware: run `scons` from the repository root (see `SConstruct`).
  - Flash device: `scons flash` (alias defined in `SConstruct`).
  - Create distributable: `scons fw_dist` or use the `dist` aliases described in `SConstruct`.
  - Python helper wrapper: `fbt` / `fbt.cmd` exist at the repo root for convenience—prefer those for common flows if available.

- **Where code lives**:
  - Application source: `applications/` (official) and `applications_user/` (user apps). Example app: `applications_user/FlippeRPG/shrine/shrine.c`.
  - Core framework: `furi/` and `lib/` (platform libraries & third-party code).
  - Hardware/targets: `targets/` and `toolchain/` (cross compiler/toolchain config).
  - Build helpers & scripts: `site_scons/` and `scripts/` (SCons extensions, flashing, linting tools).

- **SCons conventions**:
  - Each component/directory usually has a `SConscript` that registers build targets with the root `SConstruct`.
  - Do not move build logic into random scripts—use `site_scons/` and directory `SConscript` files so the root SCons graph can see targets.
  - Use `distenv` targets (aliases such as `flash`, `fap_dist`, `lint`, `format`) rather than inventing bespoke commands.

- **Patterns & code style**:
  - C apps follow the `applications[_user]/<app>/` layout with a dedicated directory per app and an `SConscript` to build it.
  - Python tooling uses `black` and repo lint scripts. There are SCons aliases for `lint_py` and `format_py`—call them via `scons lint_py`/`scons format_py`.
  - Hardware debugging targets are exposed as SCons phony targets: e.g. `scons debug`, `scons openocd`, `scons blackmagic`.

- **Integration & deployment**:
  - External app packaging and deployment are handled by `fap_dist` and `fap_deploy` (see `SConstruct` and `dist` env). `fap_deploy` uses `scripts/storage.py` to push apps to a connected device.
  - Updater and self-update flows are implemented under `dist` targets; look at `SConstruct` sections guarded by `fullenv`.

- **Where to look first when changing behavior**:
  - If changing build behavior: `SConstruct`, `site_scons/environ.scons`, `site_scons/cc.scons`.
  - If adding an app: mirror other `applications_user/<name>/` apps and add an `SConscript` in that directory.
  - If modifying core runtime: inspect `furi/` and `lib/` folders and their SConscript files.

- **Examples (copyable)**
  - Build firmware: `scons` (run in repo root).
  - Flash: `scons flash`.
  - Run Python linter: `scons lint_py`.
  - Deploy compiled apps to device: `scons fap_deploy` (uses `scripts/storage.py`).

- **Helpful files to read before coding**
  - `SConstruct` (root build graph and aliases)
  - `site_scons/environ.scons` and `site_scons/commandline.scons` (flags & toolchain setup)
  - `applications_user/FlippeRPG/` (example user app layout)
  - `scripts/` (helper utilities such as `flash.py`, `storage.py`, `lint.py`)
  - `.github/workflows/*` (CI steps and test matrix to replicate locally)

If anything in this guide is unclear or you want more detail (examples for adding an SConscript, how to run the emulator/CI locally, or common env vars), tell me which area to expand and I'll iterate.

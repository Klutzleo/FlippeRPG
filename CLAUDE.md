# Working in FlippeRPG

This repository is a fork of the Flipper Zero firmware. The game lives in `applications_user/FlippeRPG`. Do not change firmware files outside that folder unless Jason asks.

## Project status (read first, update last)
`PROJECT_STATUS.md` is how Jason and Claude stay in sync across sessions. A morning job reads it across all his repos.

- **Before starting work:** read `PROJECT_STATUS.md`, and check open PRs (`gh api repos/Klutzleo/FlippeRPG/pulls`). Work often sits in stacked PRs, not on `dev`. If the status looks older than the branches or PRs, say so and fix it.
- **Before finishing a session:** update `PROJECT_STATUS.md`:
  - `Last session` — date, what changed, where you stopped.
  - `Resume here` — the first file or PR for the next session.
  - `waiting_on` — `jason`, `claude`, `external` or `none`. If a decision or a device test is Jason's, put it in `decision` and what you will do next in `if_go`.
  - `focus`, `next_milestone`, `momentum` if they changed.
- Only write what you verified from the code, git history, a PR or a test. Say plainly what was not run on a Flipper; Claude cannot run one.
- Do not change `excitement` or `priority`; those are Jason's.
- Keep the file's existing sections.

## Conventions
- Changes go through a branch and a PR, never straight to `dev`. Open PRs as drafts until they have been run on a device.
- The lore PDFs are not in this public repository. Do not commit lore text except through the generator described in the transmissions PR.
- Build: `./fbt fap_flipperpg` (Linux/macOS) or `.\fbt.cmd fap_flipperpg` (Windows). The build must have no warnings.
- Game-logic tests that run on a PC (no Flipper) live in `applications_user/FlippeRPG/tests/host/` once the host-tests PR is merged; run them before opening a PR.
- Saved-game structs are append-only (save v2): never reorder or remove fields.

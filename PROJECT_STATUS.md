---
phase: active
priority: high
category: embedded-game
progress: 68                     # unchanged since June; Jason to confirm after the open PRs are tested
focus: "Three stacked draft PRs (#1-#3) hold the Oct 27 work; waiting for an on-device test"
next_milestone: "PRs #1-#3 tested on a Flipper and merged, then real shrine triggers"
milestone_distance: weeks
excitement: high
strategic: true
momentum: rolling
audience: flipper-zero-community
uniqueness: first-mover ARG/RPG for Flipper
viral_potential: high
mvp_distance: weeks
waiting_on: jason                # jason / claude / external / none
decision: "Test PRs #1-#3 on a Flipper (each PR has a test plan) and merge in order #1, #2, #3. Anything fail?"
if_go: "After the merges, Claude starts real shrine triggers (IR first) on a branch, host-tested; or the IR-hash fix if a device test shows different remote buttons counting as one source. Claude cannot run a Flipper, so anything hardware-facing comes back for Jason to try."
status_verified: "2026-11-04 against git history of dev and the three open PRs; nothing run on a device"
---

## Current state (2026-11-04)
`dev` has not changed since 2026-06-26. The Oct 27 work is on three **open, stacked draft PRs**, built but **not yet run on a Flipper**:

- **#1 `scoring-and-rf-merge` → dev:** scoring reworked to the lore's per-type daily rules (a source counts once a day, gain +3/+1/+0 per band per day, one band opens per day); RF merged into Sub-GHz; The Unread (RFID) shrine and Legible aura wired; The First Frequency shrine and Grounded aura retired; save format v2 (old saves load); stamina recovery and absence decay; shrine list fixes; "XP" wording removed.
- **#2 `host-tests` → #1:** PC-side tests of the game rules (`tests/host/run.sh`, no Flipper needed). Three old tests that no longer built were removed.
- **#3 `transmissions` → #2:** the lore's real transmissions (82) replace the 20 placeholders, word-wrapped at runtime. The `.fap` grows from 57 KB to 77 KB, so first check that it still loads.

The sections "What's working" and "What's missing" below describe `dev` as of June. Where they disagree with the PRs, the PRs are newer. Still true on the PR branches: shrines use a random 70% roll (`FlippeRPG.c`, search for `rand() % 100) < 70`), and the PWA does not exist. The `Klutzleo/flipperpg-pwa` repo is only a LICENSE file from 2026-03-30.

## What this is
FlippeRPG is an ARG-flavored RPG for the Flipper Zero. Players scan real-world RF signals
(IR, Sub-GHz, NFC, Bluetooth, RFID, RF) to advance through a hidden progression system.
The game is designed around mystery — locked content is hidden, not shown as locked. The
aesthetic is underground/hacker/ham-radio. The endgame (RESONANT) is community-gated via
a voucher from another player.

Builds as a `.fap` external app. API version 87.1.
Build: `.\fbt.cmd fap_flipperpg`
Deploy: `.\fbt.cmd launch APPSRC=applications_user/FlippeRPG`
Output: `build\f7-firmware-D\.extapps\flipperpg.fap` → `/ext/apps/User/flipperpg.fap` on COM6

---

## What's working
*As of `dev` on 2026-06-26. See Current state above for what the open PRs change (scoring, RF, transmissions).*

**Core signal loop**
All 6 hardware scanners implemented and integrated: IR, Sub-GHz, NFC, Bluetooth, RF, RFID.
Each scanner runs in its own thread, posts a custom event on completion, and the main loop
handles gain calculation, codex update, and save.

**Band gate (Tier 1)**
IR → Sub-GHz → NFC → Bluetooth. 5 scans each. Sequential unlock. Bands are HIDDEN until
unlocked. THE SUBSTRATE fires CQ in Morse (the only haptic in the game) when all four
Tier 1 bands are complete. This works end-to-end.

**Transmission system**
4 bands × 5 narrative levels (Ambient → Noticed → Responsive → Urgent → Threshold).
Fires automatically on qualifying scans. Acknowledge bonus (+8 signal) on OK.
Band completion bonus (+30 signal) when a Threshold tx confirms the gate.

**Signal strength**
10 tiers: STATIC → FAINT → TRACE → RISING → PRESENT → CLEAR → STRONG → DEEP →
RESONANT EDGE → RESONANT. Players see labels only — raw score is hidden. Gain values:
+3 first unique scan/day, +1 second, +0 third+, +8 tx ack, +30 band complete, +5 duel win.

**Codex / save system**
Full player state (~5KB struct) saved to `APP_DATA_PATH("codex.bin")` with magic `FLRP`.
Static Codex to avoid stack overflow. Load on boot; saves on every scan and on exit.

**UI navigation**
12 views: Menu (dynamic), Codex Status, Attune (band list), Band Detail, Transmission,
Shrine List, Shrine Detail, Campfire, Campfire Profile, Duel, The Substrate, Name Entry.
Menu rebuilds dynamically — pre-substrate shows only Attune/Status/Exit. Post-substrate
adds The Substrate, Shrines, Campfire without announcement.

**Duel system**
Signal Reading minigame: 3 options, 3-second timer, aura hint `[!]` flag. Correct =
+5 signal. Wrong/timeout = -20 stamina (stamina is separate from signal strength).
Launches from Campfire Profile. Win/loss tracked in Codex.

**Shrine system (partial)**
6 canonical shrines wired to aura assignment. Attempt/success/failure/corruption logic
implemented. Aura assignment maps correctly: Clearseeing (IR/Fixed Gaze), Touchmarked
(NFC/Exchange), Grounded (RF/First Frequency), Stormtouched (Sub-GHz/Open Channel),
Heard (BT/Unanswered Hello). Sixth shrine SHRINE_POCKET_CARRIED (RFID/The Unread) not
fully wired in the map.

**QR code / The Substrate view**
Real qrcodegen (nayuki) integrated. Encodes `FLRP:<codex_id>:<signal_level>:<6 band digits>`.
Renders at 2px/module centered on 128×64. Payload text fallback below QR.

**Echo system (data layer)**
EchoEntry structs, echo_fusion.c, echo_event() implemented. Not yet exposed in UI.

**Campfire**
Layout, fire animation (conditional — `#ifdef A_Campfire_Fire_16x16`), 4 player slots
(N/S/E/W), encounter_log population, profile view. Gated behind zero_day_confirmed + faction.

**Zero Day**
Auto-set when substrate unlocks. Gates campfire, aura reveal in status screen, faction display.

---

## What's missing / not started
*As of `dev` on 2026-06-26. PR #1 covers: SHRINE_POCKET_CARRIED / The Unread, absence decay, stamina recovery, the "XP" wording, and parts of the shrine list/detail fixes.*

**Shrine signal condition triggers**
The real shrine trigger logic (e.g., multiple distinct RFID UIDs = The Unread; high device
density + low connection rate = The Unanswered Hello) is not implemented. Currently the
shrine detail offers a random 70% success ritual instead of reading actual scan state.

**Shrine detail — lore accuracy**
- Shrine detail shows "Status: LOCKED" for incomplete shrines — should be HIDDEN per design.
- Aura name shown on shrine detail before zero_day_confirmed — should be hidden until PWA.

**SHRINE_POCKET_CARRIED (The Unread / RFID)**
Not in shrine_requirements[] mapping. Needs to be wired.

**Player NFC shrines**
Community-planted shrine system (plant NFC tag → another player scans it → both get signal
credit → appears on PWA map). Not started.

**PWA companion app**
Required for: faction selection post-Zero Day, aura reveal confirmation, Resonant voucher
system, player NFC shrine map, witness accounts. Not started. This is a hard blocker for
the full endgame loop.

**Faction chicken-and-egg**
Campfire requires `zero_day_confirmed && faction != ""`. Faction can only be set via PWA
(doesn't exist yet). Nobody can reach Campfire until PWA ships.

**Absence decay**
-1 signal per inactive day not implemented.

**Stamina recovery**
Auto-recovery over time not implemented. Stamina just sits at whatever duel loss left it.

**Resonant arrival sequence**
The three faction-specific arrival screens (Operators / Nodes / Sovereigns), the 73/AS
Morse sequences, the voucher notification on existing Resonants' devices — none of this
is built. The RESONANT tier is defined in constants.h but cannot be reached.

**Echo UI**
EchoEntry data exists in Codex but there's no view to browse, fuse, or interact with echoes.

**Techniques**
Data structs and technique.c exist. Techniques deliberately removed from UI. No technique
effects implemented. Low priority until post-Zero Day content matters.

**Tier 2 / Tier 3 band gating**
RFID and RF are Tier 2 (post-Zero Day). GPIO and iButton are Tier 3 (post-DEEP). The
Attune view currently shows all unlocked bands but doesn't enforce the Zero Day gate for
RFID/RF unlock.

**Signal sharing (multiplayer P2P)**
signal_share.h/.c stub exists. No actual BT-based player-to-player signal transfer.

---

## Known language / polish bugs
- Campfire Profile shows "Handshake: +2 XP" — should be "Signal", not "XP"
- Shrine detail shows `Status: LOCKED` — should be hidden until the player has a reason to see it
- Shrine list shows all 6 shrines — only shrines the player has encountered should appear

---

## Next up
- [ ] Run the on-device test plan in PR #1, then #2 and #3; merge in order (#3 retargets automatically)
- [ ] Wire real signal conditions to shrine triggers (start with IR; the 70% roll is still the placeholder)
- [ ] IR hash may not use the protocol, so different buttons on one remote could count as one source (follow-up named in PR #1; needs a device test)
- [ ] Shrine detail: hide content before zero_day_confirmed
- [ ] Echo UI (browse, fuse)
- [ ] PWA spec and scaffold (faction, aura confirm, shrine map); `Klutzleo/flipperpg-pwa` exists but is empty
- [ ] Re-add the "[SCAN QR TO GO DEEPER]" line to transmissions once PWA landing pages exist (left out on purpose in PR #3)

## Blockers
- Nothing has been run on a Flipper since the Oct 27 changes. Until it is, the three PRs cannot be merged safely.
- The PWA is a hard blocker for the full endgame loop (faction selection gates the Campfire).
- The lore PDFs are not in the repo, so `tools/gen_transmissions.py` (PR #3) needs them on the machine that regenerates the transmissions.

## Resume here
Open PR #1 (https://github.com/Klutzleo/FlippeRPG/pull/1) and work through its "Test plan on device" checklist with the Flipper plugged in; PRs #2 and #3 have their own lists (#3's first item: the app still loads at 77 KB).
When those are merged, the first code task is the shrine ritual: the placeholder roll is in `FlippeRPG.c`, and `shrine/shrine.c: trigger_shrine()` currently only checks that the signal type matches the shrine.

---

## Last session
2026-11-04: Status rewritten by Claude from git history and the three open PRs (the file had not been updated since the Oct 27 work). Nothing was run on a device. Before that: 2026-10-27, three stacked draft PRs opened (#1 scoring and RF merge, #2 host tests, #3 real transmissions).

2026-06-26: Project status doc rewritten from scratch. All 6 scanners confirmed integrated.
Band gate, CQ Morse, Transmission system, QR substrate view all working. Shrine logic wired
but using random success — real signal condition triggers are the immediate next milestone.

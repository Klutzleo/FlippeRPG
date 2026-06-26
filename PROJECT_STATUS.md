---
phase: active
priority: high
category: embedded-game
progress: 68
focus: Shrine signal conditions + PWA companion
next_milestone: Real shrine triggers wired to hardware scan results
milestone_distance: weeks
excitement: high
strategic: true
momentum: rolling
audience: flipper-zero-community
uniqueness: first-mover ARG/RPG for Flipper
viral_potential: high
mvp_distance: weeks
---

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
- [ ] Wire real signal conditions to shrine triggers (start with The Fixed Gaze / IR — simplest)
- [ ] Hide shrines not yet encountered from Shrine List
- [ ] Hide shrine detail content before zero_day_confirmed
- [ ] Fix "XP" language bug in Campfire Profile
- [ ] Fix SHRINE_POCKET_CARRIED missing from shrine_requirements[]
- [ ] Implement absence decay (daily -1 signal on inactive day)
- [ ] Stamina recovery over time
- [ ] Echo UI (browse, fuse)
- [ ] PWA spec and scaffold (faction, aura confirm, shrine map)

---

## Resume here
All 6 scanners work. Band gate fires CQ correctly. Transmissions display correctly. QR
on The Substrate encodes and renders. The first incomplete loop is the shrine system —
shrines need real signal condition triggers instead of the random 70% placeholder.

Start in `shrine/shrine.c` → `trigger_shrine()`. The signal type is already passed in.
Add threshold checks per band: e.g., IR shrine needs a stationary IR source held for N
scans; Sub-GHz shrine needs multiple overlapping sources. The scan state from the scanner
thread is available at the time trigger_shrine() is called.

After shrines: faction/PWA is the critical path for the full endgame loop.

---

## Last session
2026-06-26: Project status doc rewritten from scratch. All 6 scanners confirmed integrated.
Band gate, CQ Morse, Transmission system, QR substrate view all working. Shrine logic wired
but using random success — real signal condition triggers are the immediate next milestone.

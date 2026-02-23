# 🧿 FlippeRPG  
*A signal-driven micro-RPG for Flipper Zero*

FlippeRPG is a modular, narrative-driven game designed for the Flipper Zero. Absorb real-world signals (IR, NFC, Sub-GHz), complete shrine rituals to unlock techniques, battle other players at the campfire, and uncover secrets hidden in the signal spectrum. Built for hackers, storytellers, and curious explorers.

---

## ✨ Current Features (MVP Complete)

### Core Gameplay
- **Signal Absorption** - Scan and absorb signals (IR, NFC, Sub-GHz, GPIO) to gain XP
- **Shrine Progression** - Unlock shrines by collecting signal types, complete rituals to gain auras and techniques
- **Technique System** - Unlock powerful spells tied to shrine completion
- **Codex Tracking** - Player stats, signal history, encounter log, shrine progress

### Multiplayer (Campfire Scene)
- **Campfire Hub** - Visual multiplayer space with N/S/E/W player positions around a fire
- **Auto-Handshake** - Gain XP just by being near other players
- **Dueling** - Challenge other players, win duel XP (60% win rate = 15 XP, loss = 5 XP)
- **Echo Exchange** - Share signal fragments with other players
- **Player Persistence** - Encounter log tracks all meetings

### Save System
- Local save/load via filesystem
- Persistent codex state

---

## 🎨 Visual Design & Assets

### Current Status
The game currently uses **ASCII/text-based rendering** for the campfire scene:
- Fire: `[*]` text symbol
- Players: Names at cardinal directions (N, S, E, W)
- Simple text-based UI

### Next Phase: Sprite Assets
We're ready to integrate **custom sprite animations** for visual polish:
- Animated campfire with flickering frames
- Player character icons at each direction
- Pop-in/fade animations for multiplayer
- Selection glow effects

**→ See [ASSET_CREATION.md](FlippeRPG/ASSET_CREATION.md) for a complete guide on:**
- Designing sprites in Aseprite
- Exporting PNG assets
- Integrating sprites into the game code
- Animation timing and frame management

---

## 🚀 Build Instructions

### Prerequisites
- Flipper Zero Firmware repo cloned
- `fbt` build tool available (included in firmware)

### Build FlippeRPG
```bash
cd /path/to/flipperzero-firmware
./fbt fap_flipperpg
```

### Deploy to Flipper
```bash
./fbt fap_deploy  # Push compiled FAP to connected Flipper
```

### Run on Device
1. Press Home button on Flipper
2. Navigate to: Applications → Games → FlippeRPG
3. Press OK to start

---

## 📁 Project Structure
```
FlippeRPG/
├── FlippeRPG.c              # Main app entry point & UI views
├── ASSET_CREATION.md        # Guide for designing & integrating sprites
├── codex/                   # Player state management
│   ├── codex.h/c            # Codex struct and save/load
├── signal/                  # Signal detection & processing
│   ├── signal_engine.h/c    # Core signal mechanics
│   ├── signal_fusion.h/c    # Multi-signal combos
│   ├── signal_share.h/c     # Multiplayer signal exchange
├── shrine/                  # Shrine ritual system
│   ├── shrine.h/c           # Shrine logic & aura assignment
│   ├── shrine_tags.h/c      # Shrine tagging for SubGHz
├── techniques/              # Spell/ability system
│   ├── techniques.h/c       # Technique definitions
│   ├── subghz_tx.h/c        # SubGHz transmission
├── duel/                    # PvP combat (framework ready)
├── echo/                    # Signal fragments & fusion
│   ├── echo_fusion.h/c      # Echo combination mechanics
│   ├── echo_texts.h/c       # Flavor text for echoes
├── xp/                      # Experience & level mechanics
│   ├── xp_engine.h/c        # XP calculation & awards
├── save/                    # Persistence layer
│   ├── save_system.h/c      # File I/O for codex
├── assets/                  # Sprites, icons, fonts
│   ├── campfire_fire.png    # (To be created)
│   ├── campfire_*.png       # (To be created)
├── core/                    # Shared utilities
│   ├── constants.h          # Game enums (ShrineID, SignalType, etc)
│   ├── utils.h/c            # Helper functions
└── tests/                   # Unit tests
    ├── test_codex.c
    ├── test_shrine.c
    ├── test_signal_*.c
```

---

## 🎮 Gameplay Flow

### Main Menu
1. **Absorb Signals** - Scan for IR/NFC/SubGHz signals
2. **Shrines** - Browse and attempt shrine rituals
3. **Techniques** - View unlocked spells and mastery
4. **Multiplayer** - Enter the campfire scene
5. **Status** - Check your Codex stats
6. **Exit** - Save and quit

### Campfire Multiplayer
```
        [North Player]
            Name
            |
[West]----(FIRE)----[East]
 Name        [*]       Name
            |
         (You)
```
- **Arrow keys** to select position (N/S/E/W)
- **OK** to view player profile
- **L/R** in profile: Duel or Exchange Echoes

### Shrine Progression
1. Navigate Shrines menu
2. Select a shrine
3. View ritual requirements
4. Press OK to attempt (70% success)
5. Success → Gain aura trait + technique unlock
6. Failure → 50% chance of corruption (unlocks replay)

---

## 🔧 Development & Customization

### Adding New Features
- **Signal Types**: Add to `core/constants.h` (SignalType enum)
- **Shrines**: Add to `shrine/shrine.h`, implement in `shrine.c`
- **Techniques**: Add to `techniques/techniques.h`, logic in `.c`
- **UI Views**: Add to main `FlippeRPG.c` following existing pattern

### Creating Asset Packs
Follow [ASSET_CREATION.md](FlippeRPG/ASSET_CREATION.md) for:
- Sprite design best practices
- Animation frame optimization
- Aseprite workflow
- PNG export & integration

### Testing
```bash
# Run unit tests
cd tests
./test_codex.exe
./test_shrine.exe
./test_signal_*.exe
```

---

## 📊 Game Systems Reference

### XP System
- **Signal XP**: Bonus for unique signals (dedup mechanic)
- **Duel XP**: Separate pool from signal XP, separate tracking
- **Shrine XP**: Awarded on ritual completion
- **Handshake XP**: +2 XP auto-gain when meeting players at campfire

### Aura Traits (Unlocked by Shrines)
- **Clearseeing** - Flame Reach shrine (IR signals)
- **Touchmarked** - Bind Whisper shrine (NFC signals)
- **Stormtouched** - Cave That Listens shrine (SubGHz signals)
- **Grounded** - Echo Touched shrine (RF signals)
- **Heard** - Thread Touch shrine (Bluetooth signals)

### Encounter & Echo System
- **Encounter Log** - Records every Flipper you meet
- **Echo Log** - Signal fragments collected and fused
- **Handshake** - Auto-exchange on first meeting
- **Echo Transfer** - Optional deeper exchange after duel/encounter

---

## 🎯 Roadmap (Future)

### Phase 2: Polish
- [ ] Sprite assets for campfire (fire, player icons, animations)
- [ ] Real SubGHz scanning & broadcast
- [ ] NFC tag writing for save sharing
- [ ] Persistent multiplayer lobby (simulated)

### Phase 3: Expansion
- [ ] Duel animations & combat sequences
- [ ] More shrine types & aura combinations
- [ ] Signal-triggered narrative events
- [ ] Leaderboard/ranking system

### Phase 4: Community
- [ ] Asset pack system for custom UI themes
- [ ] Modular story extensions
- [ ] Community shrine/technique library

---

## 📖 Learn More

- [Flipper Zero Official Docs](https://docs.flipper.net)
- [Developer Guide](https://developer.flipper.net)
- [Flipper Discord Community](https://flipp.dev/discord)

### Asset Creation Tools
- **Aseprite** - Professional pixel art editor (~$20)
- **LibreSprite** - Free open-source alternative
- **Piskel** - Free web-based sprite editor

---

## 📄 License
MIT License - See LICENSE file

## Contributors
- Core game design & implementation
- Campfire multiplayer system
- Signal integration framework

---

## 🤝 Contributing

Found a bug? Have a feature idea? Want to contribute sprites or mechanics?

1. Check [GitHub Issues](../../issues)
2. Fork the repo
3. Create a feature branch
4. Submit a pull request

For major changes, please open an issue first to discuss.

---

**Happy signal hunting, Signalborn! 🔥**
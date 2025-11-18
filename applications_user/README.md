# 🧿 FlippeRPG  
*A signal-driven micro-RPG for Flipper Zero*

FlippeRPG is a modular, narrative-driven game designed for the Flipper Zero. Explore dungeons, interact with the real world using IR, NFC, and Sub-GHz signals, and uncover secrets hidden in the signal spectrum. Built for hackers, storytellers, and curious explorers.

---

## 🚀 Features (Planned)
- IR-triggered dungeon exploration  
- Real-world hooks (NFC scans, IR blasts, RF transmissions)  
- Minimalist pixel UI and sprite-based interactions  
- Modular story engine for branching paths  
- Pet companion or signal spirit (optional)  
- Offline score tracking and challenge seeds  

---

## 🛠️ Build Instructions

1. **Clone the Flipper Zero Firmware repo**  
   ```bash
   git clone https://github.com/flipperdevices/flipperzero-firmware
   cd flipperzero-firmware
   ```

2. **Create your app folder**  
   Inside `applications_user`, create a folder named `flippeRPG` and drop your `.c` files there.

3. **Build with FBT**  
   ```bash
   ./fbt fap_flippeRPG
   ```

4. **Flash to Flipper**  
   Use [qFlipper](https://flipperzero.one/qflipper) or USB to install the `.fap` file.

5. **Test in Emulator**  
   Visit [lab.flipper.net](https://lab.flipper.net/) to run your app without flashing hardware.

---

## 📁 Repo Structure (Coming Soon)
```
flippeRPG/
├── flippeRPG.c         # Main game logic
├── assets/             # Sprites, maps, story data
├── README.md
├── LICENSE
└── .gitignore
```

---

## 📜 License
This project is licensed under the [MIT License](LICENSE).
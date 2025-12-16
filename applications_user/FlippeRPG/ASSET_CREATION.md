# FlippeRPG Asset Creation Guide

## Overview

FlippeRPG uses sprites and animations for visual elements, particularly the **Campfire Multiplayer Scene**. This guide explains how to design, export, and integrate custom assets into the game.

**Display Info:** Flipper Zero has a **128x64 pixel monochrome LCD display** (black and white only). All sprites must be designed in black and white for optimal visibility on the device.

## Current Implementation Status

### Text-Based Rendering (Current)
The campfire scene currently uses **ASCII text** to represent:
- Fire: `[*]` in the center
- Players: Names at N/S/E/W positions with selection markers `>`
- Player identifiers: Simple text labels

### Sprite-Ready Architecture
The code is **fully prepared** to swap text rendering for sprite rendering. You just need to:
1. Design sprites in Aseprite
2. Export as PNG
3. Integrate into the code (minimal changes needed)

---

## Asset Design Requirements

### Campfire Fire Animation
**Purpose:** Center of the campfire scene, should flicker to show it's alive

**Specifications:**
- **Size:** 16x16 pixels (can scale to 8x8 if space is tight)
- **Frames:** 3-4 frames of flickering fire
- **Animation Speed:** 100-150ms per frame
- **Style:** Simple pixel art in **black and white only** (use varying density of black pixels to show flickering effect, not colors)
- **Format:** PNG with transparent background

**Example Design:**
```
Frame 1:  Frame 2:  Frame 3:  Frame 4:
  ***       * *      * *        ***
 *   *     *   *    *   *      *   *
*  O  *   *  O  *  *  O  *   *  O  *
 *   *     *   *    *   *      *   *
  ***       * *      * *        ***
```

### Player Character Icons
**Purpose:** Show which player is at each cardinal direction (N, S, E, W)

**Specifications:**
- **Size:** 8x8 or 12x12 pixels
- **Frames:** 2-3 frames (optional idle animation)
- **Style:** Simple silhouettes or basic character poses
- **Per-Player:** Different icons for each direction (↑↓←→) or player type

**Variants to Create (Basic - 4 directions):**
1. **North** - `campfire_player_north.png` (player above fire)
2. **South** - `campfire_player_south.png` (your character, always at bottom)
3. **East** - `campfire_player_east.png` (right side)
4. **West** - `campfire_player_west.png` (left side)

**Character Appearance Options:**
You can support multiple character appearances (male, female, different styles, etc.):

```
assets/
├── campfire_mage_m_north.png      # Male mage variant
├── campfire_mage_m_south.png
├── campfire_mage_m_east.png
├── campfire_mage_m_west.png
├── campfire_mage_f_north.png      # Female mage variant
├── campfire_mage_f_south.png
├── campfire_mage_f_east.png
├── campfire_mage_f_west.png
└── ... (repeat for other appearance types)
```

The game code can load the correct sprite based on player preference. See **Code Integration** section for example.

**Design Ideas:**
- **Stick figures** - Surprisingly effective on monochrome! Simple lines are highly visible, can show poses/poses distinctly (arms up, arms down, legs spread = unique silhouettes)
- **Silhouettes** - Solid black shapes with minimal detail (fast to draw, very clear on monochrome)
- **Icon-based** - Simple symbols per aura type (fire = flame shape, whisper = wavy lines, etc.)
- **Directional indicators** - Different facing direction per cardinal position

### Pop-In/Fade Animation
**Purpose:** Animate player appearance/disappearance when they join/leave the campfire

**Specifications:**
- **Frames:** 4-5 frames from invisible → fully visible
- **Animation Speed:** 200ms per frame
- **Effect:** Opacity fade-in or sprite scale-in

### Selection Indicator
**Purpose:** Show which player is currently selected

**Specifications:**
- **Size:** 4x4 or 8x8 pixels (glow effect around selected player)
- **Frames:** 2 frames (optional pulse effect)
- **Animation Speed:** 300ms per frame
- **Style:** Highlight/glow around the selected player

---

## Aseprite Workflow

### Step 1: Create New Document
```
File → New
Width: 128px (standard for single sprites or sprite sheets)
Height: 64px (Flipper's actual screen height)
Background: Transparent
Color Mode: Black & White
  (In Aseprite: Image → Mode → Indexed, then Color → Colormap → Black and White)
```

### Step 2: Design Fire Animation
1. Create a new layer called "Fire"
2. Draw 3-4 frames of flickering fire using **only black pixels** on transparent background
3. To simulate intensity/brightness: use denser pixel patterns for brighter frames, sparser patterns for dimmer flames
4. Keep it small (16x16) to leave room for text/player names
5. **Important:** Test on actual Flipper—monochrome rendering may look different than your color preview editor

### Step 3: Design Player Icons
1. Create layers for each direction (North, South, East, West)
2. Draw simple arrow or character icons (8x8 or 12x12)
3. Alternatively, draw different character types for variety

### Step 4: Export Sprite Sheet
```
File → Export As
Format: PNG
Options:
  - Interlaced: OFF
  - Crop Sprite: ON (auto-crop transparency)
  - Layer: Merge Down
```

### Step 5: Organize Files
```
FlippeRPG/
├── assets/
│   ├── campfire_fire.png          # 16x16, 4 frames (64x16 total)
│   ├── campfire_north.png         # 8x8 or 12x12
│   ├── campfire_south.png         # Your character icon
│   ├── campfire_east.png
│   ├── campfire_west.png
│   └── campfire_selection.png     # Highlight glow
```

---

## Integration into Code

### Current Code Structure
The campfire rendering is in [FlippeRPG.c](FlippeRPG.c), function `campfire_draw_callback()`:

```c
static void campfire_draw_callback(Canvas* canvas, void* model) {
    // Currently draws text-based campfire
    canvas_draw_str(canvas, 58, 35, "[*]");  // Fire
    canvas_draw_str(canvas, 8, 25, campfire_slots[CAMP_NORTH].name);  // North
    // ... etc
}
```

### How to Add Sprite Rendering

**Step 1: Load PNG as Image Resource**

In `FlippeRPG.c`, add at the top:
```c
#include <gui/modules/image.h>

// At app init, load your PNG files:
Image* fire_img = image_alloc();
image_load(fire_img, "assets/campfire_fire.png");

Image* player_north_img = image_alloc();
image_load(player_north_img, "assets/campfire_player_north.png");
// ... load other directional images
```

**For Multiple Appearances (Optional):**
If you want male/female variants, store them by appearance type:
```c
// In Codex struct (codex/codex.h), add:
typedef enum {
    APPEARANCE_MAGE_MALE,
    APPEARANCE_MAGE_FEMALE,
    // ... other variants
} AppearanceType;

typedef struct {
    // ... existing fields
    AppearanceType appearance;  // Player's chosen appearance
} Codex;

// Then load based on player choice:
const char* get_player_sprite_path(AppearanceType appearance, CardinalDir dir) {
    const char* variants[NUM_APPEARANCES][4] = {
        // APPEARANCE_MAGE_MALE
        {"assets/campfire_mage_m_north.png", "assets/campfire_mage_m_south.png",
         "assets/campfire_mage_m_east.png", "assets/campfire_mage_m_west.png"},
        // APPEARANCE_MAGE_FEMALE
        {"assets/campfire_mage_f_north.png", "assets/campfire_mage_f_south.png",
         "assets/campfire_mage_f_east.png", "assets/campfire_mage_f_west.png"},
    };
    return variants[appearance][dir];
}
```

**Step 2: Draw Images in campfire_draw_callback()**

Replace text rendering with:
```c
// Draw fire in center
canvas_draw_icon(canvas, 55, 32, fire_img);  // Centered at 58, 35

// Draw north player icon
if(campfire_slots[CAMP_NORTH].active) {
    Image* player_sprite = image_alloc();
    image_load(player_sprite, get_player_sprite_path(player_codex.appearance, CAMP_NORTH));
    canvas_draw_icon(canvas, 50, 15, player_sprite);
    canvas_draw_str(canvas, 60, 23, campfire_slots[CAMP_NORTH].name);  // Name below icon
    image_free(player_sprite);
}
// ... repeat for E, W, S directions
```

**Step 3: Animate Frames**

Add frame tracking:
```c
static uint32_t frame_tick = 0;
static int fire_frame = 0;

// In draw callback:
uint32_t now = furi_get_tick();
if(now - frame_tick > 150) {  // 150ms per frame
    fire_frame = (fire_frame + 1) % 4;  // 4 frames of fire
    frame_tick = now;
}

// Draw the current frame of the sprite sheet
// This requires a sprite sheet parser, or use individual PNG files
```

### Simpler Alternative: Individual PNGs per Frame

If you export each frame as a separate file:
```
campfire_fire_1.png
campfire_fire_2.png
campfire_fire_3.png
campfire_fire_4.png
```

Then rotate through them:
```c
const char* fire_frames[] = {
    "assets/campfire_fire_1.png",
    "assets/campfire_fire_2.png",
    "assets/campfire_fire_3.png",
    "assets/campfire_fire_4.png",
};

Image* current_fire = image_alloc();
image_load(current_fire, fire_frames[fire_frame % 4]);
canvas_draw_icon(canvas, 55, 32, current_fire);
```

---

## Design Tips

### Fire Animation
- **Flicker asymmetrically** - Real fire doesn't flicker evenly
- **Use density variation** - More black pixels = brighter/more intense flames, fewer pixels = dimmer flames
- **Add details** - Embers, smoke wisps make it feel alive (all rendered in solid black on transparent)
- **Test on actual device** - The Flipper's monochrome LCD may render patterns differently than your editor's color preview

### Player Icons - Stick Figures Rock on Monochrome
**Yes, stick figures are perfect for Flipper's monochrome display!**

Why stick figures work great:
- **High contrast** - Clean lines show up perfectly on black and white
- **Distinctive poses** - Easy to show different stances (arms up, arms down, legs spread = unique silhouettes)
- **Fast to draw** - Quick iteration in Aseprite
- **Scalable** - Works at any size from 8x8 to 16x16
- **Personality** - Can show aura/mood with simple additions (halo = good, spiky = angry, etc.)

**Stick figure variants with wizard accessories:**
- **Wizard hat** - Triangle/cone shape on head (pointy = mage vibes)
- **Magic wand/staff** - Single line extending from hand
- **Robe** - Triangle/trapezoid body instead of stick body
- **Spell effects** - Small stars/sparkles near wand
- **Male/Female distinction** - Hat style (pointy vs wide brim), robe shape, or accessory choice

**Quick examples (8x8 pixel stick figures as Signal Mages):**

**Basic Mage with Hat:**
```
  ^        (wizard hat)
  *        (head)
 -*-       (body + wand)
  |
 / \       (legs)
```

**Robed Mage with Staff:**
```
  ^        (wizard hat)
  *        (head)
 /|\       (robe)
/_|_\      (wide robe + staff)
```

**Female Mage with Wand & Stars:**
```
  ^        (pointy hat)
  *     *  (head + sparkle)
 /^\   |   (dress robe + wand)
  |        (body)
 / \       (legs)
```

**Aura-Specific Accessories:**
- **Flamebound** - Spiky/jagged hat, fire wand with pointed tip
- **Whispered** - Curved/wispy staff, flowing robe edges
- **Stormtouched** - Lightning bolt on hat, zigzag wand
- **Echoed** - Circular/spiral staff top, symmetrical design

### Other Player Icon Approaches
- **Silhouettes** - Solid black shapes with minimal detail (very visible, aura-specific shapes)
- **Icon-based** - Simple symbols per aura type (fire = flame, whisper = wavy lines, storm = lightning bolt)
- **Robed mages** - Triangular/hooded shape representing magic users

### Animation
- **Keep frame count low** - More frames = larger file size
- **Consistent timing** - Use same speed for all animations (150ms = good default)
- **Subtle is better** - Don't make things too flashy; it should complement gameplay

### Grayscale & Contrast for Monochrome Display
**Important:** Flipper Zero's LCD is monochrome, not color:
- Only **solid black** (opaque) and **white/transparent** (invisible) render properly
- **No colors or grays** - they won't display as intended
- **Design strategy:** Use solid black on transparent; vary density (pixel patterns) to simulate intensity/brightness
- **Visibility:** High contrast is essential; thin lines (1-2 pixels) may be hard to see at actual size
- **Testing:** What looks good in your editor preview may need adjustment when deployed to the device

---

## Testing Your Assets

### Visual Preview
1. Export PNG from Aseprite
2. Open in image viewer at actual size
3. Check visibility and clarity

### In-Game Testing
1. Place PNG in `assets/` folder
2. Update `FlippeRPG.c` to load and render it
3. Build: `./fbt.cmd fap_flipperpg` (Windows) or `./fbt fap_flipperpg` (Linux/macOS)
4. Deploy to Flipper and test campfire scene
5. Verify animation speed and visibility on the **actual 128x64 monochrome LCD**
6. **Critical:** Sprites that look good in your editor preview may need tweaking for the Flipper's display; be ready to iterate based on how they appear on the real device

---

## Resources & Tools

### Free Tools
- **Aseprite** (paid, ~$20) - Industry standard for pixel art
- **Piskel** (free, web-based) - Simple sprite editor
- **LibreSprite** (free, open-source) - Aseprite fork

### Pixel Art Resources
- **Lospec.com** - Pixel art palettes and tutorials
- **Opengameart.org** - Free pixel art sprites and tutorials
- **Itch.io** - Tons of pixel art tutorials and asset packs

### Flipper-Specific
- [Flipper Zero Documentation](https://docs.flipper.net/)
- Canvas rendering functions: See `furi/gui/canvas.h`
- Image loading: See Flipper SDK examples

---

## Next Steps

1. **Design sprites** in Aseprite (start with fire animation)
2. **Export as PNG** to `applications_user/FlippeRPG/assets/`
3. **Test rendering** - I can help integrate them into the code
4. **Animate frames** - Add frame cycling with proper timing
5. **Polish** - Iterate on design based on how it looks on actual Flipper

Feel free to ask for help with any step! The code is ready for sprite assets whenever you have them.

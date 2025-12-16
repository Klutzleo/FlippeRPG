# FlippeRPG Asset Creation Guide

## Overview

FlippeRPG uses sprites and animations for visual elements, particularly the **Campfire Multiplayer Scene**. This guide explains how to design, export, and integrate custom assets into the game.

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
- **Style:** Simple pixel art, warm colors (yellows, oranges, reds)
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

**Variants to Create:**
1. **North Arrow** - `arrow_north.png` (player above fire)
2. **South Arrow** - `arrow_south.png` (your character, always at bottom)
3. **East Arrow** - `arrow_east.png` (right side)
4. **West Arrow** - `arrow_west.png` (left side)

**Alternative (Character-Based):**
- Create small character icons instead of arrows
- Example: Mage silhouette, Knight silhouette, etc.

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
Width: 128px (for full sprite sheet, or individual 16x16 for each sprite)
Height: 48px (for 3 frames of 16x16, stacked)
Background: Transparent
```

### Step 2: Design Fire Animation
1. Create a new layer called "Fire"
2. Draw 3-4 frames of flickering fire
3. Use warm colors: #FF6B00 (orange), #FFD700 (gold), #FF0000 (red)
4. Keep it small (16x16) to leave room for text/player names

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
image_load(player_north_img, "assets/campfire_north.png");
// ... load other images
```

**Step 2: Draw Images in campfire_draw_callback()**

Replace text rendering with:
```c
// Draw fire in center
canvas_draw_icon(canvas, 55, 32, fire_img);  // Centered at 58, 35

// Draw north player icon
if(campfire_slots[CAMP_NORTH].active) {
    canvas_draw_icon(canvas, 50, 15, player_north_img);
    canvas_draw_str(canvas, 60, 23, campfire_slots[CAMP_NORTH].name);  // Name below icon
}
// ... repeat for E, W, S
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
- **Use color shifts** - Not just size changes, vary orange/red intensity
- **Add details** - Embers, smoke wisps make it feel alive

### Player Icons
- **Make them distinctive** - Different symbols for different aura types?
  - Flame aura: Fire symbol ✦
  - Whispered aura: Wind/ripple symbol ≈
  - Storm aura: Lightning symbol ⚡
  - Echo aura: Circle/ripple symbol ◉
- **Keep it simple** - Flipper screen is small, 8x8 pixels is tiny
- **Test at actual size** - View in Aseprite at 100% zoom as it would appear

### Animation
- **Keep frame count low** - More frames = larger file size
- **Consistent timing** - Use same speed for all animations (150ms = good default)
- **Subtle is better** - Don't make things too flashy; it should complement gameplay

### Color Palette
Recommended colors for Flipper (monochrome-friendly):
- Fire: Use warm colors, but test on actual Flipper screen
- Players: High contrast for visibility
- Glow/Selection: Bright highlight color

---

## Testing Your Assets

### Visual Preview
1. Export PNG from Aseprite
2. Open in image viewer at actual size
3. Check visibility and clarity

### In-Game Testing
1. Place PNG in `assets/` folder
2. Update `FlippeRPG.c` to load and render it
3. Build: `./fbt.cmd fap_flipperpg`
4. Deploy to Flipper and test campfire scene
5. Verify animation speed and visibility

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

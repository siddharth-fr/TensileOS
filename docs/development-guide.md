# TensileOS Application Development Guide

This guide explains how to create your own custom applications, diagnostic tools, or retro games for TensileOS and integrate them into the Classic Desktop desktop environment.

---

## 1. The `App` Base Class

Every application in TensileOS inherits from the `App` abstract class defined in [`tensileos/core/ui.h`](../tensileos/core/ui.h). 

```cpp
#pragma once
#include "core/ui.h"

class MyApp : public App {
 public:
  MyApp() {
    prefW = 40;     // Preferred width in text columns (cols * 6 + 10 pixels)
    prefH = 15;     // Preferred height in text rows (rows * 8 + 30 pixels)
    tickMs = 50;    // Background timer tick interval (0 = no timer ticks)
  }

  // Window title shown on the 3D titlebar and taskbar tab
  const char* title() override { return "My App"; }

  // Called to render application content into the window client area
  void draw(View& v) override;

  // Called when a key is pressed while the window has focus
  void key(uint8_t k) override;

  // Called when the mouse is clicked or moved in client pixel coordinates
  void mousePixel(int px, int py, uint8_t b, uint8_t prev) override;

  // Called on background timer tick
  void tick(uint32_t now) override;
};
```

---

## 2. Rendering Content into `View& v`

The `View` struct passed to `draw(View& v)` provides dimensions and drawing primitives. It supports both high-level text helpers and low-level pixel rasterization:

### Coordinates & Dimensions
* `v.px`, `v.py`: Top-left screen pixel coordinates of the client area.
* `v.pw`, `v.ph`: Width and height of the window client area in pixels.
* `v.w`, `v.h`: Width and height in 6×8 text character cells.
* `v.focused`: Boolean indicating whether this window currently has active focus.

### Available Drawing Primitives (`gfx`)
When rendering pixel graphics, use the global `gfx` rasterizer instance:

```cpp
void MyApp::draw(View& v) {
  // 1. Fill the window background with a 16-color VGA palette color
  gfx.fillRect(v.px, v.py, v.pw, v.ph, COL_SILVER);

  // 2. Draw a 3D beveled box (sunken canvas or raised panel)
  gfx.drawBevelBox(v.px + 4, v.py + 4, v.pw - 8, v.ph - 8, true);

  // 3. Draw crisp 6x8 retro monospace text
  gfx.drawText6x8(v.px + 10, v.py + 10, "Hello, TensileOS!", COL_BLACK, COL_SILVER);

  // 4. Draw geometric shapes
  gfx.drawLine(v.px + 10, v.py + 25, v.px + 100, v.py + 25, COL_BLUE);
  gfx.drawRect(v.px + 10, v.py + 35, 40, 20, COL_NAVY);

  // 5. Draw 3D push buttons
  gfx.drawButton(v.px + 10, v.py + 65, 60, 18, "Click Me", false);
}
```

### Color Palette Constants (`WinColor` Enum)
| Color Constant | Value | RGB Preview | Typical Usage |
| :--- | :---: | :---: | :--- |
| `COL_BLACK` | 0 | `#000000` | Text, borders, deep shadows |
| `COL_NAVY` | 1 | `#000080` | Active titlebar, selection highlights |
| `COL_GREEN` | 2 | `#008000` | Oscilloscope graticule, Snake body |
| `COL_TEAL` | 3 | `#008080` | Classic Desktop desktop background |
| `COL_MAROON` | 4 | `#800000` | Warning badges |
| `COL_SILVER` | 7 | `#C0C0C0` | Window frames, buttons, taskbar |
| `COL_GREY` | 8 | `#808080` | Inactive titlebars, 3D inner shadows |
| `COL_BLUE` | 9 | `#0000FF` | Hyperlinks, progress bars |
| `COL_LIME` | 10 | `#00FF00` | CRT oscilloscope trace, LED indicators |
| `COL_CYAN` | 11 | `#00FFFF` | Diagnostic readouts |
| `COL_RED` | 12 | `#FF0000` | Close button, Minesweeper boom |
| `COL_YELLOW`| 14 | `#FFFF00` | Minesweeper smiley face |
| `COL_WHITE` | 15 | `#FFFFFF` | 3D light highlights, window interiors |

---

## 3. Handling User Input

### Keyboard Events
```cpp
void MyApp::key(uint8_t k) {
  if (k == KEY_ENTER) {
    // User pressed Enter
  } else if (k == KEY_ESC) {
    closeReq = true; // Signals window manager to close this window
  } else if (k >= 32 && k <= 126) {
    // Printable ASCII character
  }
}
```

### Mouse Events
Client mouse coordinates are normalized relative to the window's client area (`px, py` from `0` to `v.pw - 1`, `v.ph - 1`):

```cpp
void MyApp::mousePixel(int px, int py, uint8_t b, uint8_t prev) {
  uint8_t edge = b & ~prev; // Button press down edge

  // Check if button was clicked
  if (edge & 1) { // Left click
    if (px >= 10 && px <= 70 && py >= 65 && py <= 83) {
      soundClick(); // Plays authentic tactile click sound
      // Handle button action
    }
  }
}
```

---

## 4. Registering Your App in the Window Manager

To make your app accessible from the Start Menu, desktop icons, or command prompt:

1. Open [`tensileos/display/desktop_wm.h`](../tensileos/display/desktop_wm.h).
2. Include your app header at the top:
   ```cpp
   #include "apps/myapp.h"
   ```
3. In `TensileDesktop::launch(const char* id, const char* arg)`:
   ```cpp
   else if (!strcmp(id, "myapp")) a = new MyApp();
   ```
4. Add it to the Start Menu items array in `drawStartMenu()` and `onMouse()`.

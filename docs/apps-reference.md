# TensileOS Applications and Tools Reference Manual

TensileOS ships with 16 native applications, diagnostic utilities, productivity tools, and retro games. Every program is written in bare-metal C++, compiled directly for the ESP32 Xtensa dual-core processor, and executes 100% on microcontroller silicon with zero emulation.

This manual provides a technical reference for each application, covering its launch commands, user controls, LittleFS flash persistence, and hardware resource footprint.

---

## Application Matrix

| Application | Domain | Binary Identifier | LittleFS Persistence | Hardware Pins / Resources |
| :--- | :--- | :--- | :--- | :--- |
| **Hardware Lab** | Engineering & Diagnostics | `hwlab` | None | ADC1 (GPIO 34/35), GPIO 2, 4, 18, 19, 21, 22 |
| **SysBench** | Engineering & Diagnostics | `sysbench` | Benchmark Read/Write | CPU, DRAM, Flash bus |
| **Tensile Paint** | Creative & Graphics | `paint` | Read/Write (`/PAINT/*.DAT`) | Framebuffer heap (12.4 KB canvas) |
| **Notepad** | Productivity & Text | `notepad` / `editor` | Read/Write (`*.TXT`, `*.BAT`, `*.INI`) | Text stream buffer |
| **File Manager** | Virtual Disk & Storage | `filemgr` | Read/Write/Delete (`C:\`) | LittleFS partition driver |
| **Scientific Calculator** | Mathematics & Compute | `calc` | None | Math runtime |
| **Minesweeper** | Entertainment | `mine` | High Score | Audio synthesizer |
| **Tetris** | Entertainment | `tetris` | High Score | Audio synthesizer |
| **Snake** | Entertainment | `snake` | High Score | Audio synthesizer |
| **System Monitor** | System Management | `sysmon` | None | FreeRTOS heap allocator, uptime timer |
| **My Computer** | System Management | `mycomp` | Read (`C:\` stats) | LittleFS partition statistics |
| **Recycle Bin** | Virtual Disk & Storage | `trash` | Read/Write/Delete (`/TRASH`) | LittleFS directory management |
| **Date & Time Properties** | System Configuration | `timeset` | Active RTC state | Software RTC timer |
| **Command Prompt** | Shell & CLI Interpreter | `cmd` / `shell` | LittleFS directory traversal | Terminal scrollback buffer |
| **Batch Script Runner** | Automation & Scripting | System service | Reads `/AUTOEXEC.BAT` | Shell command parser |
| **Network Suite** | Communications & Web | `netmgr` / `browser` | Network state | ESP32 802.11 b/g/n Wi-Fi stack |

---

## 1. Engineering and Diagnostic Tools

### Hardware Lab (Oscilloscope and Logic Analyzer)
Hardware Lab turns the ESP32 into an on-screen electronics testbench with a dual-trace CRT oscilloscope and a 6-channel digital logic controller.

* **Launch Methods:**
  * Start Menu: `Start -> Programs -> Accessories -> Hardware Lab`
  * Desktop: Double-click the **Hardware Lab** icon
  * Command Prompt: `C:\> hwlab`
* **Features:**
  * **CRT Display:** Phosphor green graticule with calibrated grid divisions and real-time waveform trace.
  * **6 Input Signal Channels:**
    * `ADC 34`: Live analog voltage from physical pin GPIO 34 (ADC1 Channel 6).
    * `ADC 35`: Live analog voltage from physical pin GPIO 35 (ADC1 Channel 7).
    * `Sine 1kHz`: Synthetic 1 kHz test sinusoid.
    * `Square`: High-speed clock waveform.
    * `Triangle`: Linear ramp generator.
    * `ECG`: Simulated cardiac electrocardiogram waveform.
  * **Signal Measurement Telemetry:** Real-time peak-to-peak voltage ($V_{\text{pp}}$), maximum voltage ($V_{\text{max}}$), minimum voltage ($V_{\text{min}}$), sweep period, and trigger frequency.
  * **Logic Analyzer Switches:** 6 interactive push-toggles wired to GPIO 2, 4, 18, 19, 21, and 22. Toggling a switch drives the physical pin HIGH (3.3V) or LOW (0V) and updates the on-screen green status indicator.
* **Controls:**
  * Click channel tabs to cycle active oscilloscope signal sources.
  * Click individual GPIO toggle buttons to switch logic outputs.

### SysBench (Hardware Benchmark Suite)
SysBench measures raw ESP32 microcontroller performance across five critical computational subsystems, outputting quantitative scores and hardware thermal metrics.

* **Launch Methods:**
  * Start Menu: `Start -> Programs -> Accessories -> SysBench`
  * Command Prompt: `C:\> sysbench`
* **Test Suites:**
  * **CPU Integer Arithmetic:** Runs a 10,000-iteration prime sieve calculation using the Sieve of Eratosthenes. Measures raw integer calculation speed in milliseconds.
  * **Floating-Point Processing:** Computes multi-term Taylor series approximations and matrix transformations, reporting single-precision floating-point throughput in MFLOPS.
  * **RAM Memory Bandwidth:** Executes sequential 32-bit block reads and writes across SRAM, measuring throughput in megabytes per second (MB/s).
  * **LittleFS Flash I/O:** Writes and reads back a 4 KB test payload to onboard SPI flash memory, benchmarking filesystem read and write latency.
  * **Text Rasterizer Throughput:** Measures CP437 character blitting speed directly into the 4-bit packed framebuffer, reporting rendering frames per second.
* **Controls:**
  * Click **Run Benchmark** to begin the automated test run.
  * Results remain on screen until cleared or re-tested.

---

## 2. Creative and Productivity Applications

### Tensile Paint (2D Pixel Art Studio)
A dedicated pixel art creation suite tailored to the 16-color retro palette. Paint features a 210x118 canvas buffer stored in dedicated packed memory, providing responsive, latency-free drawing.

* **Launch Methods:**
  * Start Menu: `Start -> Programs -> Accessories -> Paint`
  * Desktop: Double-click the **Paint** icon
  * Command Prompt: `C:\> paint`
* **Tools and Capabilities:**
  * **Tool Palette:** Pen (freehand single-pixel), Brush (multi-pixel circular head), Eraser (replaces pixels with the background color), Flood Fill (scanline-based contiguous region fill), and Line (Bresenham line rasterizer).
  * **Brush Sizes:** 1-pixel fine tip, 3-pixel medium round, and 5-pixel broad marker.
  * **Color Palette:** 16-color VGA swatch selector. Left-clicking a swatch assigns the Foreground color; right-clicking assigns the Background color.
  * **Canvas Coordinate Readout:** Live $X, Y$ coordinate indicator in the bottom status bar tracking cursor location.
* **File Persistence:**
  * Click **Save** to write the canvas image directly to LittleFS at `/PAINT/ART1.DAT`.
  * Click **Open** to reload saved `.DAT` artwork from flash.
* **Controls:**
  * `Left Click + Drag`: Draw using the active tool and Foreground color.
  * `Right Click + Drag`: Draw or erase using the Background color.
  * `Click Palette Swatch`: Select drawing colors.

### Notepad (Full-Screen Text Editor)
A lightweight text editor for writing notes, drafting configuration files, and managing batch startup scripts on Drive `C:\`.

* **Launch Methods:**
  * Start Menu: `Start -> Programs -> Accessories -> Notepad`
  * Desktop: Double-click the **Notepad** icon
  * Command Prompt: `C:\> notepad [FILENAME]` or `C:\> editor [FILENAME]`
* **Features:**
  * Full-screen editing area with 6x8 monospace typography.
  * Cursor navigation across rows and columns.
  * LittleFS flash file integration: directly reads and writes files such as `/README.TXT`, `/AUTOEXEC.BAT`, and `/SYSTEM.INI`.
* **Controls:**
  * Standard typing on any physical keyboard attached to your workstation.
  * `Enter`: Insert newline.
  * `Backspace`: Delete previous character.
  * Menu bar actions: **File -> Save**, **File -> Open**, **File -> Exit**.

---

## 3. Virtual Disk and Storage Management

### File Manager
A dual-panel directory explorer for organizing, inspecting, and maintaining files across the LittleFS partition mounted at Drive `C:\`.

* **Launch Methods:**
  * Start Menu: `Start -> Programs -> Accessories -> File Manager`
  * Desktop: Double-click the **File Manager** icon
  * Command Prompt: `C:\> filemgr`
* **Features:**
  * Displays file names, extensions, byte sizes, and directory structures.
  * Create new files and folders.
  * Rename existing files in place.
  * Delete obsolete files and move items into the Recycle Bin staging directory.
* **Controls:**
  * Double-click a file entry to view or edit it in Notepad.
  * Click toolbar buttons to create, rename, or delete items.

### My Computer
A system storage and hardware inspector providing an overview of microcontroller hardware resources and storage utilization.

* **Launch Methods:**
  * Desktop: Double-click the **My Computer** icon
  * Start Menu: `Start -> Programs -> System Tools -> My Computer`
* **Features:**
  * Displays Drive `C:\` total flash capacity, used space, and free space in kilobytes.
  * Displays CPU model (Espressif ESP32 dual-core Xtensa LX6 @ 240 MHz).
  * Reports total physical SRAM and operating system framebuffer allocation.

### Recycle Bin
A safe deletion staging utility. Rather than permanently destroying flash blocks immediately, files deleted through graphical interfaces can be moved to `/TRASH`.

* **Launch Methods:**
  * Desktop: Double-click the **Recycle Bin** icon
  * Start Menu: `Start -> Programs -> System Tools -> Recycle Bin`
* **Features:**
  * Inspect files staged for deletion.
  * Empty the Recycle Bin to reclaim LittleFS flash blocks permanently.
  * Restore items back to their original paths on Drive `C:\`.

---

## 4. Mathematics and Computation

### Scientific Calculator
A dual-mode standard and scientific calculator featuring an expression evaluator with operator precedence.

* **Launch Methods:**
  * Start Menu: `Start -> Programs -> Accessories -> Calculator`
  * Desktop: Double-click the **Calculator** icon
  * Command Prompt: `C:\> calc`
* **Capabilities:**
  * **Arithmetic Operations:** Addition, subtraction, multiplication, division, and modulo.
  * **Scientific Functions:** Trigonometric operators (`sin`, `cos`, `tan`), natural logarithm (`ln`), base-10 logarithm (`log`), square root (`sqrt`), and arbitrary powers (`^`).
  * **Mathematical Constants:** Single-click insertion of Pi ($\pi \approx 3.14159$) and Euler's Number ($e \approx 2.71828$).
  * **Parentheses:** Nested parentheses evaluation with full operator precedence.
* **Controls:**
  * Click on-screen numeric keypad buttons with the mouse, or enter equations directly via keyboard.

---

## 5. Retro Gaming Suite

### Minesweeper
An authentic recreation of the classic puzzle game with 3D sunken tiles, digital LED readouts, and an interactive reset smiley button.

* **Launch Methods:**
  * Start Menu: `Start -> Programs -> Games -> Minesweeper`
  * Desktop: Double-click the **Minesweeper** icon
  * Command Prompt: `C:\> mine`
* **Features:**
  * 3D beveled grid with concealed tiles, numbered neighbor indicators, and concealed explosive mines.
  * 7-segment digital displays showing remaining unflagged mine count and elapsed game time in seconds.
  * Yellow reset smiley button indicating game state: smiling while playing, surprised during clicks, sunglasses on victory, and crosses on defeat.
  * WebAudio square-wave sound effects for tile reveals, flag placements, and game-ending explosions.
* **Controls:**
  * `Left Click`: Reveal tile. Revealing an empty tile triggers a recursive cascade opening all connected blank spaces.
  * `Right Click`: Place or remove a 3D hazard warning flag.
  * `Click Smiley` or press `R`: Start a fresh game with a newly randomized minefield.

### Tetris
A 16-color implementation of the classic falling-block game with 3D beveled tetrominoes, drop scoring, and level progressions.

* **Launch Methods:**
  * Start Menu: `Start -> Programs -> Games -> Tetris`
  * Command Prompt: `C:\> tetris`
* **Features:**
  * 7 standard tetromino shapes (I, J, L, O, S, T, Z) rendered with authentic 3D light and shadow bevels.
  * Next piece preview window and score counter.
  * Line clear audio tone sequences synthesized on the fly.
* **Controls:**
  * `Left / Right Arrow`: Shift active block horizontally.
  * `Up Arrow`: Rotate piece clockwise.
  * `Down Arrow`: Soft drop (accelerated descent).
  * `Spacebar`: Hard drop (instantaneous lock).
  * `P`: Pause game.
  * `R`: Reset game.

### Snake
A retro arcade action game with 3D beveled snake segments, target apples, and persistent high score tracking.

* **Launch Methods:**
  * Start Menu: `Start -> Programs -> Games -> Snake`
  * Command Prompt: `C:\> snake`
* **Features:**
  * Responsive 4-directional steering with wall collision and self-intersection detection.
  * Food generation with score increments and speed scaling.
  * Speaker audio chimes on pickup and collision.
* **Controls:**
  * `Arrow Keys` or `W, A, S, D`: Change direction.
  * `P`: Pause game.
  * `R`: Restart game.

---

## 6. Systems, Network, and Shell Applications

### System Monitor (SysMon)
A real-time diagnostic dashboard displaying live resource telemetry from the ESP32 silicon and FreeRTOS kernel.

* **Launch Methods:**
  * Start Menu: `Start -> Programs -> System Tools -> System Monitor`
  * Command Prompt: `C:\> sysmon`
* **Telemetry Gauges:**
  * **Free Heap DRAM:** Live visual progress bar and numeric kilobyte readout of available internal memory.
  * **Dual-Core CPU Utilization:** Graph tracking CPU activity across Core 0 (Network) and Core 1 (OS/GUI).
  * **LittleFS Storage Allocation:** Partition capacity gauge showing used vs. free flash storage.
  * **System Uptime:** Continuous timer tracking hours, minutes, and seconds since microcontroller boot.
  * **Wi-Fi Signal Telemetry:** Connection status, RSSI signal strength, and assigned IP address.

### Date and Time Properties
An interactive dialog for reading and adjusting the operating system real-time clock.

* **Launch Methods:**
  * Desktop: Double-click the clock in the bottom-right system tray
  * Start Menu: `Start -> Settings -> Date & Time`
  * Command Prompt: `C:\> timeset`
* **Features:**
  * Analog clock dial showing live hour, minute, and second hands.
  * Numeric spin-boxes for hours and minutes.
  * Adjusting the clock immediately updates the system tray display and filesystem timestamps.

### Command Prompt
An interactive text terminal operating inside a standard 3D desktop window, providing direct command-line access to internal OS utilities and virtual disk files.

* **Launch Methods:**
  * Start Menu: `Start -> Command Prompt`
  * Desktop: Double-click the **Command Prompt** icon
  * Command Prompt invocation: `C:\> cmd`
* **Available Commands:**
  * `HELP` or `?`: Print the list of internal commands.
  * `DIR`: List directory contents with file sizes.
  * `MEM`: Report free FreeRTOS heap memory.
  * `VER`: Output TensileOS release information.
  * `TIME [HH:MM]`: Display or adjust system time.
  * `CLS`: Clear screen buffer.
  * `REBOOT`: Restart the ESP32 hardware.
* For complete syntax and batch script options, consult the [CLI Reference](cli-reference.md).

### Batch Script Runner (`AUTOEXEC.BAT`)
An automated execution service that initializes system settings upon boot.
* At system startup, TensileOS checks for `/AUTOEXEC.BAT` in the LittleFS root.
* If present, the script runner parses each line sequentially, running built-in commands (such as setting the clock or printing system parameters) before launching the graphical desktop.

### Network Suite (Network Manager and RetroWeb)
Utilities for scanning 802.11 b/g/n networks, managing wireless associations, and rendering basic web pages.

* **Network Manager:**
  * Start Menu: `Start -> Settings -> Network`
  * Scans for local Wi-Fi Access Points, lists SSIDs, RSSI signal levels, and encryption types, and provides connection controls.
* **RetroWeb:**
  * Start Menu: `Start -> Programs -> Accessories -> RetroWeb`
  * Desktop: Double-click the **Web Browser** icon
  * A lightweight text-mode HTML browser designed for reading plain web documents, documentation files, and local HTTP endpoints.

---

## Technical Summary for Developers

Developers can easily create and register new applications using the TensileOS `App` base class. Every application receives window client dimensions through `View& v`, handles mouse and keyboard input callbacks, and draws into the 4-bit packed framebuffer using fast SIMD drawing primitives.

For a complete step-by-step tutorial on writing a new app, see the [Application Development Guide](development-guide.md).

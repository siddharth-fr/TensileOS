# TensileOS CLI and Command Prompt Reference

TensileOS features an interactive Command Prompt terminal accessible via the Start Menu (`Start -> Command Prompt`) or desktop icon. It provides a command interpreter for querying system status, inspecting virtual disk storage on Drive `C:\`, managing the real-time clock, and launching batch automation scripts.

---

## 1. Built-in Commands

| Command | Syntax | Description | Example |
| :--- | :--- | :--- | :--- |
| **`HELP`** or **`?`** | `HELP` | Displays a list of all available internal commands. | `C:\> HELP` |
| **`VER`** | `VER` | Displays the TensileOS release version. | `C:\> VER` |
| **`MEM`** | `MEM` | Reports available FreeRTOS heap memory in kilobytes. | `C:\> MEM` |
| **`DIR`** | `DIR` | Lists files and subdirectories on the active drive (`C:\`) along with file sizes. | `C:\> DIR` |
| **`TIME`** | `TIME [HH:MM]` | Displays current software RTC time, or updates it if hours and minutes are provided. | `C:\> TIME 13:45` |
| **`CLS`** | `CLS` | Clears the terminal screen buffer and resets the cursor to the top. | `C:\> CLS` |
| **`REBOOT`** | `REBOOT` | Triggers a hardware restart of the ESP32 microcontroller. | `C:\> REBOOT` |

---

## 2. Application Launch Commands

In addition to internal shell utilities, you can launch any of TensileOS's graphical applications directly from the terminal prompt:

| Application | Command | Description |
| :--- | :--- | :--- |
| **Hardware Lab** | `hwlab` | Opens the dual-channel CRT oscilloscope and logic controller |
| **SysBench** | `sysbench` | Launches the hardware benchmarking suite |
| **Tensile Paint** | `paint` | Opens the 16-color 2D pixel art studio |
| **Notepad** | `notepad [FILE]` | Opens the text editor (optionally loading a file from `C:\`) |
| **File Manager** | `filemgr` | Launches the LittleFS directory and file explorer |
| **Calculator** | `calc` | Opens the scientific expression calculator |
| **TensileDoom** | `doom` | Launches the 2.5D Doom raycasting first-person shooter |
| **Minesweeper** | `mine` | Launches the 3D beveled Minesweeper game |
| **Tetris** | `tetris` | Launches the classic block falling puzzle game |
| **Snake** | `snake` | Launches the arcade snake action game |
| **System Monitor** | `sysmon` | Opens live CPU, heap DRAM, and uptime telemetry |

For a complete manual of all applications and their controls, see the [Applications Reference](apps-reference.md).

---

## 3. Detailed Command Usage

### `DIR`: Directory Listing
Lists the contents of the LittleFS flash storage partition mounted at `C:\`.
```
C:\> DIR
Directory of C:\ (4 files):
README.TXT      1971
AUTOEXEC.BAT     142
ART1.DAT       12390
SYSTEM.INI       512
```

### `MEM`: Memory Inspection
Queries the internal memory allocator (`ESP.getFreeHeap()`) and displays available RAM:
```
C:\> MEM
Free Heap: 240 KB
```

### `TIME`: Clock Query and Setting
* **View current time:**
  ```
  C:\> TIME
  Current time is 14:02:45
  ```
* **Set system clock:**
  ```
  C:\> TIME 09:15
  Time set to 09:15:00
  ```
  The system tray clock in the bottom-right corner updates immediately.

### `VER`: OS Version Information
```
C:\> VER
TensileOS v0.2.0 Classic Desktop Edition
```

---

## 4. Batch Automation: `AUTOEXEC.BAT`
TensileOS checks for the existence of `/AUTOEXEC.BAT` in the root of LittleFS at boot. If found, it parses and executes each line sequentially before loading the graphical window manager.

### Example `AUTOEXEC.BAT`:
```bat
REM TensileOS Startup Script
TIME 12:00
VER
MEM
```

You can view or edit `/AUTOEXEC.BAT` directly inside TensileOS using the **Notepad** text editor.

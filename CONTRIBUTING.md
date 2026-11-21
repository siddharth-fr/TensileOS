# Contributing to TensileOS

Thank you for your interest in contributing to TensileOS! We welcome contributions from developers, designers, retro enthusiasts, and embedded systems hackers.

---

## 1. Development Principles

- **Silicon First**: All logic runs directly on the ESP32. The browser is purely a thin display and input conduit.
- **Embedded Constraints**: TensileOS runs with ~200 KB free internal DRAM. Avoid large heap allocations in the rendering loop; prefer static buffers and stack memory.
- **Modularity**: Place new features in their respective directories (`apps/`, `games/`, `shell/`, `display/`, `core/`, `net/`).
- **Include Guards**: Every header file must use standard guards: `#ifndef TENSILE_<SUBDIR>_<NAME>_H` and `#define TENSILE_<SUBDIR>_<NAME>_H`.

---

## 2. Setting Up Your Environment

1. Install Arduino CLI or Arduino IDE 2.x.
2. Install the ESP32 core (`esp32:esp32` version 2.0.x or 3.x).
3. Ensure required libraries are installed:
   - `AsyncTCP`
   - `ESPAsyncWebServer`
   - `ArduinoJson`
4. Set board partition scheme to `huge_app` (3MB No OTA / 1MB SPIFFS).

---

## 3. Pull Request Guidelines

1. **Branching**:
   - Create feature or fix branches from `dev`: `git checkout -b feat/my-new-app` or `git checkout -b fix/window-flicker`.
   - Never commit directly to `main`.
2. **Commit Conventions**: Use Conventional Commits:
   - `feat:` for new capabilities or apps.
   - `fix:` for bug fixes.
   - `perf:` for performance optimizations.
   - `refactor:` for code restructurings without behavior changes.
   - `docs:` for documentation updates.
3. **Verification**: Verify that the sketch compiles cleanly before submitting:
   ```bash
   arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app tensileos
   ```
4. **Documentation**: Update `CHANGELOG.md` and relevant docs under `docs/` when adding major features or commands.

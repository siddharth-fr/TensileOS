# TensileOS Changelog

All notable changes to this project are documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.0] - 2026-11-21

### Added
- Modular architecture splitting system into `config/`, `core/`, `display/`, `fs/`, `net/`, `shell/`, `apps/`, and `games/`.
- Fast SIMD 32-bit memset rasterizer routines in `pixel_gfx.h`.
- Double-buffered wireframe window dragging at 60 FPS.
- WebSocket frame delta compression reducing Wi-Fi bandwidth by up to 80%.
- Custom flash filesystem with wear leveling and path resolution (`TensileFS`).
- Retro sound synthesizer engine with square wave frequency generator.
- Multi-app workspace including Text Editor, Paint, Hardware Apps, and Retro Games.
- Comprehensive technical documentation in `docs/` (`architecture.md`, `getting-started.md`, etc.).

### Changed
- Refactored monolithic firmware into clean modular C++ headers with standardized include guards.
- Renamed project to TensileOS.
- Partition scheme upgraded to `huge_app` (3.14MB app partition) for extended flash headroom.

## [0.1.0] - 2026-11-18

### Added
- Initial proof of concept desktop streaming over WebSocket.
- Basic retro window manager and mouse input handler.
- Terminal shell with core DOS-like commands.
- Standalone HTML5 client for browser-based display streaming.

## Description
Briefly describe the changes introduced by this pull request.

## Type of Change
- [ ] Bug fix (`fix:`)
- [ ] New feature or app (`feat:`)
- [ ] Performance optimization (`perf:`)
- [ ] Refactoring (`refactor:`)
- [ ] Documentation update (`docs:`)

## Verification Checklist
- [ ] Code builds cleanly with `arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app tensileos`
- [ ] Tested on physical ESP32 hardware
- [ ] Follows project header guard conventions (`TENSILE_<DIR>_<NAME>_H`)
- [ ] Zero dynamic heap leaks in OS frame loop
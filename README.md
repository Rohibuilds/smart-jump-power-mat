# Smart Jump Power Mat
**Rohi | RA TECH** · Robotics, electronics and embedded systems

An ESP32 load-cell jump game with a rocket on an ST7735 TFT and a live phone dashboard.

**[Open the complete code](firmware/Smart_Jump_Mat_Final/Smart_Jump_Mat_Final.ino) · [Wiring and parts](hardware/README.md) · [How to build](docs/README.md)**

## What is included
Startup tare, peak capture, game scores and best score, animated rocket, Wi-Fi dashboard, empty-mat re-tare and fallback access point.

This repository restores the previously delivered project source and repairs identified software issues. It is a hardware prototype; source restoration does not constitute a new hardware test.

## Get started
1. Download the repository using **Code → Download ZIP** and extract it.
2. Read the [wiring table](hardware/README.md); it retains the recovered pin map.
3. Follow the [build and configuration guide](docs/README.md).
4. Open `firmware/Smart_Jump_Mat_Final/Smart_Jump_Mat_Final.ino` in Arduino IDE. Keep the containing folder and companion headers together.

## Code and validation
- Main source: [Smart_Jump_Mat_Final.ino](firmware/Smart_Jump_Mat_Final/Smart_Jump_Mat_Final.ino)
- [Dependency versions](DEPENDENCIES.md)
- [Fixes and validation record](docs/VALIDATION.md)
- [Build workflow](.github/workflows/build.yml) / [live build results](https://github.com/Rohibuilds/smart-jump-power-mat/actions)

## Source provenance
Recovered from the earlier RA TECH deliverables `Smart_Jump_Mat_Final.ino and Smart_Jump_Mat_Wiring_Parts_Working_Guide.pdf`. The wiring was cross-checked against those files. This update preserves the project's original purpose and identifies later repairs separately.

## Recent repairs
Used 64-bit calibration sums; rejected failed calibration and stale HX711 readings; bounded jump events; made rocket timing independent of loop speed; fixed deadline rollover handling; added connection-error feedback and manual re-tare. Original tuning constants remain configurable, not physical force units.

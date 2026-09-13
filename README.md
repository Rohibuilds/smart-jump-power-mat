<div align="center">

# Smart Jump Power Mat

**ESP32 load-cell platform for jump analysis, scoring and real-time visual feedback.**

![Status](https://img.shields.io/badge/status-working_prototype-00D9A5?style=flat-square)
![Platform](https://img.shields.io/badge/platform-ESP32_%C2%B7_Sensors-101820?style=flat-square)
![Brand](https://img.shields.io/badge/by-RA_TECH-101820?style=flat-square)

</div>

## Overview

A smart exercise mat that uses a load cell and HX711 amplifier to detect standing load and jump peaks. The system turns measurements into an interactive rocket game while presenting results on a TFT display and phone dashboard.

> **Project status:** Working prototype

## Highlights

- Load-cell-based jump detection
- HX711 signal smoothing
- Standing and jump-state recognition
- Interactive rocket scoring game
- ST7735 TFT feedback
- Phone-accessible analysis dashboard

## Hardware

| Component | Role |
|---|---|
| ESP32 development board | Main processing and control |
| HX711 load-cell amplifier | Project subsystem |
| Load cell or load-cell platform | Project subsystem |
| 1.8-inch ST7735 TFT | Project subsystem |
| Stable power supply | Project subsystem |
| Mechanical mat or platform | Project subsystem |

## Repository structure

```text
smart-jump-power-mat/
├── firmware/   Tested source code and configuration notes
├── hardware/   Wiring, components, PCB, and enclosure information
├── docs/       Build guide, calibration, results, and troubleshooting
├── media/      Prototype images, diagrams, and demo links
└── README.md   Project overview and release status
```

## Current public release

This initial release establishes the verified project overview and a clean documentation structure. Firmware, wiring diagrams, and media will be added only after each item is checked for accuracy and private credentials are removed.

## Roadmap

- [ ] Publish the calibrated firmware
- [ ] Add the final load-cell mounting design
- [ ] Document calibration for different users
- [ ] Add repeatability tests and demo media

## Safety and reproducibility

- Verify every supply voltage before powering the controller or modules.
- Use a common ground and a power source sized for peak motor or audio current.
- Never commit Wi-Fi passwords, API keys, personal contact details, or certificates.
- Recheck the published pin map against the tested hardware before assembly.

---

<div align="center">

**Designed and developed by [Rohi · RA TECH](https://github.com/Rohibuilds)**

<sub>Build. Test. Improve. Share.</sub>

</div>

# Repair and validation record
## Source baseline
Recovered `Smart_Jump_Mat_Final.ino and Smart_Jump_Mat_Wiring_Parts_Working_Guide.pdf`. GPIO mappings are unchanged.

## Repairs
Used 64-bit calibration sums; rejected failed calibration and stale HX711 readings; bounded jump events; made rocket timing independent of loop speed; fixed deadline rollover handling; added connection-error feedback and manual re-tare. Original tuning constants remain configurable, not physical force units.

## Checks
- Source and wiring were compared; the repository contains the actual sketch and needed project headers.
- The automated workflow targets Espressif core 3.3.0 with pinned external libraries.
- Check the [exact GitHub Actions result](https://github.com/Rohibuilds/smart-jump-power-mat/actions) before treating a revision as compile-verified.
- Physical sensor behavior, power, audio, radio connectivity and calibration have not been retested on Rohi's hardware in this update.

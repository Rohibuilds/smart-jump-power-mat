# Build and calibrate the Jump Mat
1. Assemble the load-cell platform according to the cells' mechanical specification. Check the complete bridge with a meter before connecting the HX711.
2. Connect the [unchanged pin map](../hardware/README.md). Disconnect power while wiring.
3. Install Espressif **3.3.0** and the [specified libraries](../DEPENDENCIES.md). Select **DOIT ESP32 DEVKIT V1** or the compatible **ESP32 Dev Module** for your board.
4. Open `firmware/Smart_Jump_Mat_Final/Smart_Jump_Mat_Final.ino`. Copy `secrets.example.h` to `secrets.h`; set Wi-Fi credentials and a private fallback AP password of at least 8 characters.
5. Verify and upload. Keep the mat **empty during boot calibration**. Open Serial Monitor at **115200 baud** and confirm a valid baseline. If HX711 calibration fails, the game reports SENSOR ERROR instead of accepting fake data.
6. Connect a phone to the same Wi-Fi and open the IP printed in Serial Monitor. If router connection fails, join **RA-TECH-Jump-Mat** with your configured AP password and open **http://192.168.4.1**.
7. Inspect empty, gently loaded and standing raw readings before attempting a jump. `abs(raw - baseValue)` is the baseline-subtracted signal. Original constants are retained: noise floor 80,000; jump threshold 110,000; great 180,000; space 280,000; map maximum 320,000. These are counts, not force units, and may need adjustment for the actual bridge and mounting.
8. Set `JUMP_THRESHOLD` above steady standing readings with a useful margin; set GREAT/SPACE/MAX progressively higher while maintaining `NOISE_FLOOR < JUMP_THRESHOLD < GREAT_THRESHOLD < SPACE_THRESHOLD < MAX_FORCE_MAP`. A threshold below standing load will trigger repeated events. Do not copy someone else's scale values blindly.
9. Test peak capture and score return with gentle loading first. Tare again only with an empty mat using **Tare empty mat**. This resets the current event; the best score remains for the session.
10. Confirm the TFT rocket follows the score, the dashboard updates, and disconnecting HX711 produces SENSOR ERROR. Reconnect the sensor and perform an empty-mat tare before resuming.

## Troubleshooting
- **No readings:** check 3V3/common ground, DT4/SCK5, bridge continuity and channel A.
- **Negative raw change:** the game uses absolute baseline difference; verify bridge polarity and mechanical mounting rather than treating sign alone as a fault.
- **Instant maximum score:** thresholds do not match your loaded signal; tune the count constants.
- **Slow/missed peaks:** HX711 conversion rate and mounting limit resolution. 10 SPS can miss short landing peaks; 80 SPS requires a supported module RATE setting and rechecking noise.
- **Blank TFT:** confirm ST7735 controller, black-tab initialization, SPI18/23 and CS15/DC2/RST27.
- **No Wi-Fi:** verify credentials, 2.4 GHz coverage or fallback AP connection. A disconnected page explicitly shows that the last values are stale.

This is a game/engineering prototype. Any physical measurement claim needs an independently calibrated sensor and appropriate mechanical validation.

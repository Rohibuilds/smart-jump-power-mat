# Parts and wiring
Recovered from Smart_Jump_Mat_Final.ino and its original wiring guide.

| Part / signal | ESP32 DevKit V1 connection |
|---|---|
| HX711 VCC / GND | 3V3 / GND |
| HX711 DT / DOUT | GPIO4 |
| HX711 SCK / CLK | GPIO5 |
| ST7735 VCC / GND | 3V3 / GND, subject to the module's power specification |
| ST7735 SCK / MOSI | GPIO18 / GPIO23 |
| ST7735 CS / DC / RST | GPIO15 / GPIO2 / GPIO27 |
| TFT backlight | As specified by the display module; preserve its resistor/driver |
| USB power | Stable regulated supply through the board USB connector |

Parts: ESP32 DevKit V1, HX711, four matching three-wire load cells or an equivalent complete bridge, ST7735 TFT, a rigid platform/frame, a proper load-cell mounting arrangement, wiring and USB power.

Four three-wire half-bridge cells must form a Wheatstone bridge: bridge excitation connects to HX711 E+/E−, differential signal to A+/A−. Do not infer wire roles from color. Identify each cell's center tap by resistance measurements and follow its datasheet/combinator wiring. Mount each cell so the intended flexure is free; avoid direct overload or lateral load. Channel B is unused. GPIO2, GPIO5 and GPIO15 are boot-strapping pins on the original ESP32: a peripheral must not impose a conflicting level at reset.

The code reports ADC counts and game scores, not calibrated Newtons, medical strength or measured jump height. The platform and cells must be rated for dynamic landing loads, which exceed static body weight.

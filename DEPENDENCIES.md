# Build dependencies

- Espressif Arduino core: **3.3.0**
- Adafruit GFX Library@1.11.11
- Adafruit ST7735 and ST7789 Library@1.11.0
- HX711@0.7.5

The build workflow pins these versions. Do not install a separate I2S library for the AI Assistant; `ESP_I2S.h` comes from the ESP32 core. Hardware behavior still requires testing on the physical build.

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "HX711.h"
#include <WiFi.h>
#include <WebServer.h>

// ============================================================
// SMART JUMP MAT - FINAL INTEGRATED ESP32 CODE
// ESP32 DevKit V1 + HX711 + ST7735 + Wi-Fi Dashboard
//
// Required libraries:
//   Adafruit GFX Library
//   Adafruit ST7735 and ST7789 Library
//   HX711 by Bogde
//
// IMPORTANT:
// Keep the mat completely EMPTY during startup calibration.
// ============================================================

// -------------------- Wi-Fi --------------------
#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WebServer server(80);

// -------------------- TFT Pins --------------------
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  27

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// -------------------- HX711 Pins --------------------
#define DT   4
#define SCK  5

HX711 scale;

// -------------------- Sensor / Game tuning --------------------
// These values are based on the readings you measured previously.
// If needed later, only these constants need tuning.
const long NOISE_FLOOR      = 80000;   // ignore no-load drift below this
const long JUMP_THRESHOLD   = 110000;  // jump begins above this
const long GREAT_THRESHOLD  = 180000;
const long SPACE_THRESHOLD  = 280000;
const long MAX_FORCE_MAP    = 320000;

const int ROCKET_BASE_Y = 116;
const int ROCKET_TOP_Y  = 68;

// -------------------- Sensor values --------------------
long baseValue = 0;
bool calibrated = false;
unsigned long lastSensorSample = 0;
unsigned long lastJumpEnd = 0;
long currentForce = 0;
long rawReading = 0;

long peakForce = 0;
long lastJumpPeak = 0;
long bestJumpPeak = 0;

// -------------------- Game values --------------------
int rocketY = ROCKET_BASE_Y;
int targetY = ROCKET_BASE_Y;

int jumpScore = 0;
int bestScore = 0;

bool inJump = false;

unsigned long jumpStartTime = 0;
unsigned long launchHoldUntil = 0;
unsigned long standingSince = 0;

// -------------------- Dashboard values --------------------
String webStatus = "READY";
String jumpLevel = "READY";
String performance = "Waiting";
int legStrength = 0;
int stabilityScore = 100;

// -------------------- Display timing --------------------
unsigned long lastDisplayUpdate = 0;
unsigned long lastSerialPrint = 0;

// ============================================================
// HELPERS
// ============================================================

long readHX711Average(byte samples)
{
  if (!scale.is_ready())
  {
    return rawReading;
  }

  int64_t total = 0;
  byte valid = 0;

  for (byte i = 0; i < samples; i++)
  {
    if (scale.is_ready())
    {
      total += scale.read();
      lastSensorSample = millis();
      valid++;
    }
  }

  if (valid == 0)
    return rawReading;

  return total / valid;
}

int scoreFromForce(long f)
{
  if (f <= JUMP_THRESHOLD)
    return 0;

  long s = map(
    constrain(f, JUMP_THRESHOLD, MAX_FORCE_MAP),
    JUMP_THRESHOLD,
    MAX_FORCE_MAP,
    10,
    100
  );

  return constrain((int)s, 0, 100);
}

void updateAnalysis()
{
  legStrength = jumpScore;

  // This is a prototype "stability" estimate based on signal steadiness,
  // not a medical left/right balance measurement.
  long variation = abs(currentForce - lastJumpPeak);

  int stability = 100 - (int)(variation / 5000);
  stabilityScore = constrain(stability, 0, 100);

  if (jumpScore >= 85)
  {
    jumpLevel = "ADVANCED";
    performance = "EXCELLENT";
  }
  else if (jumpScore >= 60)
  {
    jumpLevel = "INTERMEDIATE";
    performance = "VERY GOOD";
  }
  else if (jumpScore >= 30)
  {
    jumpLevel = "BEGINNER";
    performance = "GOOD";
  }
  else if (jumpScore > 0)
  {
    jumpLevel = "LOW";
    performance = "KEEP PRACTICING";
  }
  else
  {
    jumpLevel = "READY";
    performance = "WAITING";
  }
}

void setStatusFromPeak()
{
  long value = max(currentForce, lastJumpPeak);

  if (value >= SPACE_THRESHOLD)
    webStatus = "SPACE!";
  else if (value >= GREAT_THRESHOLD)
    webStatus = "GREAT!";
  else if (value >= JUMP_THRESHOLD)
    webStatus = "GOOD!";
  else
    webStatus = "READY";
}

// ============================================================
// TFT GRAPHICS
// ============================================================

void drawRocket(int x, int y)
{
  // Body
  tft.fillRoundRect(x, y, 14, 28, 4, ST77XX_WHITE);

  // Nose
  tft.fillTriangle(
    x, y,
    x + 14, y,
    x + 7, y - 12,
    ST77XX_RED
  );

  // Fins
  tft.fillTriangle(
    x, y + 20,
    x - 6, y + 28,
    x, y + 28,
    ST77XX_BLUE
  );

  tft.fillTriangle(
    x + 14, y + 20,
    x + 20, y + 28,
    x + 14, y + 28,
    ST77XX_BLUE
  );

  // Window
  tft.fillCircle(x + 7, y + 8, 3, ST77XX_CYAN);

  // Flame while flying
  if (y < ROCKET_BASE_Y - 2)
  {
    tft.fillTriangle(
      x + 4, y + 28,
      x + 10, y + 28,
      x + 7, y + 39,
      ST77XX_YELLOW
    );

    tft.fillTriangle(
      x + 5, y + 28,
      x + 9, y + 28,
      x + 7, y + 35,
      ST77XX_RED
    );
  }
}

void drawStaticScreen()
{
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(1);

  tft.setCursor(4, 4);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("SMART JUMP MAT");

  // Fixed stars
  tft.drawPixel(8, 74, ST77XX_WHITE);
  tft.drawPixel(22, 92, ST77XX_WHITE);
  tft.drawPixel(105, 82, ST77XX_WHITE);
  tft.drawPixel(119, 108, ST77XX_WHITE);
  tft.drawPixel(16, 132, ST77XX_WHITE);
  tft.drawPixel(100, 143, ST77XX_WHITE);
}

void updateTFT()
{
  // Header region only
  tft.fillRect(0, 15, 128, 49, ST77XX_BLACK);

  tft.setTextSize(1);

  tft.setCursor(4, 18);
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("SCORE:");
  tft.print(jumpScore);

  tft.setCursor(70, 18);
  tft.setTextColor(ST77XX_CYAN);
  tft.print("F:");
  tft.print(currentForce / 1000);
  tft.print("k");

  tft.setCursor(4, 34);

  if (webStatus == "SPACE!")
    tft.setTextColor(ST77XX_CYAN);
  else if (webStatus == "GREAT!")
    tft.setTextColor(ST77XX_GREEN);
  else if (webStatus == "GOOD!")
    tft.setTextColor(ST77XX_YELLOW);
  else
    tft.setTextColor(ST77XX_WHITE);

  tft.print(webStatus);

  tft.setCursor(4, 50);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("BEST:");
  tft.print(bestScore);

  // Clear only the rocket animation area.
  // This prevents the ghost/trail glitch without repeatedly clearing
  // the entire screen.
  tft.fillRect(34, 64, 62, 96, ST77XX_BLACK);

  // Redraw a few stars inside animation region
  tft.drawPixel(44, 76, ST77XX_WHITE);
  tft.drawPixel(85, 88, ST77XX_WHITE);
  tft.drawPixel(48, 138, ST77XX_WHITE);
  tft.drawPixel(90, 150, ST77XX_WHITE);

  // Launch pad
  tft.drawFastHLine(43, 154, 42, ST77XX_BLUE);
  tft.drawFastHLine(47, 157, 34, ST77XX_WHITE);

  // Smoke near launch pad
  if (rocketY < ROCKET_BASE_Y - 4)
  {
    tft.fillCircle(57, 149, 3, ST77XX_WHITE);
    tft.fillCircle(65, 152, 3, ST77XX_WHITE);
    tft.fillCircle(72, 149, 2, ST77XX_WHITE);
  }

  drawRocket(57, rocketY);
}

// ============================================================
// WEB DASHBOARD
// ============================================================

const char MAIN_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>Smart Jump Mat Dashboard</title>
<style>
*{box-sizing:border-box}
body{
  margin:0;
  background:#07101f;
  color:white;
  font-family:Arial,Helvetica,sans-serif;
}
.container{
  max-width:780px;
  margin:auto;
  padding:20px;
}
.header{
  padding:18px 5px 24px;
}
.title{
  font-size:32px;
  font-weight:800;
  margin:0 0 6px;
}
.subtitle{
  color:#9fb0ca;
  margin:0;
}
.online{
  color:#58dd8b;
  font-weight:700;
  margin-top:12px;
}
.grid{
  display:grid;
  grid-template-columns:repeat(2,1fr);
  gap:14px;
}
.card{
  background:#101b2f;
  border:1px solid #192944;
  border-radius:22px;
  padding:20px;
  min-height:145px;
  box-shadow:0 10px 30px rgba(0,0,0,.20);
}
.wide{
  grid-column:1/-1;
}
.label{
  color:#96b9ff;
  font-weight:700;
  font-size:16px;
  margin-bottom:16px;
}
.value{
  font-size:38px;
  font-weight:800;
  color:#59dd8c;
  word-break:break-word;
}
.small{
  font-size:27px;
}
.note{
  color:#8092ad;
  font-size:13px;
  line-height:1.5;
  margin-top:20px;
}
.bar{
  height:12px;
  border-radius:20px;
  background:#1b2a44;
  margin-top:14px;
  overflow:hidden;
}
.fill{
  height:100%;
  width:0%;
  background:#59dd8c;
  transition:width .2s;
}
@media(max-width:600px){
  .container{padding:14px}
  .title{font-size:29px}
  .grid{grid-template-columns:1fr}
  .wide{grid-column:auto}
  .card{min-height:128px}
  .value{font-size:36px}
}
</style>
</head>

<body>
<div class="container">

  <div class="header">
    <div class="title">SMART JUMP MAT</div>
    <p class="subtitle">RA TECH jump-game prototype</p>
    <div class="online" id="connection">CONNECTING</div>
    <button onclick="tare()">Tare empty mat</button>
  </div>

  <div class="grid">

    <div class="card wide">
      <div class="label">Status</div>
      <div class="value" id="status">READY</div>
    </div>

    <div class="card">
      <div class="label">Load Signal (ADC counts)</div>
      <div class="value" id="force">0</div>
    </div>

    <div class="card">
      <div class="label">Jump Score</div>
      <div class="value" id="score">0</div>
      <div class="bar"><div class="fill" id="scoreBar"></div></div>
    </div>

    <div class="card">
      <div class="label">Game Strength Score</div>
      <div class="value" id="strength">0%</div>
      <div class="bar"><div class="fill" id="strengthBar"></div></div>
    </div>

    <div class="card">
      <div class="label">Stability Score (Estimated)</div>
      <div class="value" id="stability">100%</div>
      <div class="bar"><div class="fill" id="stabilityBar"></div></div>
    </div>

    <div class="card">
      <div class="label">Jump Level</div>
      <div class="value small" id="level">READY</div>
    </div>

    <div class="card">
      <div class="label">Performance</div>
      <div class="value small" id="performance">WAITING</div>
    </div>

    <div class="card">
      <div class="label">Best Jump Score</div>
      <div class="value" id="bestScore">0</div>
    </div>

    <div class="card">
      <div class="label">Best Peak (ADC counts)</div>
      <div class="value small" id="bestForce">0</div>
    </div>

  </div>

  <p class="note">
    Prototype measurements for demonstration only. "Leg strength" and
    "stability" are estimated from the jump-mat signal and are not clinical
    diagnostic measurements.
  </p>
</div>

<script>
async function refreshData(){
  try{
    const r = await fetch('/data?x=' + Date.now(), {cache:'no-store'});
    if (!r.ok) throw new Error('HTTP '+r.status);
    const d = await r.json();
    document.getElementById('connection').textContent = 'LIVE SESSION';

    document.getElementById('status').textContent = d.status;
    document.getElementById('force').textContent = d.force;
    document.getElementById('score').textContent = d.score;
    document.getElementById('strength').textContent = d.strength + '%';
    document.getElementById('stability').textContent = d.stability + '%';
    document.getElementById('level').textContent = d.level;
    document.getElementById('performance').textContent = d.performance;
    document.getElementById('bestScore').textContent = d.bestScore;
    document.getElementById('bestForce').textContent = d.bestForce;

    document.getElementById('scoreBar').style.width = d.score + '%';
    document.getElementById('strengthBar').style.width = d.strength + '%';
    document.getElementById('stabilityBar').style.width = d.stability + '%';
  }catch(e){document.getElementById("connection").textContent="DISCONNECTED - last values shown";}
  finally{setTimeout(refreshData,250);}
}

async function tare(){
  if(!confirm('Remove all weight from the mat before calibration. Continue?')) return;
  try{const r=await fetch('/tare',{method:'POST'});alert(await r.text());}
  catch(e){alert('Calibration request failed. Check connection.');}
}
refreshData();
</script>

</body>
</html>
)rawliteral";

void handleRoot()
{
  server.send_P(200, "text/html", MAIN_PAGE);
}

void handleData()
{
  String json = "{";

  json += "\"sensor_ok\":" + String(calibrated && millis()-lastSensorSample<1000 ? "true" : "false") + ",";
  json += "\"force\":" + String(currentForce) + ",";
  json += "\"score\":" + String(jumpScore) + ",";
  json += "\"status\":\"" + webStatus + "\",";
  json += "\"strength\":" + String(legStrength) + ",";
  json += "\"stability\":" + String(stabilityScore) + ",";
  json += "\"level\":\"" + jumpLevel + "\",";
  json += "\"performance\":\"" + performance + "\",";
  json += "\"bestScore\":" + String(bestScore) + ",";
  json += "\"bestForce\":" + String(bestJumpPeak);

  json += "}";

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

// ============================================================
// CALIBRATION
// ============================================================

void calibrateMat()
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);

  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(15, 44);
  tft.print("KEEP MAT EMPTY");

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(16, 62);
  tft.print("CALIBRATING...");

  Serial.println();
  Serial.println("================================");
  Serial.println("KEEP MAT EMPTY - CALIBRATING");
  Serial.println("================================");

  // Allow the hardware to settle.
  delay(3000);

  int64_t calibrationTotal = 0;
  const int calibrationSamples = 300;
  int validSamples = 0;

  for (int i = 0; i < calibrationSamples; i++)
  {
    if (scale.is_ready())
    {
      calibrationTotal += scale.read();
      lastSensorSample = millis();
      validSamples++;
    }

    delay(5);
  }

  calibrated = validSamples >= 8;
  if (!calibrated) {
    webStatus = "SENSOR ERROR";
    Serial.println("HX711 calibration failed: check power, DOUT=4 and SCK=5.");
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(2,60); tft.print("HX711 NOT READY");
    return;
  }
  baseValue = (long)(calibrationTotal / validSamples);

  Serial.print("Base Value = ");
  Serial.println(baseValue);

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(37, 60);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("READY");

  delay(700);
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  // Start HX711
  scale.begin(DT, SCK);

  // Start TFT
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);

  calibrateMat();
  drawStaticScreen();

  // Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  unsigned long wifiStart = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 20000)
  {
    delay(250);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi Connected!");
    Serial.print("Dashboard IP: http://");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("WiFi connection failed.");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("RA-TECH-Jump-Mat", AP_PASSWORD);
    Serial.print("Connect to RA-TECH-Jump-Mat, then http://");
    Serial.println(WiFi.softAPIP());
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/tare", HTTP_POST, []() {
    inJump = false; jumpScore = 0; lastJumpPeak = 0; peakForce = 0;
    standingSince = 0; launchHoldUntil = 0;
    calibrateMat();
    drawStaticScreen();
    server.send(calibrated ? 200 : 503,"text/plain",calibrated ? "Tare complete" : "HX711 not ready");
  });
  server.begin();

  Serial.println("Web Server Started");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
  server.handleClient();

  // Fast enough to catch a jump, but with a little averaging for noise.
  rawReading = readHX711Average(5);

  long measured = abs(rawReading - baseValue);

  // No-load drift / noise suppression.
  if (measured < NOISE_FLOOR)
    measured = 0;

  currentForce = measured;
  if (!calibrated || millis() - lastSensorSample > 1000) {
    inJump = false;
    webStatus = "SENSOR ERROR";
    if (millis()-lastDisplayUpdate >= 200) { lastDisplayUpdate=millis(); updateTFT(); }
    delay(2);
    return;
  }

  unsigned long now = millis();

  // ----------------------------------------------------------
  // Standing detection:
  // Your earlier readings showed standing close to the lower jump range.
  // Holding a moderate force for a short time resets the previous score.
  // ----------------------------------------------------------
  if (!inJump &&
      currentForce >= NOISE_FLOOR &&
      currentForce < JUMP_THRESHOLD)
  {
    if (standingSince == 0)
      standingSince = now;

    if (now - standingSince > 400)
    {
      jumpScore = 0;
      lastJumpPeak = 0;
      webStatus = "READY";
      jumpLevel = "READY";
      performance = "WAITING";
    }
  }
  else
  {
    standingSince = 0;
  }

  // ----------------------------------------------------------
  // Jump start
  // ----------------------------------------------------------
  if (!inJump && currentForce >= JUMP_THRESHOLD && now - lastJumpEnd >= 350)
  {
    inJump = true;
    jumpStartTime = now;

    peakForce = currentForce;
    lastJumpPeak = currentForce;

    jumpScore = scoreFromForce(currentForce);
    launchHoldUntil = now + 350;
  }

  // ----------------------------------------------------------
  // During jump: capture peak force immediately
  // ----------------------------------------------------------
  if (inJump)
  {
    if (currentForce > peakForce)
    {
      peakForce = currentForce;
      lastJumpPeak = peakForce;

      jumpScore = scoreFromForce(peakForce);

      // Keep rocket launched briefly after every new peak.
      launchHoldUntil = now + 350;
    }

    // End the sensing phase when load drops after the jump.
    if ((currentForce < JUMP_THRESHOLD / 2 && now - jumpStartTime > 120) ||
        now - jumpStartTime >= 1500)
    {
      inJump = false;
      lastJumpEnd = now;

      lastJumpPeak = peakForce;
      jumpScore = scoreFromForce(lastJumpPeak);

      if (jumpScore > bestScore)
        bestScore = jumpScore;

      if (lastJumpPeak > bestJumpPeak)
        bestJumpPeak = lastJumpPeak;

      launchHoldUntil = now + 350;
      peakForce = 0;
    }
  }

  // ----------------------------------------------------------
  // Rocket target based on the most recent jump peak
  // ----------------------------------------------------------
  if ((int32_t)(launchHoldUntil - now) > 0 && lastJumpPeak >= JUMP_THRESHOLD)
  {
    int jumpHeight = map(
      constrain(lastJumpPeak, JUMP_THRESHOLD, MAX_FORCE_MAP),
      JUMP_THRESHOLD,
      MAX_FORCE_MAP,
      12,
      ROCKET_BASE_Y - ROCKET_TOP_Y
    );

    targetY = ROCKET_BASE_Y - jumpHeight;

    if (targetY < ROCKET_TOP_Y)
      targetY = ROCKET_TOP_Y;
  }
  else
  {
    targetY = ROCKET_BASE_Y;
  }

  setStatusFromPeak();
  updateAnalysis();

  // ----------------------------------------------------------
  // TFT update
  // ----------------------------------------------------------
  if (now - lastDisplayUpdate >= 40)
  {
    lastDisplayUpdate = now;
  // Fast launch
  if (rocketY > targetY)
  {
    rocketY -= 10;

    if (rocketY < targetY)
      rocketY = targetY;
  }

  // Faster return to launch pad
  if (rocketY < targetY)
  {
    rocketY += 6;

    if (rocketY > targetY)
      rocketY = targetY;
  }

    updateTFT();
  }

  // ----------------------------------------------------------
  // Serial debug
  // ----------------------------------------------------------
  if (now - lastSerialPrint >= 150)
  {
    lastSerialPrint = now;

    Serial.print("Raw=");
    Serial.print(rawReading);

    Serial.print(" Base=");
    Serial.print(baseValue);

    Serial.print(" Force=");
    Serial.print(currentForce);

    Serial.print(" Score=");
    Serial.print(jumpScore);

    Serial.print(" Status=");
    Serial.println(webStatus);
  }

  // No large delay here: this keeps jump detection responsive.
  delay(2);
}

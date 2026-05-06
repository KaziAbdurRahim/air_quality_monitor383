#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <math.h>

// ===== WIFI CONFIG =====
const char* ssid = "wifiNAme";
const char* password = "wifiPassword";

WebServer server(80);

// ===== PIN CONFIG =====
#define DHT_PIN 4
#define MQ135_PIN 34
#define RELAY_PIN 32
#define BUZZER_PIN 18
#define PMS_RX 16
#define PMS_TX 17

DHT dht(DHT_PIN, DHT11);

// ===== MQ135 =====
#define RL 10.0
float R0 = 10.0;   // calibrate later

// ===== PMS STRUCT =====
struct PMSData {
  uint16_t pm1_0;
  uint16_t pm2_5;
  uint16_t pm10;
};

// ===== GLOBAL VARIABLES =====
PMSData pms = {0, 0, 0};
bool pms_valid = false;
unsigned long lastPMS = 0;

bool manualOverride = false;   // AUTO by default
bool relayState = false;       // false = OFF

unsigned long lastPrint = 0;

// ===== PMS READER =====
bool readPMS(PMSData &data) {
  static uint8_t buffer[32];

  while (Serial2.available() >= 32) {

    if (Serial2.peek() != 0x42) {
      Serial2.read();
      continue;
    }

    Serial2.readBytes(buffer, 32);

    if (buffer[0] != 0x42 || buffer[1] != 0x4D) continue;

    uint16_t sum = 0;
    for (int i = 0; i < 30; i++) sum += buffer[i];

    uint16_t received = (buffer[30] << 8) | buffer[31];
    if (sum != received) continue;

    data.pm1_0 = (buffer[4] << 8) | buffer[5];
    data.pm2_5 = (buffer[6] << 8) | buffer[7];
    data.pm10  = (buffer[8] << 8) | buffer[9];

    return true;
  }
  return false;
}

// ===== AQI FUNCTION =====
int getAQI(float pm25) {
  if (pm25 <= 12.0)
    return map(pm25 * 10, 0, 120, 0, 50);
  else if (pm25 <= 35.4)
    return map(pm25 * 10, 121, 354, 51, 100);
  else if (pm25 <= 55.4)
    return map(pm25 * 10, 355, 554, 101, 150);
  else if (pm25 <= 150.4)
    return map(pm25 * 10, 555, 1504, 151, 200);
  else if (pm25 <= 250.4)
    return map(pm25 * 10, 1505, 2504, 201, 300);
  else
    return 301;
}

// ===== MQ135 =====
float getMQ135Resistance(int raw_adc) {
  float voltage = raw_adc * (3.3 / 4095.0);
  if (voltage <= 0.01) return 0;
  return (3.3 - voltage) / voltage * RL;
}

float getCO2ppm(float ratio) {
  return 116.6020682 * pow(ratio, -2.769034857);
}

// ===== WIFI SETUP =====
void setupWiFi() {

  // OPTIONAL STATIC IP (uncomment if needed)
  /*
  IPAddress local_IP(192,168,0,200);
  IPAddress gateway(192,168,0,1);
  IPAddress subnet(255,255,255,0);
  WiFi.config(local_IP, gateway, subnet);
  */

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n\n✅ WiFi Connected!");

  Serial.println("===== NETWORK INFO =====");

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());

  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());

  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());

  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  Serial.print("RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  Serial.println("\n===== DASHBOARD LINKS =====");

  Serial.print("Main: http://");
  Serial.println(WiFi.localIP());

  Serial.print("Toggle: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/toggle");

  Serial.print("Mode: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/mode");

  Serial.println("==========================\n");
}

// ===== DASHBOARD HTML =====
String getHTML(float temp, float hum, float co2, int aqi) {

  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<style>";
  html += "body{font-family:Arial;text-align:center;background:#111;color:#fff;}";
  html += "button{padding:15px 25px;font-size:18px;margin:10px;}";
  html += ".ok{color:lime;} .alert{color:red;}";
  html += "</style></head><body>";

  html += "<h1>Air Quality Dashboard</h1>";

  html += "<p>Temp: " + String(temp) + " C</p>";
  html += "<p>Humidity: " + String(hum) + " %</p>";
  html += "<p>CO2: " + String(co2) + " ppm</p>";

  if (pms_valid) {
    html += "<p>PM2.5: " + String(pms.pm2_5) + "</p>";
    html += "<p>AQI: " + String(aqi) + "</p>";
  } else {
    html += "<p>PMS: No Data</p>";
  }

  if (aqi > 50) html += "<h2 class='alert'>ALERT</h2>";
  else html += "<h2 class='ok'>HEALTHY</h2>";

  html += "<p>Fan: ";
  html += relayState ? "ON" : "OFF";
  html += "</p>";

  html += "<p>Mode: ";
  html += manualOverride ? "MANUAL" : "AUTO";
  html += "</p>";

  html += "<a href='/toggle'><button>Toggle Fan</button></a>";
  html += "<a href='/mode'><button>Switch Mode</button></a>";

  html += "</body></html>";

  return html;
}

// ===== ROUTES =====
void handleRoot() {

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  int mq_raw = analogRead(MQ135_PIN);
  float RS = getMQ135Resistance(mq_raw);
  float co2 = getCO2ppm(RS / R0);

  int aqi = pms_valid ? getAQI(pms.pm2_5) : 0;

  server.send(200, "text/html", getHTML(temp, hum, co2, aqi));
}

void handleToggle() {
  manualOverride = true;
  relayState = !relayState;

  digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);
  digitalWrite(BUZZER_PIN, relayState ? HIGH : LOW);

  server.sendHeader("Location", "/");
  server.send(303);
}

void handleMode() {
  manualOverride = !manualOverride;

  server.sendHeader("Location", "/");
  server.send(303);
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);

  dht.begin();

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // 🔴 Active LOW relay fix
  digitalWrite(RELAY_PIN, HIGH);  // OFF
  digitalWrite(BUZZER_PIN, LOW);  // OFF

  while (Serial2.available()) Serial2.read();

  setupWiFi();

  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/mode", handleMode);

  server.begin();

  Serial.println("🚀 System Ready");
}

// ===== LOOP =====
void loop() {

  server.handleClient();

  // ===== PMS =====
  if (readPMS(pms)) {
    pms_valid = true;
    lastPMS = millis();
  }

  if (millis() - lastPMS > 3000) {
    pms_valid = false;
  }

  int aqi = pms_valid ? getAQI(pms.pm2_5) : 0;

  // ===== AUTO CONTROL =====
  if (!manualOverride) {

    if (aqi > 50 && pms_valid) {
      relayState = true;
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      relayState = false;
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }

  // ===== PERIODIC LINK PRINT =====
  if (millis() - lastPrint > 30000) {
    Serial.print("Dashboard: http://");
    Serial.println(WiFi.localIP());
    lastPrint = millis();
  }

  delay(1000);
}
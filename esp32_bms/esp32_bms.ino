/*
  BMS - ESP32 Gateway with Real Sensors
  Kirim data sensor DHT11, BH1750 ke server via WebSocket + tampilkan di OLED SH1106
  
  Library yang dibutuhkan (install via Arduino Library Manager):
  - WebSockets by Markus Sattler  ← search "WebSockets"
  - ArduinoJson by Benoit Blanchon ← search "ArduinoJson"
  - DHT sensor library by Adafruit (dan dependensinya "Adafruit Unified Sensor")
  - BH1750 by Christopher Laws
  - Adafruit SH110X (dan dependensinya "Adafruit GFX Library")

  Wiring:
    ESP32 3V3    -> VCC semua sensor (DHT11, BH1750, OLED)
    ESP32 GND    -> GND semua sensor
    ESP32 GPIO4  -> DATA DHT11
    ESP32 GPIO5  -> Relay Kipas (aktif LOW)
    ESP32 GPIO21 (SDA) -> SDA BH1750 & SDA OLED
    ESP32 GPIO22 (SCL) -> SCL BH1750 & SCL OLED

  GANTI bagian WiFi dan SERVER sesuai jaringan kamu.
*/

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ── KONFIGURASI — GANTI SESUAI KONDISI KAMU ──────────────────
const char* WIFI_SSID     = "Sumatra Kuphi 1";
const char* WIFI_PASSWORD = "milosusu";
const char* SERVER_IP     = "192.168.100.134";  // IP laptop/PC kamu
const int   SERVER_PORT   = 5000;
const char* DEVICE_ID     = "ESP32-01";
// ─────────────────────────────────────────────────────────────

// ── KONFIGURASI SENSOR DHT11 ──────────────────────────────────
#define DHTPIN   4
#define DHTTYPE  DHT11
DHT dht(DHTPIN, DHTTYPE);

// ── KONFIGURASI RELAY KIPAS ───────────────────────────────────
#define FAN_RELAY_PIN  5
float TEMP_THRESHOLD_MAX  = 35.0;  // Default, akan diupdate dari server

// CATATAN: Relay module aktif LOW
// HIGH = Relay OFF (standby)
// LOW  = Relay ON (kipas jalan)

// ── KONFIGURASI LED DEBUG ─────────────────────────────────────
#define LED_PIN  2  // Built-in LED ESP32

// ── KONFIGURASI BH1750 ────────────────────────────────────────
BH1750 lightMeter;

// ── KONFIGURASI OLED SH1106 ───────────────────────────────────
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1     // pakai -1 kalau OLED tidak punya pin reset terpisah
#define OLED_ADDR     0x3C   // alamat I2C umum untuk OLED SH1106

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WebSocketsClient ws;

unsigned long lastSendTime      = 0;
unsigned long lastHeartbeatTime = 0;
unsigned long lastDisplayUpdate = 0;
const long SEND_INTERVAL        = 5000;   // kirim data tiap 5 detik
const long HEARTBEAT_INTERVAL   = 10000;  // heartbeat tiap 10 detik
const long DISPLAY_INTERVAL     = 2000;   // update OLED tiap 2 detik
unsigned long uptimeSeconds     = 0;

bool wsConnected = false;

// Variabel untuk menyimpan data sensor terakhir
float lastTemp  = 0.0;
float lastHumid = 0.0;
float lastLight = 0.0;
bool sensorError = false;
bool fanStatus = false;  // Status kipas (ON/OFF)


// ── FUNGSI BACA SENSOR ────────────────────────────────────────
// Membaca data ASLI dari sensor DHT11 dan BH1750

float readTemperature() {
  // Retry 3x untuk DHT11 (sensor kadang butuh beberapa kali baca)
  for (int i = 0; i < 3; i++) {
    float temp = dht.readTemperature();
    if (!isnan(temp) && temp > -50 && temp < 100) {
      // Nilai valid (dalam range normal)
      sensorError = false;
      lastTemp = temp;
      return temp;
    }
    delay(100);  // Tunggu sebentar sebelum retry
  }
  
  // Setelah 3x retry masih gagal
  Serial.println("[ERROR] Gagal membaca suhu dari DHT11 setelah 3x retry!");
  sensorError = true;
  return lastTemp; // kembalikan nilai terakhir
}

float readHumidity() {
  // Retry 3x untuk DHT11
  for (int i = 0; i < 3; i++) {
    float humid = dht.readHumidity();
    if (!isnan(humid) && humid >= 0 && humid <= 100) {
      // Nilai valid (0-100%)
      sensorError = false;
      lastHumid = humid;
      return humid;
    }
    delay(100);
  }
  
  // Setelah 3x retry masih gagal
  Serial.println("[ERROR] Gagal membaca kelembapan dari DHT11 setelah 3x retry!");
  sensorError = true;
  return lastHumid; // kembalikan nilai terakhir
}

float readLight() {
  float lux = lightMeter.readLightLevel();
  if (lux < 0) {
    Serial.println("[ERROR] Gagal membaca cahaya dari BH1750!");
    return lastLight; // kembalikan nilai terakhir jika error
  }
  lastLight = lux;
  return lux;
}


// ── KONTROL KIPAS OTOMATIS ────────────────────────────────────

void controlFan(float temperature) {
  if (temperature > TEMP_THRESHOLD_MAX) {
    // Suhu TINGGI → Kipas ON (LOW untuk active low relay)
    if (!fanStatus) {
      digitalWrite(FAN_RELAY_PIN, LOW);  // Relay ON = LOW
      fanStatus = true;
      Serial.printf("[FAN] ON - Suhu %.1f°C > %.1f°C (threshold)\n", temperature, TEMP_THRESHOLD_MAX);
    }
  } else {
    // Suhu NORMAL → Kipas OFF (HIGH untuk active low relay)
    if (fanStatus) {
      digitalWrite(FAN_RELAY_PIN, HIGH);  // Relay OFF = HIGH
      fanStatus = false;
      Serial.printf("[FAN] OFF - Suhu %.1f°C <= %.1f°C (threshold)\n", temperature, TEMP_THRESHOLD_MAX);
    }
  }
}


// ── KIRIM DATA SENSOR ─────────────────────────────────────────

void sendSensorData() {
  if (!wsConnected) return;

  float temp  = readTemperature();
  float humid = readHumidity();
  float light = readLight();

  // TIDAK lagi kontrol kipas lokal - tunggu perintah dari server
  // controlFan(temp);  // ← DINONAKTIFKAN

  // Buat JSON
  StaticJsonDocument<200> doc;
  doc["device_id"]   = DEVICE_ID;
  doc["temperature"] = temp;
  doc["humidity"]    = humid;
  doc["light"]       = light;

  String payload;
  serializeJson(doc, payload);

  // Kirim event "esp32_data" ke server
  String msg = "42[\"esp32_data\"," + payload + "]";
  ws.sendTXT(msg);

  Serial.printf("[KIRIM] Temp=%.1f°C  Humid=%.1f%%  Light=%.0f Lux  Fan=%s%s\n",
                temp, humid, light, fanStatus ? "ON" : "OFF",
                sensorError ? " [SENSOR ERROR]" : "");
}


// ── KIRIM HEARTBEAT ───────────────────────────────────────────

void sendHeartbeat() {
  if (!wsConnected) return;

  uptimeSeconds += HEARTBEAT_INTERVAL / 1000;

  StaticJsonDocument<100> doc;
  doc["device_id"] = DEVICE_ID;
  doc["uptime"]    = uptimeSeconds;

  String payload;
  serializeJson(doc, payload);

  String msg = "42[\"esp32_heartbeat\"," + payload + "]";
  ws.sendTXT(msg);

  Serial.printf("[HEARTBEAT] Uptime: %lu detik\n", uptimeSeconds);
}


// ── UPDATE DISPLAY OLED ───────────────────────────────────────

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  // Header
  display.println("BMS Monitoring");
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SH110X_WHITE);

  // Status koneksi
  display.setCursor(0, 14);
  display.print("WiFi: ");
  display.println(WiFi.status() == WL_CONNECTED ? "OK" : "X");
  
  display.print("Server: ");
  display.println(wsConnected ? "OK" : "X");

  // Data sensor
  display.setCursor(0, 32);
  if (sensorError) {
    display.println("DHT11: Error!");
  } else {
    display.print("Suhu: ");
    display.print(lastTemp, 1);
    display.println(" C");
    
    display.print("Humid: ");
    display.print(lastHumid, 1);
    display.println(" %");
  }
  
  display.print("Cahaya: ");
  display.print(lastLight, 0);
  display.println(" lux");

  // Status kipas
  display.setCursor(0, 56);
  display.print("Kipas: ");
  display.println(fanStatus ? "ON" : "OFF");

  display.display();
}


// ── HANDLER WEBSOCKET ─────────────────────────────────────────

void onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {

    case WStype_CONNECTED:
      wsConnected = true;
      Serial.println("[WS] Konek ke server!");
      
      // Wajib kirim "40" agar Flask-SocketIO mau menerima data dari ESP32
      ws.sendTXT("40");
      
      // Request threshold dari server
      ws.sendTXT("42[\"request_threshold\",{}]");
      Serial.println("[WS] Request threshold dari server");
      break;

    case WStype_DISCONNECTED:
      wsConnected = false;
      Serial.println("[WS] Putus dari server, mencoba reconnect...");
      break;

    case WStype_TEXT:
      {  // Tambahkan scope block untuk deklarasi variabel
        Serial.printf("[WS] Pesan dari server: %s\n", payload);
        
        // Handle ping-pong
        if (length > 0 && payload[0] == '2') {
          ws.sendTXT("3");
          Serial.println("[WS] Membalas Ping dari server (Pong dikirim)");
        }
        
        // Handle threshold update dari server
        String msg = String((char*)payload);
        if (msg.indexOf("\"threshold_update\"") > 0) {
          parseThresholdUpdate(msg);
        }
        
        // Handle command dari browser
        if (msg.indexOf("\"server_command\"") > 0) {
          handleCommand(msg);
        }
      }
      break;

    default:
      break;
  }
}


// ── PARSE THRESHOLD UPDATE ────────────────────────────────────

void parseThresholdUpdate(String message) {
  // Format: 42["threshold_update",{"temperature":{"min":18,"max":35},...}]
  int startIdx = message.indexOf("{\"temperature\"");
  if (startIdx < 0) return;
  
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, message.substring(startIdx));
  
  if (error) {
    Serial.println("[ERROR] Gagal parse threshold JSON");
    return;
  }
  
  if (doc.containsKey("temperature")) {
    float newMax = doc["temperature"]["max"];
    if (newMax > 0 && newMax != TEMP_THRESHOLD_MAX) {
      TEMP_THRESHOLD_MAX = newMax;
      Serial.printf("[THRESHOLD] Temperature MAX updated: %.1f°C\n", TEMP_THRESHOLD_MAX);
      
      // Re-check fan status dengan threshold baru
      controlFan(lastTemp);
    }
  }
}


// ── HANDLE COMMAND DARI BROWSER ───────────────────────────────

void handleCommand(String message) {
  // Format: 42["server_command",{"type":"command","device":"led","action":"on"}]
  int startIdx = message.indexOf("{\"type\"");
  if (startIdx < 0) return;
  
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message.substring(startIdx));
  
  if (error) {
    Serial.println("[ERROR] Gagal parse command JSON");
    return;
  }
  
  String device = doc["device"];
  String action = doc["action"];
  
  Serial.printf("[COMMAND] Received: device=%s, action=%s\n", device.c_str(), action.c_str());
  
  // Handle FAN command (dari server)
  if (device == "fan") {
    if (action == "on") {
      digitalWrite(FAN_RELAY_PIN, LOW);  // Active LOW: LOW = ON
      fanStatus = true;
      Serial.println("[FAN] ON (via server command)");
    } else if (action == "off") {
      digitalWrite(FAN_RELAY_PIN, HIGH);  // Active LOW: HIGH = OFF
      fanStatus = false;
      Serial.println("[FAN] OFF (via server command)");
    }
  }
  
  // Handle LED command
  else if (device == "led") {
    if (action == "on") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("[LED] ON");
    } else if (action == "off") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("[LED] OFF");
    }
  }
  
  // Handle ESP32 command
  else if (device == "esp32") {
    if (action == "ping") {
      Serial.println("[PING] Pong! ESP32 is alive!");
      // Kirim response ke server
      ws.sendTXT("42[\"esp32_response\",{\"status\":\"pong\",\"uptime\":" + String(uptimeSeconds) + "}]");
    } else if (action == "restart") {
      Serial.println("[RESTART] Restarting ESP32 in 2 seconds...");
      delay(2000);
      ESP.restart();
    }
  }
}


// ── SETUP ─────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  Serial.println("\n[BMS] ESP32 Starting...");

  // Inisialisasi GPIO untuk relay kipas
  pinMode(FAN_RELAY_PIN, OUTPUT);
  digitalWrite(FAN_RELAY_PIN, HIGH);  // Kipas OFF saat startup (HIGH = standby untuk active low)
  Serial.println("[RELAY] Fan relay initialized (GPIO5) - Active LOW mode");

  // Inisialisasi LED debug
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // LED OFF saat startup
  Serial.println("[LED] Debug LED initialized (GPIO2)");

  // Inisialisasi I2C (SDA=21, SCL=22)
  Wire.begin(21, 22);
  Serial.println("[I2C] Initialized");

  // Inisialisasi DHT11
  dht.begin();
  delay(2000);  // DHT11 butuh waktu 2 detik setelah power on
  Serial.println("[SENSOR] DHT11 initialized");
  
  // Test baca DHT11
  float testTemp = dht.readTemperature();
  float testHumid = dht.readHumidity();
  if (isnan(testTemp) || isnan(testHumid)) {
    Serial.println("[WARNING] DHT11 test read FAILED! Cek wiring GPIO4");
  } else {
    Serial.printf("[SENSOR] DHT11 test OK - Temp:%.1f°C Humid:%.1f%%\n", testTemp, testHumid);
  }

  // Inisialisasi BH1750
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("[ERROR] BH1750 tidak terdeteksi, cek wiring!");
  } else {
    Serial.println("[SENSOR] BH1750 initialized");
  }

  // Inisialisasi OLED
  if (!display.begin(OLED_ADDR, true)) {
    Serial.println("[ERROR] OLED tidak terdeteksi, cek wiring/alamat I2C!");
  } else {
    Serial.println("[DISPLAY] OLED initialized");
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Inisialisasi...");
    display.println("Menghubungkan WiFi");
    display.display();
  }

  // Konek WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[WiFi] Menghubungkan");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.printf("[WiFi] Terhubung! IP: %s\n", WiFi.localIP().toString().c_str());

  // Update OLED dengan status WiFi
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi: Connected");
  display.print("IP: ");
  display.println(WiFi.localIP().toString().c_str());
  display.println("\nMenghubungkan ke");
  display.println("server...");
  display.display();

  // Setup WebSocket — pakai path /socket.io/ karena server pakai Flask-SocketIO
  ws.begin(SERVER_IP, SERVER_PORT, "/socket.io/?EIO=4&transport=websocket");
  ws.onEvent(onWebSocketEvent);
  ws.setReconnectInterval(3000);  // reconnect tiap 3 detik kalau putus

  Serial.printf("[WS] Menghubungkan ke %s:%d\n", SERVER_IP, SERVER_PORT);
  Serial.printf("[CONFIG] Temperature threshold MAX: %.1f°C\n", TEMP_THRESHOLD_MAX);
}


// ── LOOP ──────────────────────────────────────────────────────

void loop() {
  ws.loop();  // wajib dipanggil terus

  unsigned long now = millis();

  // Kirim data sensor tiap 5 detik
  if (now - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = now;
    sendSensorData();
  }

  // Kirim heartbeat tiap 10 detik
  if (now - lastHeartbeatTime >= HEARTBEAT_INTERVAL) {
    lastHeartbeatTime = now;
    sendHeartbeat();
  }

  // Update OLED tiap 2 detik
  if (now - lastDisplayUpdate >= DISPLAY_INTERVAL) {
    lastDisplayUpdate = now;
    updateDisplay();
  }
}

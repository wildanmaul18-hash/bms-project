# 🏢 Building Management System (BMS)

> Real-time IoT monitoring system dengan ESP32, Flask, dan WebSocket

## 📌 Deskripsi Project

BMS (Building Management System) adalah sistem monitoring dan kontrol gedung berbasis IoT yang menggunakan:
- **ESP32** sebagai gateway sensor
- **Flask + Socket.IO** sebagai backend server
- **Web Dashboard** untuk monitoring real-time

### Fitur Utama

✅ **Real-time Monitoring**
- Temperature (DHT11)
- Humidity (DHT11)
- Light level (BH1750)
- Live chart updates

✅ **Smart Fan Control**
- Otomatis ON/OFF berdasarkan suhu
- Room occupancy detection (cahaya)
- Server-controlled GPIO

✅ **Alarm System**
- Threshold-based alerts
- Room-aware (no alarm jika ruangan tidak digunakan)
- History logging

✅ **User Management**
- Login/logout system
- Role-based access (Admin/User)
- Password management

✅ **Data Logging**
- Database SQLite
- Export to CSV
- Date range filter

✅ **Remote Control**
- LED ON/OFF
- Fan ON/OFF
- ESP32 ping & restart

## 🛠 Teknologi yang Digunakan

**Hardware**:
- ESP32 DevKit
- DHT11 (Temperature & Humidity sensor)
- BH1750 (Light sensor)
- SH1106 OLED Display (128x64)
- Relay Module (Active LOW)

**Backend**:
- Python 3.11
- Flask 3.0
- Flask-SocketIO 5.3
- SQLite database

**Frontend**:
- HTML5 + CSS3 + JavaScript
- Socket.IO client
- Chart.js
- Font Awesome icons

## 📋 Cara Install & Jalankan

### 1. Setup Python Environment

```bash
cd bms
python -m venv venv
venv\Scripts\activate  # Windows
pip install -r requirements.txt
```

### 2. Run Flask Server

```bash
python app.py
```

Server jalan di: `http://localhost:5000`

### 3. Upload ESP32 Code

1. Install Arduino IDE
2. Install libraries:
   - WebSockets by Markus Sattler
   - ArduinoJson by Benoit Blanchon
   - DHT sensor library by Adafruit
   - BH1750 by Christopher Laws
   - Adafruit SH110X
3. Edit `esp32_bms/esp32_bms.ino`:
   - WiFi SSID & password
   - Server IP & port
4. Upload ke ESP32

### 4. Login ke Dashboard

- URL: `http://localhost:5000`
- Username: `admin`
- Password: `admin123`

## 🚀 Deploy ke Cloud

Ikuti panduan lengkap di: **[DEPLOYMENT_CLOUD_GUIDE.md](DEPLOYMENT_CLOUD_GUIDE.md)**

Platform yang didukung:
- ✅ Render.com (Recommended - GRATIS)
- ✅ Railway.app
- ✅ Fly.io

## 📁 Struktur Project

```
bms/
├── app.py                  # Flask server
├── database.py             # Database handler
├── static/                 # CSS files
├── templates/              # HTML files
├── esp32_bms/              # Arduino code
├── database/               # SQLite database
└── docs/                   # Documentation
    ├── PROJECT_STRUCTURE.md
    ├── DEPLOYMENT_CLOUD_GUIDE.md
    ├── DHT11_TROUBLESHOOTING.md
    └── ROOM_OCCUPANCY_LOGIC.md
```

## 📊 Halaman Web

| Halaman | URL | Akses | Deskripsi |
|---------|-----|-------|-----------|
| Login | `/login` | Public | Authentication |
| Dashboard | `/dashboard` | All Users | Real-time monitoring |
| Logging | `/logging` | All Users | Data history |
| Alarm | `/alarm` | All Users | Alarm history |
| Settings | `/settings` | Admin Only | Threshold settings |
| Users | `/users` | Admin Only | User management |
| Debug | `/debug` | Admin Only | ESP32 control |

## 🔐 Default Users

**Admin**:
- Username: `admin`
- Password: `admin123`
- Access: Full (all pages)

**User**:
- Username: `user`
- Password: `user123`
- Access: Limited (dashboard, logging, alarm)

## 🔌 Wiring Diagram

```
ESP32 Pin    →    Component
─────────────────────────────
3V3          →    VCC (All sensors)
GND          →    GND (All sensors)
GPIO4        →    DHT11 DATA
GPIO5        →    Relay IN (Fan)
GPIO21 (SDA) →    BH1750 SDA, OLED SDA
GPIO22 (SCL) →    BH1750 SCL, OLED SCL
GPIO2        →    Built-in LED
```

## 📡 API Endpoints

### REST API

- `GET /api/latest` - Latest sensor data
- `GET /api/logs` - Sensor logs (with filters)
- `GET /api/alarms` - Alarm history
- `GET /api/threshold` - Get thresholds
- `POST /api/threshold` - Update thresholds
- `GET /api/stats` - Statistics (min/max/avg)

### Socket.IO Events

**From ESP32**:
- `esp32_data` - Sensor data
- `esp32_heartbeat` - Keep-alive ping
- `request_threshold` - Request threshold update

**To ESP32**:
- `threshold_update` - New threshold values
- `server_command` - Control commands (fan/led/ping/restart)

**To Browser**:
- `sensor_update` - Real-time sensor data
- `alarm` - Alarm notifications
- `heartbeat` - ESP32 uptime

## 🐛 Troubleshooting

### ESP32 tidak connect ke server

1. Cek WiFi credentials di `esp32_bms.ino`
2. Cek server IP dan port
3. Pastikan server Flask sudah jalan
4. Lihat Serial Monitor (115200 baud)

### Sensor DHT11 error

Lihat: [DHT11_TROUBLESHOOTING.md](DHT11_TROUBLESHOOTING.md)

### Dashboard tidak update

1. Refresh browser
2. Cek Console browser (F12)
3. Pastikan ESP32 status "Connected"
4. Cek logs di terminal Flask

## 📚 Dokumentasi Lengkap

- **[PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md)** - Struktur folder & file
- **[DEPLOYMENT_CLOUD_GUIDE.md](DEPLOYMENT_CLOUD_GUIDE.md)** - Deploy ke cloud
- **[DHT11_TROUBLESHOOTING.md](DHT11_TROUBLESHOOTING.md)** - Fix sensor error
- **[ROOM_OCCUPANCY_LOGIC.md](ROOM_OCCUPANCY_LOGIC.md)** - Logic kipas & occupancy
- **[SYSTEM_CHECK_REPORT.md](SYSTEM_CHECK_REPORT.md)** - Testing report

## 🤝 Contributing

Project ini adalah tugas kuliah (PBL Semester 4).

## 📄 License

MIT License - Free to use for educational purposes

## 👤 Developer

**BMS Team - PBL Semester 4**

---

⭐ **Jika project ini membantu, jangan lupa beri star!**

# BMS Project - Struktur Folder dan File

## 📁 Struktur Project Lengkap

```
bms/
│
├── 📄 app.py                          # Flask server utama (Backend)
├── 📄 database.py                     # Database handler (SQLite)
│
├── 📁 database/
│   └── 📄 bms.db                      # Database SQLite (auto-generated)
│
├── 📁 static/                         # CSS dan file statis
│   ├── 📄 style.css                   # CSS utama untuk tampilan web
│   └── 📄 style_1.css                 # CSS backup
│
├── 📁 templates/                      # HTML templates (Frontend)
│   ├── 📄 login.html                  # Halaman login
│   ├── 📄 dashboard.html              # Dashboard monitoring real-time
│   ├── 📄 logging.html                # Halaman log data sensor
│   ├── 📄 alarm.html                  # Halaman history alarm
│   ├── 📄 settings.html               # Halaman pengaturan threshold
│   ├── 📄 users.html                  # Halaman manajemen user (Admin)
│   ├── 📄 debug.html                  # Halaman debug & command ESP32
│   ├── 📄 sidebar.html                # Sidebar navigation (included)
│   ├── 📄 Notes Update.txt            # Catatan update
│   │
│   └── 📁 Cache/                      # Backup versi lama HTML
│       ├── 📄 alarm_V1.html
│       ├── 📄 dashboard_V1.html
│       ├── 📄 dashboard_V2.html
│       └── 📁 Ver1/, Ver2/            # Backup berdasarkan versi
│
├── 📁 esp32_bms/                      # Arduino code untuk ESP32
│   ├── 📄 esp32_bms.ino               # Program utama ESP32
│   │
│   └── 📁 Cache/                      # Backup versi lama
│       ├── 📄 esp32_bms_V1.ino
│       └── 📄 esp32_dht11_bh1750_oled_sh1106.ino
│
├── 📁 Cache/                          # Backup file Python lama
│   ├── 📄 app_Ver1.py
│   ├── 📄 app_Ver2.py
│   ├── 📄 app_Ver3.py
│   ├── 📄 app_Ver4.py
│   ├── 📄 app_Ver5.py
│   ├── 📄 database_Ver1.py
│   └── 📄 files.zip, files_1.zip ... files_5.zip
│
├── 📁 Templete_Backup/                # Backup lengkap folder templates
│   └── (mirror of templates/)
│
├── 📁 venv/                           # Python virtual environment
│   └── (Python packages)
│
├── 📁 __pycache__/                    # Python cache (auto-generated)
│
├── 📁 .vscode/                        # VS Code settings
│
└── 📄 DOCUMENTATION FILES:
    ├── 📄 DEPLOYMENT_GUIDE.md         # Panduan deploy ke cloud
    ├── 📄 DHT11_TROUBLESHOOTING.md    # Troubleshooting sensor DHT11
    ├── 📄 ROOM_OCCUPANCY_LOGIC.md     # Dokumentasi logika occupancy
    ├── 📄 SYSTEM_CHECK_REPORT.md      # Report pengecekan sistem
    ├── 📄 FINAL_CHECK_SUMMARY.md      # Summary pengecekan final
    └── 📄 GUI_CHANGES.md              # Catatan perubahan GUI
```

## 📝 Deskripsi File-File Penting

### 🔵 Backend (Python Flask)

#### `app.py` - Server Utama
**Fungsi**:
- Web server menggunakan Flask
- WebSocket server untuk komunikasi real-time dengan ESP32 dan browser
- Routing untuk semua halaman web
- API endpoints untuk data sensor, alarm, logs
- Authentication & authorization

**Key Features**:
- Socket.IO events: `esp32_data`, `esp32_heartbeat`, `sensor_update`, `alarm`
- Session management untuk login
- Role-based access (admin/user)
- Room occupancy logic (kontrol kipas berdasarkan cahaya)

#### `database.py` - Database Handler
**Fungsi**:
- Manajemen database SQLite
- CRUD operations untuk:
  - Users (login, password, role)
  - Sensor logs (temperature, humidity, light)
  - Alarms (history alarm)
  - Thresholds (min/max settings)

**Tables**:
1. `users` - Data user dan authentication
2. `sensor_logs` - History data sensor
3. `alarms` - History alarm yang terjadi
4. `thresholds` - Setting threshold untuk setiap parameter

### 🔵 Frontend (HTML/CSS/JavaScript)

#### `templates/login.html`
- Halaman login dengan username/password
- Default user: `admin` / `admin123`

#### `templates/dashboard.html`
- Dashboard monitoring real-time
- Metric cards: Temperature, Humidity, Light
- Fan animation (spinning jika ON)
- Room occupancy indicator
- Real-time chart (Chart.js)
- ESP32 connection status
- Alarm notifications

#### `templates/logging.html`
- Tabel log data sensor
- Filter berdasarkan tanggal
- Export data (CSV)
- Pagination

#### `templates/alarm.html`
- History alarm yang pernah terjadi
- Filter alarm berdasarkan parameter
- Tabel alarm dengan level (HIGH/LOW)

#### `templates/settings.html` (Admin only)
- Update threshold min/max untuk:
  - Temperature (°C)
  - Humidity (%)
  - Light (Lux)
- Broadcast threshold ke ESP32

#### `templates/users.html` (Admin only)
- Manajemen user (tambah, hapus, edit)
- Ganti password user
- Ubah role (admin/user)

#### `templates/debug.html` (Admin only)
- Monitor ESP32 connection real-time
- Kirim command ke ESP32:
  - LED ON/OFF (GPIO2)
  - Fan ON/OFF (GPIO5)
  - Ping ESP32
  - Restart ESP32
- View raw sensor data

#### `templates/sidebar.html`
- Navigation sidebar (included di semua halaman)
- Menu: Dashboard, Logging, Alarm, Settings, Users, Debug
- Logout button

#### `static/style.css`
- Styling untuk semua halaman
- Responsive design
- Animations:
  - `.fan-spinning` - Animasi kipas berputar
  - Status indicators (green/orange/red dots)
  - Card hover effects

### 🔵 ESP32 Firmware

#### `esp32_bms/esp32_bms.ino`
**Hardware**:
- ESP32 DevKit
- DHT11 (Temperature & Humidity) - GPIO4
- BH1750 (Light sensor) - I2C (GPIO21/22)
- SH1106 OLED Display - I2C (GPIO21/22)
- Relay module (Fan control) - GPIO5 (Active LOW)
- Built-in LED - GPIO2

**Fungsi**:
- Baca sensor DHT11, BH1750
- Tampilkan data di OLED SH1106
- Kirim data ke server via WebSocket
- Terima command dari server (fan on/off, led, ping, restart)
- Update threshold dari server
- Retry mechanism untuk DHT11 (3x retry)
- Error handling untuk sensor

**Communication**:
- WebSocket client ke Flask-SocketIO server
- Protocol: Socket.IO v4 (EIO=4)
- Events dikirim: `esp32_data`, `esp32_heartbeat`, `request_threshold`
- Events diterima: `threshold_update`, `server_command`

**Configuration** (Edit di bagian atas file):
```cpp
const char* WIFI_SSID     = "Sumatra Kuphi 1";
const char* WIFI_PASSWORD = "milosusu";
const char* SERVER_IP     = "192.168.100.134";  // Ganti dengan IP server
const int   SERVER_PORT   = 5000;
```

### 🔵 Database

#### `database/bms.db` (SQLite)
Auto-generated saat pertama kali run `app.py`.

**Schema**:
```sql
-- Users table
CREATE TABLE users (
    id INTEGER PRIMARY KEY,
    username TEXT UNIQUE,
    password TEXT,  -- Hashed dengan werkzeug
    role TEXT       -- 'admin' atau 'user'
);

-- Sensor logs table
CREATE TABLE sensor_logs (
    id INTEGER PRIMARY KEY,
    timestamp TEXT,
    temperature REAL,
    humidity REAL,
    light REAL
);

-- Alarms table
CREATE TABLE alarms (
    id INTEGER PRIMARY KEY,
    timestamp TEXT,
    parameter TEXT,  -- 'temperature', 'humidity', 'light'
    value REAL,
    threshold REAL,
    level TEXT       -- 'HIGH' atau 'LOW'
);

-- Thresholds table
CREATE TABLE thresholds (
    parameter TEXT PRIMARY KEY,
    min_value REAL,
    max_value REAL
);
```

## 🔄 Data Flow Architecture

```
┌─────────────────┐
│   ESP32 DEVICE  │
│  (Hardware)     │
├─────────────────┤
│ • DHT11         │
│ • BH1750        │
│ • OLED SH1106   │
│ • Relay (Fan)   │
│ • LED           │
└────────┬────────┘
         │ WebSocket (Socket.IO)
         │ Send: sensor data, heartbeat
         │ Receive: threshold, commands
         ▼
┌─────────────────────────────┐
│   FLASK SERVER (app.py)     │
│   Backend + WebSocket       │
├─────────────────────────────┤
│ • Receive sensor data       │
│ • Check room occupancy      │
│ • Decide fan on/off         │
│ • Send command to ESP32     │
│ • Save to database          │
│ • Broadcast to browsers     │
└────────┬────────────────────┘
         │ HTTP + WebSocket
         │ Emit: sensor_update, alarm
         │ Receive: user actions
         ▼
┌─────────────────────────────┐
│   WEB BROWSER (Frontend)    │
│   dashboard.html + CSS/JS   │
├─────────────────────────────┤
│ • Display real-time data    │
│ • Show fan animation        │
│ • Show room status          │
│ • Display alarms            │
│ • User interactions         │
└─────────────────────────────┘
         │
         │ Save to
         ▼
┌─────────────────────────────┐
│   SQLite Database (bms.db)  │
├─────────────────────────────┤
│ • sensor_logs               │
│ • alarms                    │
│ • users                     │
│ • thresholds                │
└─────────────────────────────┘
```

## 🔐 Default Credentials

**Admin Account**:
- Username: `admin`
- Password: `admin123`
- Role: `admin` (akses penuh)

**User Account**:
- Username: `user`
- Password: `user123`
- Role: `user` (akses terbatas: dashboard, logging, alarm)

## 📊 Port & Network

**Local Development**:
- Flask Server: `http://localhost:5000`
- WebSocket: `ws://localhost:5000/socket.io/`

**Production (Cloud)**:
- Flask Server: `https://your-app.onrender.com`
- WebSocket: `wss://your-app.onrender.com/socket.io/`

## 🛠 Dependencies

**Python (Backend)**:
```
Flask
Flask-SocketIO
python-socketio
eventlet
werkzeug
```

**Arduino (ESP32)**:
```
WiFi (ESP32 core)
WebSockets by Markus Sattler
ArduinoJson by Benoit Blanchon
DHT sensor library by Adafruit
Adafruit Unified Sensor
BH1750 by Christopher Laws
Adafruit SH110X
Adafruit GFX Library
```

**Frontend (JavaScript)**:
```
Socket.IO client (CDN)
Chart.js (CDN)
Font Awesome icons (CDN)
```

## 📈 Feature Summary

### ✅ Implemented Features

1. **Real-time Monitoring**
   - Temperature, Humidity, Light sensor
   - WebSocket communication
   - Live chart updates

2. **Smart Fan Control**
   - Server-controlled based on temperature + room occupancy
   - Active LOW relay support
   - Visual dashboard synchronization

3. **Room Occupancy Detection**
   - Light sensor threshold (100 lux)
   - Visual indicators (door icon)
   - Alarm suppression when room unused

4. **Alarm System**
   - Threshold-based alerts
   - History logging
   - Room-aware (no alarm if unused)

5. **User Management**
   - Login/logout
   - Role-based access (admin/user)
   - Password management

6. **Data Logging**
   - Database storage
   - Date range filtering
   - Export capability

7. **ESP32 Remote Control**
   - LED control
   - Fan control
   - Ping/restart commands

8. **Threshold Configuration**
   - Dynamic threshold update
   - Broadcast to ESP32
   - Per-parameter settings

## 📁 Files You Can Safely Delete

Jika ingin membersihkan project:

**Backup folders** (opsional, untuk safety):
```
Cache/
Templete_Backup/
templates/Cache/
esp32_bms/Cache/
```

**Generated files** (akan di-generate ulang):
```
__pycache__/
database/bms.db (akan dibuat ulang dengan init_db())
```

**VS Code settings** (opsional):
```
.vscode/
```

## 🚀 Cara Menjalankan (Local)

1. **Setup Python Environment**
   ```cmd
   cd i:\PBL_SEM4\bms
   python -m venv venv
   venv\Scripts\activate
   pip install Flask Flask-SocketIO python-socketio eventlet
   ```

2. **Run Flask Server**
   ```cmd
   python app.py
   ```
   Server jalan di: `http://localhost:5000`

3. **Upload ESP32 Code**
   - Buka `esp32_bms/esp32_bms.ino` di Arduino IDE
   - Edit WiFi credentials dan SERVER_IP
   - Upload ke ESP32

4. **Access Web Dashboard**
   - Browser: `http://localhost:5000`
   - Login dengan `admin` / `admin123`

## 📚 Dokumentasi Tambahan

Baca file-file berikut untuk info lebih detail:
- `DEPLOYMENT_GUIDE.md` - Panduan deploy ke cloud
- `DHT11_TROUBLESHOOTING.md` - Fix sensor DHT11 error
- `ROOM_OCCUPANCY_LOGIC.md` - Penjelasan logic occupancy & fan
- `SYSTEM_CHECK_REPORT.md` - Report testing semua fitur

---

**Project**: Building Management System (BMS)  
**Version**: 2.0 (Server-Controlled Fan)  
**Last Update**: 2026-07-14  
**Developer**: BMS Team

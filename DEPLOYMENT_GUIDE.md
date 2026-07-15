# 🚀 Panduan Deployment BMS ke Cloud (GRATIS)

## 📋 Daftar Isi
1. [Persiapan](#persiapan)
2. [Pilihan Platform Cloud Gratis](#pilihan-platform-cloud-gratis)
3. [Skema Deployment](#skema-deployment)
4. [Langkah-langkah Deployment](#langkah-langkah-deployment)
5. [Konfigurasi ESP32](#konfigurasi-esp32)
6. [Testing & Troubleshooting](#testing--troubleshooting)

---

## 🎯 Persiapan

### Yang Anda Butuhkan:
- ✅ Akun GitHub (untuk version control)
- ✅ Akun email (untuk registrasi cloud platform)
- ✅ Aplikasi sudah berjalan di localhost
- ✅ ESP32 yang sudah terhubung ke WiFi

---

## ☁️ Pilihan Platform Cloud Gratis

### **Rekomendasi #1: Render.com** ⭐ (PALING MUDAH)

**Kelebihan:**
- ✅ Free tier sangat generous
- ✅ Support WebSocket (penting untuk Socket.IO)
- ✅ Auto-deploy dari GitHub
- ✅ SSL/HTTPS otomatis
- ✅ Tidak perlu kartu kredit
- ✅ Database PostgreSQL gratis (jika butuh upgrade dari SQLite)

**Limitasi Free Tier:**
- 750 jam/bulan (cukup untuk 1 bulan 24/7)
- Service sleep setelah 15 menit tidak ada traffic (cold start ~1 menit)
- 512 MB RAM
- Shared CPU

**Website:** https://render.com

---

### **Rekomendasi #2: Railway.app** ⭐

**Kelebihan:**
- ✅ $5 credit gratis setiap bulan (cukup untuk aplikasi kecil)
- ✅ Support WebSocket
- ✅ Deploy dari GitHub
- ✅ SSL otomatis
- ✅ Tidak sleep (selalu aktif)

**Limitasi Free Tier:**
- $5 credit/bulan (~500 jam eksekusi)
- Butuh kartu kredit untuk verifikasi (tapi tidak akan dicharge)

**Website:** https://railway.app

---

### **Alternatif #3: Fly.io**

**Kelebihan:**
- ✅ Free tier decent
- ✅ Support WebSocket
- ✅ Global deployment

**Limitasi:**
- Butuh kartu kredit
- Lebih teknis (pakai CLI)

---

### **⚠️ TIDAK DIREKOMENDASIKAN untuk BMS:**

❌ **Heroku** - Sudah tidak ada free tier sejak 2022  
❌ **PythonAnywhere** - Tidak support WebSocket di free tier  
❌ **Vercel/Netlify** - Untuk static/serverless, tidak cocok untuk aplikasi real-time  
❌ **Replit** - Service sleep terlalu cepat, tidak stabil untuk IoT

---

## 🏗️ Skema Deployment

```
┌─────────────────────────────────────────────────────────────┐
│                    DEPLOYMENT ARCHITECTURE                   │
└─────────────────────────────────────────────────────────────┘

  [ESP32 + Sensors]
         │
         │ WebSocket over HTTPS
         │ (wss://your-app.onrender.com)
         ↓
  ┌──────────────────┐
  │   Render.com     │
  │  Cloud Server    │
  │                  │
  │  ┌────────────┐  │
  │  │  Flask App │  │
  │  │ (app.py)   │  │
  │  └────────────┘  │
  │        │         │
  │        ↓         │
  │  ┌────────────┐  │
  │  │ SQLite DB  │  │
  │  │  (bms.db)  │  │
  │  └────────────┘  │
  └──────────────────┘
         │
         │ HTTPS + WebSocket
         │
         ↓
  [Browser User]
  (Dashboard Web)

┌─────────────────────────────────────────────────────────────┐
│                     FILE STRUCTURE                           │
└─────────────────────────────────────────────────────────────┘

bms/
├── app.py                 # Flask application
├── database.py            # Database functions
├── requirements.txt       # Python dependencies (BARU)
├── render.yaml           # Render config (BARU)
├── static/
│   └── style.css
├── templates/
│   ├── dashboard.html
│   ├── login.html
│   └── ...
└── database/
    └── bms.db
```

---

## 📝 Langkah-langkah Deployment

### **FASE 1: Persiapan File Project**

#### 1. Buat file `requirements.txt`
File ini berisi daftar library Python yang dibutuhkan.

```txt
Flask==3.0.0
Flask-SocketIO==5.3.5
python-socketio==5.10.0
python-engineio==4.8.0
Werkzeug==3.0.1
bcrypt==4.1.2
gunicorn==21.2.0
eventlet==0.33.3
```

**Lokasi:** `i:\PBL_SEM4\bms\requirements.txt`

---

#### 2. Buat file `render.yaml`
File konfigurasi untuk Render.com (optional, tapi recommended).

```yaml
services:
  - type: web
    name: bms-monitoring
    env: python
    buildCommand: pip install -r requirements.txt
    startCommand: gunicorn --worker-class eventlet -w 1 --bind 0.0.0.0:$PORT app:app
    envVars:
      - key: PYTHON_VERSION
        value: 3.11.0
      - key: SECRET_KEY
        generateValue: true
```

**Lokasi:** `i:\PBL_SEM4\bms\render.yaml`

---

#### 3. Update `app.py` untuk Production

Tambahkan di bagian paling atas setelah import:

```python
import os

# Untuk production
app.secret_key = os.environ.get('SECRET_KEY', 'bms-secret-key-ganti-ini')

# ... kode lainnya tetap sama ...

# Di bagian paling bawah, ganti:
if __name__ == '__main__':
    init_db()
    port = int(os.environ.get('PORT', 5000))
    print(f'[BMS] Server jalan di port {port}')
    socketio.run(app, host='0.0.0.0', port=port, debug=False)
```

---

#### 4. Buat `.gitignore`

```gitignore
# Python
__pycache__/
*.pyc
*.pyo
*.pyd
venv/
env/

# Database (optional: bisa di-commit atau di-ignore)
# database/bms.db

# Cache
Cache/
Templete_Backup/

# IDE
.vscode/
.idea/

# OS
.DS_Store
Thumbs.db
```

**Lokasi:** `i:\PBL_SEM4\bms\.gitignore`

---

### **FASE 2: Upload ke GitHub**

#### 1. Inisialisasi Git (jika belum)

```bash
cd i:\PBL_SEM4\bms
git init
git add .
git commit -m "Initial commit: BMS project ready for deployment"
```

#### 2. Buat Repository di GitHub

1. Buka https://github.com
2. Klik tombol **"New"** (repository baru)
3. Nama: `bms-monitoring` (atau nama lain)
4. Visibility: **Public** atau **Private** (bebas)
5. **JANGAN** centang "Initialize with README"
6. Klik **"Create repository"**

#### 3. Push ke GitHub

```bash
git remote add origin https://github.com/USERNAME-KAMU/bms-monitoring.git
git branch -M main
git push -u origin main
```

Ganti `USERNAME-KAMU` dengan username GitHub Anda.

---

### **FASE 3: Deploy ke Render.com**

#### 1. Registrasi Render.com

1. Buka https://render.com
2. Klik **"Get Started for Free"**
3. Sign up dengan GitHub (recommended)
4. Authorize Render untuk akses repository

#### 2. Create New Web Service

1. Di dashboard Render, klik **"New +"** → **"Web Service"**
2. Connect repository GitHub Anda (`bms-monitoring`)
3. Klik **"Connect"**

#### 3. Konfigurasi Service

**Isi form:**

| Field | Value |
|-------|-------|
| **Name** | `bms-monitoring` (atau nama lain) |
| **Region** | Singapore (terdekat dengan Indonesia) |
| **Branch** | `main` |
| **Root Directory** | (kosongkan) |
| **Runtime** | `Python 3` |
| **Build Command** | `pip install -r requirements.txt` |
| **Start Command** | `gunicorn --worker-class eventlet -w 1 --bind 0.0.0.0:$PORT app:app` |

**Environment Variables:**
- Klik **"Add Environment Variable"**
- Key: `SECRET_KEY`
- Value: `bms-secret-key-change-this-in-production-12345`

**Instance Type:**
- Pilih **"Free"**

#### 4. Deploy!

1. Klik **"Create Web Service"**
2. Tunggu proses build (5-10 menit)
3. Setelah selesai, Anda akan dapat URL:
   ```
   https://bms-monitoring-xxxx.onrender.com
   ```

---

### **FASE 4: Konfigurasi ESP32**

Update file `esp32_bms.ino`:

```cpp
// ── KONFIGURASI CLOUD ─────────────────────────────────────
const char* WIFI_SSID     = "Test12";           // WiFi Anda
const char* WIFI_PASSWORD = "abc12345";          // Password WiFi
const char* SERVER_IP     = "bms-monitoring-xxxx.onrender.com";  // ⚠️ GANTI dengan URL Render
const int   SERVER_PORT   = 443;                 // ⚠️ Port HTTPS (bukan 5000)
const char* DEVICE_ID     = "ESP32-01";

// Untuk HTTPS, perlu tambahan library
#include <WiFiClientSecure.h>

// ... (kode lainnya sama)

// Di setup(), update WebSocket untuk HTTPS:
void setup() {
    // ... WiFi setup sama ...
    
    // ⚠️ Setup WebSocket dengan SSL
    ws.beginSSL(SERVER_IP, SERVER_PORT, "/socket.io/?EIO=4&transport=websocket");
    ws.onEvent(onWebSocketEvent);
    ws.setReconnectInterval(3000);
}
```

**PENTING:** 
- Ganti `bms-monitoring-xxxx.onrender.com` dengan URL Render Anda
- Port 443 untuk HTTPS
- Gunakan `beginSSL()` bukan `begin()`

---

### **FASE 5: Testing**

#### Test dari Browser:
1. Buka `https://bms-monitoring-xxxx.onrender.com`
2. Login dengan user default
3. Pastikan dashboard muncul

#### Test dari ESP32:
1. Upload kode ESP32 yang sudah diupdate
2. Buka Serial Monitor
3. Lihat log koneksi:
   ```
   [WiFi] Terhubung! IP: 192.168.x.x
   [WS] Menghubungkan ke bms-monitoring-xxxx.onrender.com:443
   [WS] Konek ke server!
   [KIRIM] Temp=25.3°C  Humid=67.2%  Light=450 Lux
   ```

#### Test Koneksi Real-time:
- ESP32 kirim data → Dashboard update otomatis
- Jika ada alarm → Notifikasi muncul

---

## 🔧 Troubleshooting

### Problem: Service sleep setelah 15 menit

**Solusi:**
1. Tambahkan cron job untuk ping service setiap 10 menit
2. Upgrade ke paid plan ($7/bulan untuk always-on)
3. Gunakan Railway.app (tidak sleep di free tier)

### Problem: ESP32 tidak bisa connect (SSL Error)

**Solusi 1 - Skip SSL Verification (NOT RECOMMENDED):**
```cpp
WiFiClientSecure client;
client.setInsecure();  // Skip certificate validation
```

**Solusi 2 - Use Railway instead (simpler URL):**
Railway memberikan domain yang lebih simple dan SSL lebih mudah.

### Problem: Database hilang setelah restart

**Penyebab:** Render free tier tidak persistent storage.

**Solusi:**
1. Upgrade ke PostgreSQL (gratis di Render)
2. Atau gunakan external database (ElephantSQL gratis)

---

## 💰 Estimasi Biaya

| Platform | Free Tier | Upgrade Cost |
|----------|-----------|--------------|
| **Render.com** | 750 jam/bulan | $7/bulan (always-on) |
| **Railway.app** | $5 credit/bulan | Pay as you go (~$5-10/bulan) |
| **Fly.io** | Limited free | ~$5/bulan |

**Rekomendasi untuk BMS:**
- **Development/Testing:** Render.com (free)
- **Production (selalu aktif):** Railway.app ($5/bulan)

---

## 📱 Akses dari Mana Saja

Setelah deploy, Anda bisa:
- ✅ Akses dashboard dari HP/laptop di mana saja
- ✅ ESP32 kirim data dari lokasi mana pun (asal ada WiFi)
- ✅ Share URL ke tim/klien
- ✅ Monitor real-time 24/7

**URL Format:**
```
https://bms-monitoring-xxxx.onrender.com
```

---

## 🎯 Next Steps (Optional)

Setelah deployment berhasil:

1. **Custom Domain** (opsional)
   - Beli domain di Namecheap/Cloudflare
   - Arahkan ke Render URL
   - Contoh: `bms.yourcompany.com`

2. **Database Upgrade**
   - Dari SQLite → PostgreSQL
   - Untuk persistent storage

3. **Monitoring**
   - Setup Uptime Robot (gratis)
   - Email notification jika server down

4. **Security**
   - Rate limiting
   - CORS configuration
   - Environment variables untuk credentials

---

## 📞 Butuh Bantuan?

Jika ada masalah:
1. Cek logs di Render dashboard
2. Cek Serial Monitor ESP32
3. Tanyakan ke saya dengan log error yang spesifik

---

**Dibuat oleh:** [Your Name]  
**Tanggal:** Juli 2026  
**Versi:** 1.0

---

## 🚦 Status Deployment

- [ ] File `requirements.txt` dibuat
- [ ] File `render.yaml` dibuat
- [ ] Update `app.py` untuk production
- [ ] Push ke GitHub
- [ ] Deploy ke Render
- [ ] Update ESP32 config
- [ ] Testing browser
- [ ] Testing ESP32 connection
- [ ] ✅ **DEPLOYMENT SELESAI**

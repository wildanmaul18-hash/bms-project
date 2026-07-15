# 🚀 Panduan Deploy BMS ke Cloud (Render.com) - GRATIS

## 📌 Mengapa Render.com?

✅ **GRATIS** untuk hobby projects  
✅ **HTTPS** otomatis (SSL certificate gratis)  
✅ **Support WebSocket** (Socket.IO)  
✅ **Easy deployment** dari GitHub  
✅ **Auto-deploy** setiap kali push ke GitHub  
✅ **750 jam gratis/bulan** (cukup untuk 1 app 24/7)

## 🎯 Yang Akan Kita Lakukan

1. Persiapan file-file untuk deployment
2. Upload project ke GitHub
3. Deploy ke Render.com
4. Update ESP32 code untuk connect ke cloud server
5. Testing

---

## 📋 STEP 1: Persiapan File Deployment

### 1.1 Buat File `requirements.txt`

File ini berisi daftar library Python yang dibutuhkan.

**Buat file baru**: `i:\PBL_SEM4\bms\requirements.txt`

```txt
Flask==3.0.0
Flask-SocketIO==5.3.5
python-socketio==5.10.0
eventlet==0.33.3
Werkzeug==3.0.1
gunicorn==21.2.0
```

### 1.2 Buat File `Procfile`

File ini memberitahu Render cara menjalankan aplikasi.

**Buat file baru**: `i:\PBL_SEM4\bms\Procfile` (tanpa ekstensi)

```
web: gunicorn --worker-class eventlet -w 1 app:app
```

### 1.3 Buat File `render.yaml`

File konfigurasi untuk Render (opsional tapi direkomendasikan).

**Buat file baru**: `i:\PBL_SEM4\bms\render.yaml`

```yaml
services:
  - type: web
    name: bms-server
    env: python
    plan: free
    buildCommand: pip install -r requirements.txt
    startCommand: gunicorn --worker-class eventlet -w 1 --bind 0.0.0.0:$PORT app:app
    envVars:
      - key: PYTHON_VERSION
        value: 3.11.0
```


### 1.4 Buat File `.gitignore`

Agar file-file yang tidak perlu tidak ikut terupload ke GitHub.

**Buat file baru**: `i:\PBL_SEM4\bms\.gitignore`

```
# Python
__pycache__/
*.py[cod]
*$py.class
*.so
venv/
env/
ENV/

# Database (akan dibuat otomatis di cloud)
database/bms.db

# VS Code
.vscode/

# Backup folders (tidak perlu di cloud)
Cache/
Templete_Backup/
templates/Cache/
esp32_bms/Cache/

# Logs
*.log

# OS
.DS_Store
Thumbs.db
```

### 1.5 Update `app.py` untuk Production

Tambahkan konfigurasi untuk support cloud deployment.


**Edit bagian bawah file `app.py`**, ubah dari:

```python
if __name__ == '__main__':
    init_db()
    print('[BMS] Server jalan di http://localhost:5000')
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
```

**Menjadi**:

```python
if __name__ == '__main__':
    init_db()
    import os
    port = int(os.environ.get('PORT', 5000))
    print(f'[BMS] Server starting on port {port}')
    socketio.run(app, host='0.0.0.0', port=port, debug=False)
```

**Penjelasan**:
- `os.environ.get('PORT', 5000)`: Ambil port dari environment variable (Render akan set ini)
- `debug=False`: Matikan debug mode untuk production

---

## 📋 STEP 2: Upload ke GitHub

### 2.1 Install Git (Jika Belum Ada)

Download dari: https://git-scm.com/download/win

Setelah install, restart Command Prompt.


### 2.2 Buat Akun GitHub (Jika Belum Ada)

1. Buka: https://github.com
2. Klik **Sign up**
3. Isi email, password, username
4. Verifikasi email

### 2.3 Buat Repository Baru di GitHub

1. Login ke GitHub
2. Klik tombol **+** (kanan atas) → **New repository**
3. Isi:
   - **Repository name**: `bms-project` (atau nama lain)
   - **Description**: Building Management System
   - **Public** atau **Private** (pilih Public agar gratis)
   - **JANGAN** centang "Add a README file"
4. Klik **Create repository**

### 2.4 Upload Project ke GitHub

Buka **Command Prompt** di folder project:

```cmd
cd i:\PBL_SEM4\bms
```

Jalankan command berikut **satu per satu**:

```cmd
git init
git add .
git commit -m "Initial commit: BMS project"
```


Lalu connect ke GitHub repository Anda (ganti `USERNAME` dan `REPO_NAME`):

```cmd
git remote add origin https://github.com/USERNAME/REPO_NAME.git
git branch -M main
git push -u origin main
```

**Contoh**:
```cmd
git remote add origin https://github.com/anishamaura/bms-project.git
git branch -M main
git push -u origin main
```

**Jika diminta login**:
- Username: username GitHub Anda
- Password: **BUKAN password GitHub**, tapi **Personal Access Token**

### 2.5 Cara Buat Personal Access Token (PAT)

1. GitHub → Klik foto profil → **Settings**
2. Scroll ke bawah → **Developer settings**
3. **Personal access tokens** → **Tokens (classic)**
4. **Generate new token** → **Generate new token (classic)**
5. Isi:
   - **Note**: `BMS Deploy`
   - **Expiration**: `90 days` (atau No expiration)
   - **Centang**: `repo` (semua sub-checkbox)
6. Klik **Generate token**
7. **COPY TOKEN** (hanya muncul 1x)


**Gunakan token ini sebagai password** saat git push.

✅ **Sekarang project Anda sudah di GitHub!**

---

## 📋 STEP 3: Deploy ke Render.com

### 3.1 Buat Akun Render

1. Buka: https://render.com
2. Klik **Get Started** atau **Sign Up**
3. **Sign up with GitHub** (recommended)
4. Authorize Render untuk akses GitHub

### 3.2 Deploy Web Service

1. Dashboard Render → Klik **New +** → **Web Service**
2. **Connect Repository**:
   - Pilih repository `bms-project` (atau nama repo Anda)
   - Klik **Connect**
3. **Konfigurasi**:
   - **Name**: `bms-server` (atau nama lain)
   - **Region**: Singapore (terdekat dengan Indonesia)
   - **Branch**: `main`
   - **Root Directory**: (kosongkan)
   - **Runtime**: `Python 3`
   - **Build Command**: `pip install -r requirements.txt`
   - **Start Command**: `gunicorn --worker-class eventlet -w 1 --bind 0.0.0.0:$PORT app:app`
   - **Plan**: **Free** (pilih ini)
4. Klik **Create Web Service**


### 3.3 Tunggu Deployment Selesai

Render akan:
1. Clone repository dari GitHub
2. Install dependencies (`pip install -r requirements.txt`)
3. Jalankan server dengan gunicorn
4. Deploy ke URL public

**Proses ini memakan waktu 3-5 menit.**

Status deployment:
- 🟡 **Building**: Sedang install dependencies
- 🟡 **Deploying**: Sedang deploy
- 🟢 **Live**: Berhasil deploy! ✅

### 3.4 Dapatkan URL Cloud Server

Setelah status **Live**, Anda akan mendapat URL seperti:

```
https://bms-server-xxxx.onrender.com
```

**Catat URL ini!** Kita akan pakai untuk ESP32.

### 3.5 Test Cloud Server

1. Buka URL di browser: `https://bms-server-xxxx.onrender.com`
2. Anda akan melihat halaman login BMS
3. Login dengan `admin` / `admin123`
4. Dashboard akan terbuka (tapi ESP32 belum connect)

✅ **Server berhasil di-deploy ke cloud!**


---

## 📋 STEP 4: Update ESP32 untuk Connect ke Cloud

### 4.1 Edit `esp32_bms.ino`

Buka file: `i:\PBL_SEM4\bms\esp32_bms\esp32_bms.ino`

**Ubah bagian konfigurasi** di baris 44-48:

**SEBELUM** (Local):
```cpp
const char* WIFI_SSID     = "Sumatra Kuphi 1";
const char* WIFI_PASSWORD = "milosusu";
const char* SERVER_IP     = "192.168.100.134";  // IP lokal
const int   SERVER_PORT   = 5000;
const char* DEVICE_ID     = "ESP32-01";
```

**SESUDAH** (Cloud):
```cpp
const char* WIFI_SSID     = "Sumatra Kuphi 1";  // Tetap sama
const char* WIFI_PASSWORD = "milosusu";          // Tetap sama
const char* SERVER_HOST   = "bms-server-xxxx.onrender.com";  // ← GANTI INI
const int   SERVER_PORT   = 443;                 // ← HTTPS port
const char* DEVICE_ID     = "ESP32-01";
```

**⚠️ PENTING**: Ganti `bms-server-xxxx.onrender.com` dengan URL Render Anda (tanpa `https://`).


### 4.2 Update Code untuk HTTPS/WSS

Render menggunakan **HTTPS** (bukan HTTP), jadi ESP32 perlu support **WebSocket Secure (WSS)**.

**Cari baris ini** (sekitar baris 70-71):
```cpp
#include <WebSocketsClient.h>
```

**Tambahkan di bawahnya**:
```cpp
#include <WiFiClientSecure.h>
```

**Cari fungsi `setup()`**, cari bagian WebSocket initialization (sekitar baris 470):

**SEBELUM**:
```cpp
ws.begin(SERVER_IP, SERVER_PORT, "/socket.io/?EIO=4&transport=websocket");
```

**SESUDAH** (untuk HTTPS/WSS):
```cpp
// Setup WebSocket Secure (WSS) untuk HTTPS
ws.beginSSL(SERVER_HOST, SERVER_PORT, "/socket.io/?EIO=4&transport=websocket");
ws.setReconnectInterval(5000);  // Reconnect tiap 5 detik kalau putus
```

**⚠️ CATATAN**: `beginSSL()` akan menggunakan WSS (secure WebSocket).


### 4.3 Update Variable Declaration

**Cari bagian deklarasi variabel** di atas (sekitar baris 44-48), ubah:

**SEBELUM**:
```cpp
const char* SERVER_IP     = "192.168.100.134";
const int   SERVER_PORT   = 5000;
```

**SESUDAH**:
```cpp
const char* SERVER_HOST   = "bms-server-xxxx.onrender.com";  // Cloud server
const int   SERVER_PORT   = 443;  // HTTPS port
```

### 4.4 Upload ke ESP32

1. Buka Arduino IDE
2. **Tools** → **Board** → **ESP32 Dev Module**
3. **Tools** → **Port** → Pilih COM port ESP32
4. Klik **Upload** (ikon panah kanan)
5. Tunggu sampai "Done uploading"

### 4.5 Monitor Serial

1. **Tools** → **Serial Monitor**
2. Set baud rate: **115200**
3. Perhatikan output:

```
[BMS] ESP32 Starting...
[WiFi] Menghubungkan.....
[WiFi] Terhubung! IP: 192.168.100.25
[WS] Menghubungkan ke bms-server-xxxx.onrender.com:443
[WS] Konek ke server!
[KIRIM] Temp=28.5°C  Humid=65.0%  Light=120 Lux  Fan=OFF
```


✅ **ESP32 berhasil connect ke cloud server!**

---

## 📋 STEP 5: Testing Dashboard Cloud

### 5.1 Akses Dashboard dari Browser

1. Buka browser (Chrome/Firefox/Edge)
2. Buka URL: `https://bms-server-xxxx.onrender.com`
3. Login dengan `admin` / `admin123`

### 5.2 Cek Status ESP32

Di dashboard:
- **ESP32 STATUS** harus: **Connected** (hijau)
- **DATA DITERIMA**: Harus bertambah (1 paket, 2 paket, ...)
- **Sensor values**: Harus update setiap 5 detik

### 5.3 Test Fitur-Fitur

✅ **Dashboard**: Real-time data, chart, fan animation  
✅ **Logging**: Tabel data sensor  
✅ **Alarm**: History alarm  
✅ **Settings**: Update threshold  
✅ **Debug**: Kirim command (LED ON/OFF, Fan ON/OFF, Ping ESP32)

---

## 🌍 Akses dari Mana Saja

**Keuntungan Cloud Deployment**:

1. **Akses dari HP**: Buka URL di browser HP
2. **Akses dari laptop lain**: Tidak perlu satu WiFi yang sama
3. **Akses dari mana saja**: Selama ada internet
4. **HTTPS**: Koneksi aman (terenkripsi)
5. **Always Online**: Server jalan 24/7


---

## ⚠️ TROUBLESHOOTING

### Problem 1: ESP32 Tidak Connect ke Server

**Cek Serial Monitor**:
```
[WS] Putus dari server, mencoba reconnect...
```

**Solusi**:
1. Pastikan `SERVER_HOST` benar (tanpa `https://`)
2. Pastikan `SERVER_PORT = 443` (bukan 5000)
3. Pastikan WiFi ESP32 stabil
4. Pastikan server Render status **Live** (buka dashboard Render)

### Problem 2: Server Render Status "Failed"

**Cek Logs di Render**:
1. Dashboard Render → Klik service name
2. Tab **Logs**
3. Lihat error message

**Solusi Umum**:
- Error `ModuleNotFoundError`: Library belum ada di `requirements.txt`
- Error `Port already in use`: Restart service
- Error `Application failed to start`: Cek `Procfile` dan `render.yaml`

### Problem 3: Database Kosong Setelah Deploy

**Penyebab**: Database SQLite di cloud bersifat **ephemeral** (reset tiap deploy).

**Solusi untuk Production**:
Gunakan database persistent seperti **PostgreSQL** (gratis di Render):

1. Render Dashboard → **New +** → **PostgreSQL**
2. Free tier (cukup untuk BMS)
3. Connect database ke app (edit `database.py`)


### Problem 4: Render "Service Sleeps" Setelah 15 Menit

**Render Free Plan**: Service akan "sleep" (mati) jika tidak ada traffic selama 15 menit.

**Dampak**:
- First request setelah sleep butuh 30-50 detik untuk "wake up"
- ESP32 akan reconnect otomatis

**Solusi**:
1. **Keep-alive ping**: Buat cron job yang ping server tiap 10 menit
2. **Upgrade ke Paid Plan**: $7/bulan (tidak sleep)

**Alternatif Gratis (Render Keep-Alive)**:
Gunakan service gratis seperti **UptimeRobot**:
- Daftar di: https://uptimerobot.com
- Add monitor: HTTP(s) → URL: `https://bms-server-xxxx.onrender.com`
- Interval: 5 minutes
- UptimeRobot akan ping server tiap 5 menit (keep awake)

---

## 🔄 Update Code Setelah Deploy

Jika Anda ubah code dan ingin update di cloud:

### Update Flow:

```cmd
cd i:\PBL_SEM4\bms
git add .
git commit -m "Update: deskripsi perubahan"
git push
```

**Render akan auto-deploy** dalam 2-3 menit setelah push ke GitHub!


---

## 📊 Alternatif Cloud Gratis Lainnya

Jika Render tidak cocok, coba:

### 1. **Railway.app**
- ✅ 500 jam gratis/bulan
- ✅ Support WebSocket
- ✅ Deploy from GitHub
- ✅ Database PostgreSQL gratis
- 🔗 https://railway.app

### 2. **Fly.io**
- ✅ 3 VM gratis
- ✅ Support WebSocket
- ✅ Deploy from Dockerfile
- ⚠️ Lebih kompleks setup
- 🔗 https://fly.io

### 3. **PythonAnywhere**
- ✅ 100% gratis forever (limited)
- ⚠️ **TIDAK support WebSocket** (hanya HTTP)
- ❌ **TIDAK COCOK untuk BMS** (butuh WebSocket)
- 🔗 https://www.pythonanywhere.com

**Rekomendasi**: **Render.com** (paling mudah + support WebSocket)

---

## 📝 Checklist Deployment

Gunakan checklist ini untuk memastikan semua langkah sudah dilakukan:

- [ ] File `requirements.txt` sudah dibuat
- [ ] File `Procfile` sudah dibuat
- [ ] File `render.yaml` sudah dibuat
- [ ] File `.gitignore` sudah dibuat
- [ ] `app.py` sudah diupdate untuk production
- [ ] Project sudah diupload ke GitHub
- [ ] Service sudah di-deploy ke Render
- [ ] URL cloud server sudah dicatat
- [ ] ESP32 code sudah diupdate dengan URL cloud
- [ ] ESP32 sudah berhasil connect ke cloud server
- [ ] Dashboard bisa diakses dari browser
- [ ] Data sensor tampil real-time
- [ ] Semua fitur sudah di-test (logging, alarm, settings, debug)


---

## 🎓 Tips untuk Pemula

### Git Commands yang Sering Dipakai:

```cmd
# Cek status file yang berubah
git status

# Tambah semua file ke staging
git add .

# Commit dengan pesan
git commit -m "Pesan perubahan"

# Push ke GitHub
git push

# Pull update dari GitHub
git pull

# Lihat history commit
git log --oneline
```

### Render Dashboard:

- **Logs**: Lihat real-time logs server
- **Environment**: Set environment variables (untuk password, API key, dll)
- **Manual Deploy**: Deploy ulang manual (klik tombol)
- **Settings**: Ubah konfigurasi service

### UptimeRobot (Keep Server Awake):

1. Daftar gratis: https://uptimerobot.com
2. Add New Monitor
3. Monitor Type: **HTTP(s)**
4. Friendly Name: `BMS Server`
5. URL: `https://bms-server-xxxx.onrender.com`
6. Monitoring Interval: **5 minutes**
7. Save

Sekarang server Anda tidak akan sleep!


---

## 📞 Jika Ada Masalah

### Resources:

1. **Render Documentation**: https://render.com/docs
2. **GitHub Docs**: https://docs.github.com
3. **ESP32 WebSocket SSL**: https://github.com/Links2004/arduinoWebSockets

### Common Errors & Solutions:

| Error | Penyebab | Solusi |
|-------|----------|--------|
| `Module not found` | Library tidak ada | Tambah ke `requirements.txt` |
| `Port already in use` | Port conflict | Restart service |
| `WebSocket connection failed` | SSL/WSS issue | Gunakan `beginSSL()` |
| `ESP32 tidak connect` | URL/port salah | Cek `SERVER_HOST` dan `PORT` |
| `Database empty` | Deploy reset DB | Gunakan PostgreSQL (persistent) |
| `Service sleeping` | No traffic 15 min | Setup UptimeRobot |

---

## 🎉 Selamat!

Anda telah berhasil deploy BMS ke cloud!

**Apa yang sudah Anda capai**:
- ✅ Project di-manage dengan Git & GitHub
- ✅ Server Flask di-deploy ke Render (gratis)
- ✅ HTTPS & WSS aktif (secure)
- ✅ ESP32 connect ke cloud server
- ✅ Dashboard bisa diakses dari mana saja
- ✅ Real-time monitoring & control via internet

**Next Level**:
- Setup PostgreSQL untuk database persistent
- Add user authentication dengan JWT
- Implementasi email notification untuk alarm
- Add mobile app (Flutter/React Native)
- Multi-device support (ESP32-02, ESP32-03, ...)

---

**Happy Deploying! 🚀**

*Last Updated: 2026-07-14*  
*BMS Project - Cloud Deployment Guide*

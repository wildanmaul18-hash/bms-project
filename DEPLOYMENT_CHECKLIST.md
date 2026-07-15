# ✅ Checklist Deploy BMS ke Cloud

## 📋 Sebelum Deploy

- [ ] Semua file deployment sudah ada:
  - [ ] `requirements.txt`
  - [ ] `Procfile`
  - [ ] `render.yaml`
  - [ ] `.gitignore`
- [ ] File `app.py` sudah diupdate untuk production
- [ ] Akun GitHub sudah dibuat
- [ ] Akun Render.com sudah dibuat
- [ ] Git sudah terinstall di komputer

## 📋 Upload ke GitHub

- [ ] Repository GitHub sudah dibuat (public/private)
- [ ] Git init sudah dijalankan
- [ ] File sudah di-commit (`git add .` dan `git commit`)
- [ ] Project sudah di-push ke GitHub (`git push`)
- [ ] Cek di GitHub - semua file sudah terupload

## 📋 Deploy ke Render

- [ ] Repository sudah diconnect ke Render
- [ ] Service configuration sudah benar:
  - [ ] Name: `bms-server` (atau nama lain)
  - [ ] Region: Singapore
  - [ ] Runtime: Python 3
  - [ ] Build Command: `pip install -r requirements.txt`
  - [ ] Start Command: `gunicorn --worker-class eventlet -w 1 --bind 0.0.0.0:$PORT app:app`
  - [ ] Plan: Free
- [ ] Deploy sudah selesai (status: Live)
- [ ] URL cloud server sudah dicatat: `https://________.onrender.com`
- [ ] Dashboard bisa dibuka dari browser
- [ ] Bisa login dengan `admin` / `admin123`

## 📋 Update ESP32

- [ ] File `esp32_bms.ino` sudah diedit:
  - [ ] WiFi SSID & password sudah benar
  - [ ] `SERVER_HOST` sudah diganti dengan URL Render (tanpa https://)
  - [ ] `SERVER_PORT` sudah diganti ke `443`
  - [ ] Code sudah ditambahkan: `#include <WiFiClientSecure.h>`
  - [ ] `ws.begin()` sudah diganti dengan `ws.beginSSL()`
- [ ] Code sudah diupload ke ESP32
- [ ] Serial Monitor menunjukkan:
  - [ ] WiFi connected
  - [ ] WebSocket connected ke cloud server
  - [ ] Data sensor terkirim

## 📋 Testing

- [ ] Dashboard menunjukkan ESP32 status "Connected" (hijau)
- [ ] Data sensor update setiap 5 detik:
  - [ ] Temperature
  - [ ] Humidity
  - [ ] Light
- [ ] Chart menampilkan data real-time
- [ ] Fan animation berfungsi (spinning saat alarm)
- [ ] Room status update (Used/Unused Room)
- [ ] Halaman Logging berfungsi
- [ ] Halaman Alarm berfungsi
- [ ] Halaman Settings berfungsi (update threshold)
- [ ] Halaman Users berfungsi (admin only)
- [ ] Halaman Debug berfungsi:
  - [ ] LED ON/OFF command
  - [ ] Fan ON/OFF command
  - [ ] Ping ESP32
  - [ ] Restart ESP32

## 📋 Keep-Alive Setup (Opsional)

Agar server tidak sleep setelah 15 menit:

- [ ] Akun UptimeRobot sudah dibuat
- [ ] Monitor sudah ditambahkan:
  - [ ] Type: HTTP(s)
  - [ ] URL: `https://________.onrender.com`
  - [ ] Interval: 5 minutes
- [ ] Monitor status: Active

## 🎉 Deployment Berhasil!

Jika semua checklist diatas sudah ✅, maka:

- ✅ Server BMS sudah online 24/7
- ✅ Bisa diakses dari mana saja
- ✅ ESP32 connect ke cloud server
- ✅ Dashboard real-time monitoring berfungsi
- ✅ Semua fitur sudah teruji

## 📝 Catatan Penting

1. **URL Cloud Server**: `https://__________.onrender.com`
2. **Admin Login**: `admin` / `admin123`
3. **GitHub Repo**: `https://github.com/USERNAME/REPO_NAME`
4. **UptimeRobot**: (jika dipakai) - Keep server awake

## 🔄 Cara Update Setelah Deploy

Jika ada perubahan code:

```bash
cd i:\PBL_SEM4\bms
git add .
git commit -m "Deskripsi perubahan"
git push
```

Render akan **auto-deploy** dalam 2-3 menit!

---

**Selamat! Project BMS Anda sudah production-ready! 🚀**

*Last Updated: 2026-07-14*

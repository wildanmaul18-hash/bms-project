# 🔔 Setup Telegram Notification untuk BMS

## 📌 Overview

Fitur ini akan mengirim notifikasi otomatis ke Telegram saat ada alarm (temperature/humidity/light melebihi threshold).

**Keuntungan**:
- ✅ **100% Gratis** (no limit)
- ✅ **Real-time** notification di HP
- ✅ **Mudah setup** (5-10 menit)
- ✅ **Tidak ada spam** (rate limit 5 menit)

---

## 🤖 Step 1: Buat Telegram Bot (5 Menit)

### 1.1. Buka Telegram App

Di HP atau Desktop, buka aplikasi Telegram.

### 1.2. Cari BotFather

1. Search: `@BotFather`
2. Atau buka link: https://t.me/BotFather
3. Klik **Start**

### 1.3. Buat Bot Baru

Kirim command ke BotFather:

```
/newbot
```

BotFather akan bertanya:

**1. Nama Bot**:
```
BMS Monitoring Bot
```
(Atau nama lain yang Anda mau)

**2. Username Bot** (harus diakhiri `bot`):
```
bms_monitoring_bot
```
(Harus unik, jika sudah dipakai coba yang lain: `bms_monitoring_v2_bot`)

### 1.4. Simpan Bot Token

BotFather akan memberikan **Bot Token** seperti ini:

```
123456789:ABCdefGHIjklMNOpqrsTUVwxyz1234567890
```

⚠️ **PENTING**: Simpan token ini dengan aman! Jangan share ke orang lain.

---

## 💬 Step 2: Dapatkan Chat ID (3 Menit)

### 2.1. Start Bot Anda

1. Klik link bot yang diberikan BotFather
2. Atau search username bot Anda di Telegram
3. Klik **Start**
4. Kirim pesan apa saja, contoh: `Hello`

### 2.2. Dapatkan Chat ID

**Cara A: Via Browser** (Recommended)

1. Buka browser
2. Ganti `<TOKEN>` dengan bot token Anda:
   ```
   https://api.telegram.org/bot<TOKEN>/getUpdates
   ```
   
   Contoh:
   ```
   https://api.telegram.org/bot123456789:ABCdefGHI/getUpdates
   ```

3. Cari bagian `"chat":{"id":123456789}`
4. Copy angka `123456789` (itu Chat ID Anda)

**Cara B: Via Bot GetID** (Alternatif)

1. Search bot: `@userinfobot`
2. Klik Start
3. Bot akan kirim Chat ID Anda

### 2.3. Simpan Chat ID

Contoh Chat ID: `123456789` atau `987654321`

⚠️ Jangan simpan angka negatif atau yang ada huruf, harus angka positif saja.

---

## ⚙️ Step 3: Set Environment Variable di Railway

### 3.1. Buka Railway Dashboard

1. Login ke Railway: https://railway.app
2. Pilih project **bms-project**
3. Klik service Anda

### 3.2. Tambah Variables

1. Klik tab **"Variables"**
2. Klik **"+ New Variable"**

**Variable 1: Bot Token**
```
Key:   TELEGRAM_BOT_TOKEN
Value: 123456789:ABCdefGHIjklMNOpqrsTUVwxyz1234567890
```
(Ganti dengan token Anda)

**Variable 2: Chat ID**
```
Key:   TELEGRAM_CHAT_ID
Value: 123456789
```
(Ganti dengan chat ID Anda)

3. Klik **"Add"** untuk setiap variable

---

## 🚀 Step 4: Deploy

### 4.1. Push Code ke GitHub

```bash
cd i:\PBL_SEM4\bms
git add .
git commit -m "Add Telegram notification feature"
git push
```

### 4.2. Railway Auto-Deploy

Railway akan auto-deploy dalam 2-3 menit.

---

## 🧪 Step 5: Test Notification

### 5.1. Cek Logs Railway

Setelah deploy selesai, buka **Logs** di Railway.

**Jika berhasil**, akan muncul:
```
[TELEGRAM] Enabled
```

**Jika gagal** (token/chat ID salah):
```
[TELEGRAM] Disabled - Bot token or Chat ID not configured
```

### 5.2. Trigger Alarm

**Cara A: Via ESP32** (Real Test)

1. Pastikan ESP32 terhubung ke server
2. Ubah threshold di **Settings** (turunkan agar mudah trigger alarm)
   - Contoh: Temperature Max = 20°C (pasti alarm karena suhu ruangan > 20)
3. Tunggu ESP32 kirim data
4. **Notifikasi masuk ke Telegram!** 🎉

**Cara B: Via Debug Page** (Test Manual)

1. Login ke BMS
2. Buka **Debug** page
3. (Feature ini bisa ditambahkan: button "Test Telegram Notification")

### 5.3. Contoh Notifikasi

Telegram akan mengirim pesan seperti ini:

```
⚠️ ALARM BMS
━━━━━━━━━━━━━━━━━━━━━━

🌡️ TEMPERATURE: 38.5 °C
📊 Threshold: 35.0 °C
🚨 Status: HIGH ALARM
📍 Device: ESP32-01
⏰ Time: 2026-07-15 22:45:00

[Lihat Dashboard]
```

---

## 🔧 Troubleshooting

### Notifikasi Tidak Masuk

**1. Cek Logs Railway**

Cari error message:
```
[TELEGRAM] ❌ Failed: 401 - Unauthorized
```

**Solusi**: Bot token salah, cek ulang token di Variables.

---

**2. Cek Chat ID**

Error:
```
[TELEGRAM] ❌ Failed: 400 - Chat not found
```

**Solusi**: Chat ID salah, cek ulang chat ID.

---

**3. Notifikasi Tidak Keluar (Rate Limit)**

Logs:
```
[TELEGRAM] Skipped (rate limit): temperature_HIGH
```

**Ini normal!** Notifikasi hanya dikirim 1x per 5 menit untuk parameter yang sama (hindari spam).

**Solusi**: Tunggu 5 menit atau trigger alarm parameter lain (humidity/light).

---

**4. Bot Token/Chat ID Tidak Ter-set**

Logs:
```
[TELEGRAM] Disabled - Bot token or Chat ID not configured
```

**Solusi**:
1. Cek Railway Variables → Pastikan ada `TELEGRAM_BOT_TOKEN` dan `TELEGRAM_CHAT_ID`
2. Redeploy service

---

### Test Manual (Advanced)

Jika ingin test manual tanpa trigger alarm, tambahkan endpoint test di `app.py`:

```python
@app.route('/api/test-telegram')
@login_required
def test_telegram():
    if session.get('role') != 'admin':
        return jsonify({'error': 'Forbidden'}), 403
    
    send_telegram_notification(
        'temperature', 
        38.5, 
        35.0, 
        'HIGH',
        datetime.now(TIMEZONE_WIB).strftime('%Y-%m-%d %H:%M:%S')
    )
    
    return jsonify({'ok': True, 'msg': 'Test notification sent!'})
```

Akses: `https://bms-project.up.railway.app/api/test-telegram`

---

## 🎨 Customization (Opsional)

### 1. Ubah Pesan Notifikasi

Edit fungsi `send_telegram_notification()` di `app.py`:

```python
message = f"""🔔 *BMS ALERT*

Suhu terlalu tinggi!
Nilai: {value}°C
Batas: {threshold}°C

Cek dashboard sekarang!
"""
```

### 2. Kirim ke Group Telegram

1. Buat group di Telegram
2. Tambahkan bot Anda ke group
3. Dapatkan Group Chat ID (sama caranya, tapi hasilnya angka negatif)
4. Update `TELEGRAM_CHAT_ID` di Railway Variables dengan Group Chat ID

**Keuntungan**: Semua admin di group dapat notifikasi!

### 3. Kirim Notifikasi dengan Button

Update fungsi di `app.py`:

```python
import json

keyboard = {
    "inline_keyboard": [
        [
            {"text": "🌡️ Lihat Dashboard", "url": "https://bms-project.up.railway.app"},
        ],
        [
            {"text": "📊 Lihat Logging", "url": "https://bms-project.up.railway.app/logging"},
        ]
    ]
}

data = {
    "chat_id": TELEGRAM_CHAT_ID,
    "text": message,
    "parse_mode": "Markdown",
    "reply_markup": json.dumps(keyboard)
}
```

### 4. Ubah Cooldown Time

Edit di `app.py`:

```python
NOTIFICATION_COOLDOWN = 300  # Default: 5 menit (300 detik)
```

Ubah jadi:
```python
NOTIFICATION_COOLDOWN = 60   # 1 menit
NOTIFICATION_COOLDOWN = 600  # 10 menit
NOTIFICATION_COOLDOWN = 0    # Disabled (kirim setiap alarm)
```

---

## 📊 Rate Limiting

Sistem menggunakan **rate limiting** untuk hindari spam:

- **Cooldown**: 5 menit
- **Per Parameter**: Temperature, Humidity, Light ditrack terpisah
- **Per Level**: HIGH dan LOW ditrack terpisah

**Contoh**:
```
22:00 → Temperature HIGH alarm → Notifikasi dikirim ✅
22:02 → Temperature HIGH alarm → Skipped (rate limit) ⏭️
22:06 → Temperature HIGH alarm → Notifikasi dikirim ✅ (sudah 5 menit)

22:03 → Humidity LOW alarm → Notifikasi dikirim ✅ (parameter berbeda)
```

---

## 🔐 Security Tips

1. ⚠️ **Jangan commit** Bot Token ke GitHub
   - Gunakan environment variables
   - Jangan hardcode di `app.py`

2. ⚠️ **Protect Bot Token**
   - Token = password untuk bot Anda
   - Jangan share ke orang lain

3. ✅ **Use HTTPS only**
   - Railway sudah HTTPS otomatis
   - Komunikasi ke Telegram API aman

---

## 📚 Referensi

- [Telegram Bot API Documentation](https://core.telegram.org/bots/api)
- [BotFather Commands](https://core.telegram.org/bots#botfather)
- [Railway Environment Variables](https://docs.railway.app/guides/variables)

---

## ✅ Checklist Setup

- [ ] Buat bot via @BotFather
- [ ] Simpan Bot Token
- [ ] Start bot dan kirim pesan
- [ ] Dapatkan Chat ID
- [ ] Set `TELEGRAM_BOT_TOKEN` di Railway Variables
- [ ] Set `TELEGRAM_CHAT_ID` di Railway Variables
- [ ] Push code ke GitHub
- [ ] Railway auto-deploy
- [ ] Cek logs: "TELEGRAM Enabled"
- [ ] Test trigger alarm
- [ ] Notifikasi masuk ke Telegram ✅

---

**Setup Time**: ~10 menit  
**Cost**: 🆓 Free Forever  
**Difficulty**: ⭐ Easy

---

**Last Updated**: 2026-07-15  
**Status**: Production Ready 🚀

# Setup Railway Volume - Data Persisten (10 Menit)

## 🎯 Tujuan

Agar data logging **TIDAK HILANG** setelah restart/redeploy Railway.

---

## 📋 Langkah-Langkah

### Step 1: Buka Railway Dashboard

1. Login ke Railway: https://railway.app
2. Pilih project **bms-project**
3. Klik service Anda (bms-project-production)

---

### Step 2: Buat Volume

1. Klik tab **"Settings"** (di menu atas)
2. Scroll ke bawah cari section **"Volumes"**
3. Klik tombol **"+ New Volume"** atau **"Add Volume"**

---

### Step 3: Konfigurasi Volume

Isi form:

```
Mount Path: /data
```

**PENTING**: Harus `/data` (sesuai dengan kode di `database.py`)

Klik **"Add"** atau **"Create"**

---

### Step 4: Deploy Ulang

#### Cara A: Auto Deploy (Recommended)

Push kode terbaru ke GitHub:

```bash
cd i:\PBL_SEM4\bms
git add .
git commit -m "Add Railway Volume support for persistent database"
git push
```

Railway akan **auto-redeploy** dalam 2-3 menit.

#### Cara B: Manual Redeploy

Di Railway Dashboard:
1. Tab **"Deployments"**
2. Klik **"Redeploy"** pada deployment terakhir

---

### Step 5: Verifikasi

Setelah deployment selesai, cek **Logs**:

Seharusnya muncul:

```
[DB] Using Railway persistent volume: /data
[DB] Database path: /data/bms.db
[DB] Database siap.
```

✅ Jika muncul text di atas = **Volume berhasil!**

❌ Jika muncul:
```
[DB] Using local directory: database/
```
= Volume belum aktif, coba redeploy lagi.

---

### Step 6: Test Data Persistence

1. **Buka Dashboard BMS**: `https://bms-project-production.up.railway.app`
2. **Login**: `admin` / `admin123`
3. **Tunggu ESP32 kirim data** (5-10 menit)
4. **Buka Logging page** → Lihat data muncul
5. **Restart service** di Railway:
   - Settings → Restart
6. **Buka Logging lagi** → **Data masih ada!** ✅

---

## 🎉 Hasil

Setelah setup Volume:

✅ Database disimpan di `/data/bms.db` (persistent disk)  
✅ Data **TIDAK HILANG** setelah restart  
✅ Data **TIDAK HILANG** setelah redeploy  
✅ Data historis tetap ada  
✅ Filter tanggal berfungsi normal  

---

## 📊 Struktur Data

### Sebelum Volume:

```
Railway Container (ephemeral)
└── /app/
    ├── app.py
    ├── database.py
    └── database/
        └── bms.db  ← HILANG saat restart ❌
```

### Setelah Volume:

```
Railway Container (ephemeral)
└── /app/
    ├── app.py
    └── database.py

Railway Volume (persistent) ✅
└── /data/
    └── bms.db  ← TETAP ADA saat restart ✅
```

---

## ⚠️ Catatan Penting

### Kapasitas Volume

Railway free tier:
- **1 GB** volume storage (gratis)
- Cukup untuk **ribuan** data logging

### Backup Data

Volume bersifat persistent, tapi tetap disarankan backup berkala:

1. Railway Dashboard → Service → Volume
2. Klik **"Download"** untuk backup
3. Atau export CSV dari halaman Logging

### Delete Volume

⚠️ **HATI-HATI**: Jika volume dihapus, **semua data hilang permanen!**

Cara delete (jika perlu):
1. Settings → Volumes
2. Klik ikon **trash/delete**
3. Confirm

---

## 🔍 Troubleshooting

### Volume Tidak Terdeteksi

**Cek Logs**:
```
[DB] Using local directory: database/
```

**Solusi**:
1. Pastikan Mount Path = `/data` (huruf kecil semua)
2. Redeploy service
3. Wait 2-3 menit

### Data Masih Hilang

**Kemungkinan**:
- Volume belum dibuat
- Mount path salah (bukan `/data`)
- Service belum redeploy setelah volume dibuat

**Solusi**:
1. Cek Settings → Volumes → Pastikan ada volume dengan mount `/data`
2. Redeploy service
3. Cek logs → pastikan `Using Railway persistent volume: /data`

### Volume Penuh

**Cek kapasitas**:
- Railway Dashboard → Service → Volumes
- Lihat usage (0.5 MB / 1 GB)

**Solusi jika penuh**:
- Delete data lama dari halaman Logging
- Atau upgrade Railway plan

---

## 📚 Referensi

- [Railway Volumes Documentation](https://docs.railway.app/guides/volumes)
- [SQLite Best Practices](https://www.sqlite.org/bestpractice.html)

---

## ✅ Checklist Setup

- [ ] Buka Railway Dashboard
- [ ] Buat Volume dengan mount path `/data`
- [ ] Push code ke GitHub
- [ ] Railway auto-redeploy
- [ ] Cek logs: `Using Railway persistent volume: /data`
- [ ] ESP32 kirim data
- [ ] Cek Logging page → ada data
- [ ] Restart service
- [ ] Cek Logging page lagi → data masih ada ✅

---

**Waktu setup**: ~10 menit  
**Difficulty**: ⭐ Easy  
**Cost**: 🆓 Free (1 GB)

---

**Last Updated**: 2026-07-15  
**Status**: Ready to Deploy

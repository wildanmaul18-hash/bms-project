# Railway Database Persistence - Solusi Data Hilang

## 🔴 Masalah

### 1. Data Logging Hilang Setiap Restart

**Gejala**:
- Sistem baru dimulai → Data logging kosong
- Data session sebelumnya hilang
- Tidak ada data historis

**Penyebab**:
Railway menggunakan **ephemeral filesystem**:
- File system bersifat **sementara**
- Setiap restart/redeploy → **container baru** → **database kosong**
- File `bms.db` dibuat ulang dari awal

### 2. Filter Tanggal Tidak Ada Efek

**Gejala**:
- Ubah tanggal/jam di filter
- Klik "Tampilkan"
- Tidak ada data yang muncul

**Penyebab**:
- Database kosong (tidak ada data historis)
- Atau format tanggal tidak cocok dengan query

---

## ✅ Solusi 1: Railway Volume (Persistent Disk)

### Cara Setup Railway Volume:

#### Step 1: Buat Volume di Railway

1. **Railway Dashboard** → Pilih service BMS
2. Tab **"Settings"**
3. Scroll ke **"Volumes"**
4. Klik **"+ New Volume"**
5. Isi:
   - **Mount Path**: `/data`
   - **Name**: `bms-database`
6. Klik **"Add"**

#### Step 2: Update `database.py`

Ubah path database ke volume:

```python
import sqlite3
import hashlib
import os

# BASE_DIR adalah folder tempat file database.py berada
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# Cek jika ada volume mount (Railway)
if os.path.exists('/data'):
    # Production: Gunakan persistent volume
    DB_DIR = '/data'
    print('[DB] Using persistent volume: /data')
else:
    # Development: Gunakan folder lokal
    DB_DIR = os.path.join(BASE_DIR, "database")
    print('[DB] Using local directory: database/')

# Membuat folder database jika belum ada
os.makedirs(DB_DIR, exist_ok=True)

DB_PATH = os.path.join(DB_DIR, "bms.db")
```

#### Step 3: Push & Deploy

```bash
git add database.py
git commit -m "Use Railway volume for persistent database"
git push
```

#### Hasil:

✅ Database disimpan di `/data/bms.db` (persistent)  
✅ Data **TIDAK HILANG** setelah restart/redeploy  
✅ Data historis tetap ada  

---

## ✅ Solusi 2: Railway PostgreSQL (Recommended)

### Keuntungan PostgreSQL:

✅ **Persistent** (data tidak hilang)  
✅ **Scalable** (untuk production)  
✅ **Free tier** tersedia (512 MB storage)  
✅ **Backup otomatis**  
✅ **Multi-connection support**  

### Cara Setup PostgreSQL:

#### Step 1: Buat Database PostgreSQL

1. **Railway Dashboard** → **New** → **Database**
2. Pilih **PostgreSQL**
3. Klik **Add PostgreSQL**
4. Tunggu provisioning selesai
5. Copy **Database URL** (CONNECTION_URL)

#### Step 2: Install psycopg2

Update `requirements.txt`:

```txt
Flask==3.0.0
Flask-SocketIO==5.3.5
python-socketio==5.10.0
simple-websocket==1.0.0
Werkzeug==3.0.1
gunicorn==21.2.0
pytz==2024.1
psycopg2-binary==2.9.9  # ← Tambahkan ini
```

#### Step 3: Update `database.py`

```python
import os
import hashlib

# Cek environment untuk connection string
DATABASE_URL = os.environ.get('DATABASE_URL')

if DATABASE_URL:
    # Production: PostgreSQL
    import psycopg2
    from psycopg2.extras import RealDictCursor
    
    def get_db():
        conn = psycopg2.connect(DATABASE_URL)
        return conn
    
    def init_db():
        conn = get_db()
        cur = conn.cursor()
        
        # Buat tabel users
        cur.execute('''
            CREATE TABLE IF NOT EXISTS users (
                id       SERIAL PRIMARY KEY,
                username TEXT UNIQUE NOT NULL,
                password TEXT NOT NULL,
                role     TEXT NOT NULL DEFAULT 'user'
            )
        ''')
        
        # Buat tabel sensor_log
        cur.execute('''
            CREATE TABLE IF NOT EXISTS sensor_log (
                id          SERIAL PRIMARY KEY,
                datetime    TIMESTAMP NOT NULL,
                temperature REAL,
                humidity    REAL,
                light       REAL
            )
        ''')
        
        # ... dst (convert semua CREATE TABLE ke PostgreSQL syntax)
        
        conn.commit()
        conn.close()

else:
    # Development: SQLite (tetap sama)
    import sqlite3
    
    BASE_DIR = os.path.dirname(os.path.abspath(__file__))
    DB_DIR = os.path.join(BASE_DIR, "database")
    os.makedirs(DB_DIR, exist_ok=True)
    DB_PATH = os.path.join(DB_DIR, "bms.db")
    
    def get_db():
        conn = sqlite3.connect(DB_PATH)
        conn.row_factory = sqlite3.Row
        return conn
    
    # ... (fungsi lainnya tetap sama)
```

#### Step 4: Set Environment Variable di Railway

1. Railway Dashboard → Service → **Variables**
2. Klik **"+ New Variable"**
3. **Key**: `DATABASE_URL`
4. **Value**: (paste connection URL PostgreSQL dari step 1)
5. Save

#### Step 5: Deploy

```bash
git add .
git commit -m "Migrate to PostgreSQL for persistent storage"
git push
```

---

## 🔍 Troubleshooting Filter Tanggal

### Debug di Browser Console

Buka halaman Logging, tekan **F12** (DevTools), tab **Console**.

Klik tombol **"Tampilkan"**, perhatikan output:

```
[FILTER] From: 2026-07-15 00:00:00 To: 2026-07-16 23:59:59
[DATA] Loaded: 0 records
```

### Jika `Loaded: 0 records`:

**Kemungkinan 1**: Database memang kosong
- Solusi: Tunggu ESP32 kirim data sensor
- Atau gunakan persistent storage (Volume/PostgreSQL)

**Kemungkinan 2**: Format tanggal salah
- Cek format di database: `YYYY-MM-DD HH:MM:SS`
- Cek timezone: WIB vs UTC

**Kemungkinan 3**: Range tanggal tidak cocok
- Data di database: `2026-07-15 22:00:00` (WIB)
- Filter set: `2026-07-15 00:00:00` s/d `2026-07-15 21:00:00`
- Data tidak masuk range!

### Fix: Auto-set Range ke "Hari Ini"

Update `logging.html`:

```javascript
// Set default filter: Hari ini (00:00 - 23:59)
const now = new Date();
const todayStart = new Date(now.getFullYear(), now.getMonth(), now.getDate(), 0, 0);
const todayEnd = new Date(now.getFullYear(), now.getMonth(), now.getDate(), 23, 59);

document.getElementById('from-dt').value = toLocal(todayStart);
document.getElementById('to-dt').value   = toLocal(todayEnd);
```

---

## 📊 Perbandingan Solusi

| Fitur | SQLite (Ephemeral) | Railway Volume | PostgreSQL |
|-------|-------------------|----------------|------------|
| **Data Persist** | ❌ Hilang saat restart | ✅ Persisten | ✅ Persisten |
| **Setup** | ✅ Mudah (sudah ada) | ⚠️ Medium | ⚠️ Medium |
| **Performance** | ⚠️ Single connection | ⚠️ File-based | ✅ Optimal |
| **Scalability** | ❌ Terbatas | ⚠️ Terbatas | ✅ Scalable |
| **Backup** | ❌ Manual | ⚠️ Manual | ✅ Otomatis |
| **Cost** | ✅ Gratis | ✅ Gratis (1GB) | ✅ Gratis (512MB) |

### Rekomendasi:

- **Development/Testing**: SQLite (ephemeral) - OK
- **Production (Small)**: Railway Volume + SQLite
- **Production (Scalable)**: PostgreSQL ⭐ (Recommended)

---

## 🧪 Test Data Persistence

### Test Railway Volume:

1. Deploy dengan Volume
2. ESP32 kirim data sensor (tunggu 5-10 menit)
3. Cek halaman Logging → Ada data
4. Restart service di Railway
5. Cek halaman Logging lagi → **Data masih ada** ✅

### Test PostgreSQL:

1. Deploy dengan PostgreSQL
2. ESP32 kirim data sensor
3. Redeploy aplikasi (push commit baru)
4. Cek halaman Logging → **Data masih ada** ✅

---

## 📝 Summary

### Masalah 1: Data Hilang
- **Penyebab**: Railway ephemeral filesystem
- **Solusi**: Railway Volume atau PostgreSQL

### Masalah 2: Filter Tidak Berfungsi
- **Penyebab**: Database kosong atau timezone mismatch
- **Solusi**: 
  - Gunakan persistent storage
  - Fix timezone (sudah dilakukan)
  - Debug console untuk cek data

---

## 🚀 Langkah Selanjutnya

### Pilihan A: Quick Fix (Railway Volume)
1. Buat Volume di Railway Settings
2. Update `database.py` untuk pakai `/data`
3. Push & deploy
4. **Waktu**: 10 menit

### Pilihan B: Production Ready (PostgreSQL)
1. Buat PostgreSQL di Railway
2. Update `requirements.txt` (tambah psycopg2)
3. Update `database.py` (support PostgreSQL)
4. Set environment variable
5. Push & deploy
6. **Waktu**: 30 menit

---

**Rekomendasi**: Jika ini untuk **demo/presentasi**, pakai **Railway Volume** (lebih cepat).  
Jika untuk **production**, pakai **PostgreSQL** (lebih robust).

---

**Last Updated**: 2026-07-15  
**Status**: Documented - Awaiting Implementation

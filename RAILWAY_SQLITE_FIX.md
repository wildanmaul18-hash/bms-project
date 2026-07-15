# Fix SQLite Database untuk Railway

## 🔴 Masalah

Saat aplikasi dijalankan di Railway muncul error:

```
sqlite3.OperationalError: unable to open database file
```

### Penyebabnya:

1. File database (`database/bms.db`) **tidak di-upload** ke GitHub karena ada di `.gitignore`:
   ```gitignore
   database/bms.db
   ```

2. Folder `database/` juga **tidak ter-upload** karena Git tidak menyimpan folder kosong

3. Akibatnya SQLite mencoba membuka:
   ```
   /app/database/bms.db
   ```
   
   Padahal folder:
   ```
   /app/database/
   ```
   **belum ada!**

---

## ✅ Solusi

### 1. Update `database.py`

Buat folder `database/` secara otomatis sebelum membuka koneksi SQLite:

```python
import sqlite3
import hashlib
import os

# BASE_DIR adalah folder tempat file database.py berada
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DB_DIR = os.path.join(BASE_DIR, "database")

# Membuat folder database jika belum ada (penting untuk Railway/Cloud)
os.makedirs(DB_DIR, exist_ok=True)

DB_PATH = os.path.join(DB_DIR, "bms.db")


def get_db():
    """Buka koneksi ke database."""
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn
```

**Penjelasan**:
- `os.makedirs(DB_DIR, exist_ok=True)` → Buat folder `database/` jika belum ada
- `exist_ok=True` → Tidak error jika folder sudah ada
- Kode ini dijalankan saat `database.py` di-import pertama kali

---

### 2. Buat File `.gitkeep`

Buat file `database/.gitkeep` agar folder `database/` ter-track di Git:

```bash
# File: database/.gitkeep
# File ini untuk memastikan folder database/ ter-upload ke Git
# Database file (bms.db) akan dibuat otomatis saat pertama kali run
```

**Mengapa `.gitkeep`?**
- Git tidak bisa track folder kosong
- `.gitkeep` adalah file placeholder agar folder ter-upload
- File `bms.db` tetap di-ignore di `.gitignore`

---

### 3. Verifikasi `.gitignore`

Pastikan `.gitignore` sudah benar:

```gitignore
# Database (akan dibuat otomatis di cloud)
database/bms.db
```

**Catatan**:
- ✅ Folder `database/` → **Ter-upload** (karena ada `.gitkeep`)
- ❌ File `bms.db` → **Tidak ter-upload** (di-ignore)

---

### 4. Verifikasi `app.py`

Pastikan `init_db()` dipanggil saat aplikasi start:

```python
from flask import Flask
from database import init_db

app = Flask(__name__)

if __name__ == '__main__':
    init_db()  # ← Membuat tabel dan user default
    import os
    port = int(os.environ.get('PORT', 5000))
    print(f'[BMS] Server starting on port {port}')
    socketio.run(app, host='0.0.0.0', port=port, debug=False)
```

**Flow Inisialisasi**:
1. Import `database.py` → Folder `database/` dibuat otomatis
2. Panggil `init_db()` → File `bms.db` dibuat + tabel + user default
3. Server mulai menerima request

---

## 🎯 Hasil

Saat aplikasi pertama kali dijalankan di Railway, sistem akan otomatis:

1. ✅ Membuat folder `database/`
2. ✅ Membuat file `bms.db`
3. ✅ Membuat seluruh tabel (users, sensor_log, alarm_history, threshold)
4. ✅ Membuat user default:
   - Username: `admin`
   - Password: `admin123`
   - Role: `admin`

---

## 📝 Catatan Penting

### Database Ephemeral di Railway

⚠️ **PENTING**: Database SQLite di Railway bersifat **ephemeral** (sementara):

- Data akan **hilang** setiap kali redeploy/restart
- Railway **tidak menyimpan** perubahan file system
- Cocok untuk **testing/demo**, tapi **TIDAK untuk production**

### Solusi untuk Production

Gunakan database persistent seperti **PostgreSQL**:

1. Railway Dashboard → **New** → **PostgreSQL**
2. Free tier (cukup untuk BMS)
3. Update `database.py` untuk connect ke PostgreSQL
4. Install library: `pip install psycopg2-binary`

**Atau** gunakan **Railway Volume** (persistent disk):
- Railway Dashboard → Service Settings → **Add Volume**
- Mount path: `/app/database`
- Database akan persist setelah restart

---

## 🚀 Langkah Deploy

Setelah melakukan perubahan:

```bash
cd i:\PBL_SEM4\bms
git add database.py database/.gitkeep
git commit -m "Fix: Create database folder automatically for Railway"
git push
```

Railway akan melakukan **redeploy** secara otomatis.

Setelah deployment selesai, aplikasi dapat diakses **tanpa error**:

```
✅ Database folder created
✅ Database file created
✅ Tables initialized
✅ Default user created
✅ Server started successfully
```

---

## 🔍 Troubleshooting

### Error masih muncul setelah deploy

1. **Cek Railway Logs**:
   - Railway Dashboard → Service → **Logs**
   - Cari error message: `OperationalError: unable to open database file`

2. **Verifikasi folder dibuat**:
   - Logs seharusnya menunjukkan: `[DB] Database siap.`
   - Jika tidak muncul, berarti `init_db()` tidak dipanggil

3. **Test manual di Railway Shell**:
   ```bash
   # Railway Dashboard → Service → Settings → "Open Shell"
   ls -la database/
   # Seharusnya muncul: .gitkeep dan bms.db
   ```

### Database reset setiap deploy

Ini normal untuk SQLite di Railway (ephemeral filesystem).

**Solusi**:
- Gunakan PostgreSQL (persistent)
- Atau Railway Volume (persistent disk)

---

## 📚 Referensi

- [Railway Documentation - Persistent Storage](https://docs.railway.app/guides/volumes)
- [Flask-SQLite Tutorial](https://flask.palletsprojects.com/en/2.3.x/patterns/sqlite3/)
- [Git .gitkeep Convention](https://git-scm.com/docs/gitignore)

---

**Last Updated**: 2026-07-15  
**Status**: ✅ Resolved

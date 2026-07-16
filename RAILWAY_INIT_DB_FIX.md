# Fix `init_db()` untuk Deployment Railway

## 🔴 Masalah

Setelah berhasil memperbaiki error:
```
sqlite3.OperationalError: unable to open database file
```

Muncul error baru:
```
sqlite3.OperationalError: no such table: users
```

Log Railway menunjukkan:
```
File "/app/database.py", line XX, in get_user_by_username
    user = conn.execute(...)
sqlite3.OperationalError: no such table: users
```

---

## 🔍 Penyebab

Fungsi `init_db()` hanya dipanggil di dalam blok:

```python
if __name__ == "__main__":
    init_db()
```

### Cara ini berfungsi di lokal:

Saat menjalankan:
```bash
python app.py
```

Python set `__name__ = "__main__"`, sehingga `init_db()` dipanggil.

### Masalah di Railway/Gunicorn:

Railway menjalankan aplikasi dengan **Gunicorn**:
```bash
gunicorn app:app
```

Gunicorn **mengimpor** aplikasi, bukan menjalankan file:
```python
from app import app
```

Saat diimpor:
- `__name__ = "app"` (bukan `"__main__"`)
- Blok `if __name__ == "__main__":` **tidak dijalankan**
- `init_db()` **tidak dipanggil**
- Database **tidak diinisialisasi**

### Akibatnya:

❌ Tabel tidak dibuat (`users`, `sensor_log`, `alarm_history`, `threshold`)  
❌ Login gagal karena tabel `users` tidak ada  
❌ Error: `no such table: users`

---

## ✅ Solusi

Panggil `init_db()` **di luar** blok `if __name__ == "__main__":`

### ❌ Sebelum (Salah untuk Gunicorn):

```python
app = Flask(__name__)
app.secret_key = 'bms-secret-key-ganti-ini'
socketio = SocketIO(app, cors_allowed_origins='*')

# ... kode lainnya ...

if __name__ == "__main__":
    init_db()  # ← Hanya dipanggil saat `python app.py`
    import os
    port = int(os.environ.get("PORT", 5000))
    socketio.run(app, host="0.0.0.0", port=port, debug=False)
```

### ✅ Sesudah (Benar untuk Semua Deployment):

```python
app = Flask(__name__)
app.secret_key = 'bms-secret-key-ganti-ini'
socketio = SocketIO(app, cors_allowed_origins='*')

# Inisialisasi database saat aplikasi di-load (penting untuk Gunicorn/Railway)
init_db()  # ← Dipanggil saat aplikasi diimpor

# ... kode lainnya ...

if __name__ == "__main__":
    import os
    port = int(os.environ.get("PORT", 5000))
    socketio.run(app, host="0.0.0.0", port=port, debug=False)
```

---

## 🎯 Mengapa Solusi Ini Berhasil?

### Flow Eksekusi:

#### Local Development (`python app.py`):
```
1. Import modules
2. Create Flask app
3. Create SocketIO
4. init_db() ← Dijalankan
5. Define routes & functions
6. __name__ == "__main__" → True
7. socketio.run() starts server
```

#### Railway/Gunicorn (`gunicorn app:app`):
```
1. Import modules
2. Create Flask app
3. Create SocketIO
4. init_db() ← Dijalankan
5. Define routes & functions
6. __name__ == "__main__" → False (skip)
7. Gunicorn serves the app
```

**Kunci**: `init_db()` dipanggil **saat import**, bukan saat run.

---

## 🔄 Database Initialization Flow

```
Gunicorn starts
    ↓
from app import app
    ↓
app = Flask(__name__)
    ↓
socketio = SocketIO(...)
    ↓
init_db() ← DATABASE CREATED HERE
    ↓
  ┌─────────────────────┐
  │ os.makedirs()       │
  │ database/ created   │
  └─────────────────────┘
    ↓
  ┌─────────────────────┐
  │ CREATE TABLE users  │
  │ CREATE TABLE logs   │
  │ CREATE TABLE alarms │
  │ CREATE TABLE thresh │
  └─────────────────────┘
    ↓
  ┌─────────────────────┐
  │ INSERT admin user   │
  │ INSERT thresholds   │
  └─────────────────────┘
    ↓
[DB] Database siap.
    ↓
Gunicorn accepts requests
```

---

## 📊 Hasil

Saat aplikasi dijalankan pertama kali di Railway:

✅ Folder `database/` dibuat otomatis  
✅ File `bms.db` dibuat  
✅ Tabel `users`, `sensor_log`, `alarm_history`, `threshold` dibuat  
✅ User default dibuat:
   - Username: `admin`
   - Password: `admin123`
   - Role: `admin`

---

## 🧪 Testing

### Test di Railway Logs:

Setelah deploy, cek logs Railway, seharusnya muncul:

```
[DB] Database siap.
[BMS] Server starting on port 8080
Listening at: http://0.0.0.0:8080
Using worker: sync
```

### Test Login:

1. Buka URL Railway: `https://your-app.up.railway.app`
2. Login dengan:
   - Username: `admin`
   - Password: `admin123`
3. ✅ Login berhasil → Dashboard terbuka
4. ❌ Jika error `no such table: users` → `init_db()` tidak dipanggil

---

## 💡 Catatan Penting

### Kompatibilitas

Solusi ini **compatible** dengan:

✅ **Local Development**: `python app.py`  
✅ **Gunicorn**: `gunicorn app:app`  
✅ **Railway**: Auto-deploy  
✅ **Render**: Auto-deploy  
✅ **Docker**: Container deployment  
✅ **VPS Linux**: Systemd/Supervisor  

### Performance

**Apakah `init_db()` dipanggil berulang kali?**

❌ **Tidak!** 

`init_db()` menggunakan:
```sql
CREATE TABLE IF NOT EXISTS users (...)
INSERT OR IGNORE INTO users (...)
```

- `IF NOT EXISTS` → Hanya buat tabel jika belum ada
- `INSERT OR IGNORE` → Hanya insert jika belum ada

Jadi aman dipanggil berulang kali tanpa duplicate data.

### Database Ephemeral Warning

⚠️ SQLite di Railway bersifat **ephemeral**:
- Data hilang setiap redeploy/restart
- Cocok untuk **demo/testing**
- **TIDAK untuk production**

**Solusi Production**: PostgreSQL atau Railway Volume

---

## 🚀 Langkah Deploy

Setelah melakukan perubahan:

```bash
cd i:\PBL_SEM4\bms
git add app.py
git commit -m "Fix: Initialize database on application startup for Gunicorn"
git push
```

Railway akan melakukan **redeploy** secara otomatis.

Setelah deployment selesai:

1. ✅ Database dibuat otomatis
2. ✅ Tabel dibuat
3. ✅ User admin dibuat
4. ✅ Login berhasil
5. ✅ Aplikasi berfungsi normal

---

## 🔍 Troubleshooting

### Error masih muncul: `no such table: users`

**Cek Railway Logs**:
1. Apakah muncul: `[DB] Database siap.`?
2. Jika tidak muncul, berarti `init_db()` tidak dipanggil

**Solusi**:
1. Pastikan `init_db()` ada **di luar** blok `if __name__ == "__main__":`
2. Pastikan tidak ada syntax error di `database.py`
3. Redeploy Railway

### Database kosong setelah restart

Ini **normal** untuk SQLite di Railway (ephemeral filesystem).

**Solusi**: Gunakan PostgreSQL atau Railway Volume.

---

## 📚 Referensi

- [Gunicorn Application Loading](https://docs.gunicorn.org/en/stable/run.html#application-names)
- [Flask Application Context](https://flask.palletsprojects.com/en/2.3.x/appcontext/)
- [Python `__name__` Variable](https://docs.python.org/3/library/__main__.html)

---

**Last Updated**: 2026-07-15  
**Status**: ✅ Resolved  
**Deployment**: Railway Compatible

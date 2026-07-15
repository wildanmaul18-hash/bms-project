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
    conn.row_factory = sqlite3.Row  # hasil query bisa diakses seperti dict
    return conn


def init_db():
    """Buat semua tabel jika belum ada, dan buat user admin default."""
    conn = get_db()
    cur = conn.cursor()

    # Tabel users
    cur.execute('''
        CREATE TABLE IF NOT EXISTS users (
            id       INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT    UNIQUE NOT NULL,
            password TEXT    NOT NULL,
            role     TEXT    NOT NULL DEFAULT 'user'
        )
    ''')

    # Tabel sensor_log
    cur.execute('''
        CREATE TABLE IF NOT EXISTS sensor_log (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            datetime    TEXT    NOT NULL,
            temperature REAL,
            humidity    REAL,
            light       REAL
        )
    ''')

    # Tabel alarm_history
    cur.execute('''
        CREATE TABLE IF NOT EXISTS alarm_history (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            datetime    TEXT    NOT NULL,
            parameter   TEXT    NOT NULL,
            value       REAL    NOT NULL,
            threshold   REAL    NOT NULL,
            level       TEXT    NOT NULL,
            acknowledged INTEGER DEFAULT 0
        )
    ''')

    # Tabel threshold
    cur.execute('''
        CREATE TABLE IF NOT EXISTS threshold (
            parameter TEXT PRIMARY KEY,
            minimum   REAL NOT NULL,
            maximum   REAL NOT NULL
        )
    ''')

    # Buat user admin default jika belum ada
    password_hash = hashlib.sha256('admin123'.encode()).hexdigest()
    cur.execute('''
        INSERT OR IGNORE INTO users (username, password, role)
        VALUES (?, ?, ?)
    ''', ('admin', password_hash, 'admin'))

    # Threshold default
    defaults = [
        ('temperature', 18.0, 35.0),
        ('humidity',    40.0, 90.0),
        ('light',      100.0, 1000.0),
    ]
    for param, mn, mx in defaults:
        cur.execute('''
            INSERT OR IGNORE INTO threshold (parameter, minimum, maximum)
            VALUES (?, ?, ?)
        ''', (param, mn, mx))

    conn.commit()
    conn.close()
    print('[DB] Database siap.')


# ── Helper: Users ─────────────────────────────────────────────

def get_user_by_username(username):
    conn = get_db()
    user = conn.execute(
        'SELECT * FROM users WHERE username = ?', (username,)
    ).fetchone()
    conn.close()
    return user


def verify_password(plain, hashed):
    return hashlib.sha256(plain.encode()).hexdigest() == hashed


# ── Helper: Sensor Log ────────────────────────────────────────

def save_sensor(datetime_str, temperature, humidity, light):
    conn = get_db()
    conn.execute('''
        INSERT INTO sensor_log (datetime, temperature, humidity, light)
        VALUES (?, ?, ?, ?)
    ''', (datetime_str, temperature, humidity, light))
    conn.commit()
    conn.close()


def get_sensor_logs(limit=100):
    conn = get_db()
    rows = conn.execute('''
        SELECT * FROM sensor_log ORDER BY datetime DESC LIMIT ?
    ''', (limit,)).fetchall()
    conn.close()
    return [dict(r) for r in rows]


def get_sensor_logs_by_range(from_dt, to_dt):
    conn = get_db()
    rows = conn.execute('''
        SELECT * FROM sensor_log
        WHERE datetime BETWEEN ? AND ?
        ORDER BY datetime ASC
    ''', (from_dt, to_dt)).fetchall()
    conn.close()
    return [dict(r) for r in rows]


# ── Helper: Alarm ─────────────────────────────────────────────

def save_alarm(datetime_str, parameter, value, threshold, level):
    conn = get_db()
    conn.execute('''
        INSERT INTO alarm_history (datetime, parameter, value, threshold, level)
        VALUES (?, ?, ?, ?, ?)
    ''', (datetime_str, parameter, value, threshold, level))
    conn.commit()
    conn.close()


def get_alarms(limit=100):
    conn = get_db()
    rows = conn.execute('''
        SELECT * FROM alarm_history ORDER BY datetime DESC LIMIT ?
    ''', (limit,)).fetchall()
    conn.close()
    return [dict(r) for r in rows]


# ── Helper: Threshold ─────────────────────────────────────────

def get_thresholds():
    conn = get_db()
    rows = conn.execute('SELECT * FROM threshold').fetchall()
    conn.close()
    return {r['parameter']: {'min': r['minimum'], 'max': r['maximum']} for r in rows}


def update_threshold(parameter, minimum, maximum):
    conn = get_db()
    conn.execute('''
        UPDATE threshold SET minimum = ?, maximum = ? WHERE parameter = ?
    ''', (minimum, maximum, parameter))
    conn.commit()
    conn.close()


# ── Helper: User Management ───────────────────────────────────

def get_all_users():
    conn = get_db()
    rows = conn.execute('SELECT id, username, role FROM users').fetchall()
    conn.close()
    return [dict(r) for r in rows]

def add_user(username, password, role='user'):
    import hashlib
    password_hash = hashlib.sha256(password.encode()).hexdigest()
    conn = get_db()
    try:
        conn.execute('INSERT INTO users (username, password, role) VALUES (?, ?, ?)',
                     (username, password_hash, role))
        conn.commit()
        conn.close()
        return True, 'User berhasil ditambahkan.'
    except Exception as e:
        conn.close()
        return False, 'Username sudah digunakan.'

def delete_user(user_id):
    conn = get_db()
    conn.execute('DELETE FROM users WHERE id = ?', (user_id,))
    conn.commit()
    conn.close()

def change_password(user_id, new_password):
    import hashlib
    password_hash = hashlib.sha256(new_password.encode()).hexdigest()
    conn = get_db()
    conn.execute('UPDATE users SET password = ? WHERE id = ?', (password_hash, user_id))
    conn.commit()
    conn.close()

def change_role(user_id, role):
    conn = get_db()
    conn.execute('UPDATE users SET role = ? WHERE id = ?', (role, user_id))
    conn.commit()
    conn.close()

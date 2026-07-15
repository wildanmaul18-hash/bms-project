from flask import Flask, render_template, request, redirect, url_for, session, jsonify
from flask_socketio import SocketIO, emit
from database import (init_db, get_user_by_username, verify_password,
                      save_sensor, get_sensor_logs, get_sensor_logs_by_range,
                      save_alarm, get_alarms, get_thresholds, update_threshold,
                      get_all_users, add_user, delete_user, change_password, change_role)
import functools
from datetime import datetime

app = Flask(__name__)
app.secret_key = 'bms-secret-key-ganti-ini'
socketio = SocketIO(app, cors_allowed_origins='*')

last_sensor_data = {
    'temperature': None, 'humidity': None, 'light': None,
    'timestamp': None, 'device_id': None, 'connected': False,
    'room_is_used': False,  # Status apakah ruangan digunakan
    'fan_status': 'off'      # Status kipas (on/off)
}


# ── Decorator ─────────────────────────────────────────────────

def login_required(f):
    @functools.wraps(f)
    def wrapper(*args, **kwargs):
        if 'user_id' not in session:
            return redirect(url_for('login'))
        return f(*args, **kwargs)
    return wrapper


# ── Auth ──────────────────────────────────────────────────────

@app.route('/')
def index():
    return redirect(url_for('dashboard') if 'user_id' in session else url_for('login'))

@app.route('/login', methods=['GET', 'POST'])
def login():
    error = None
    if request.method == 'POST':
        user = get_user_by_username(request.form.get('username', '').strip())
        if user and verify_password(request.form.get('password', ''), user['password']):
            session.update({'user_id': user['id'], 'username': user['username'], 'role': user['role']})
            return redirect(url_for('dashboard'))
        error = 'Username atau password salah.'
    return render_template('login.html', error=error)

@app.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('login'))


# ── Halaman ───────────────────────────────────────────────────

@app.route('/dashboard')
@login_required
def dashboard():
    return render_template('dashboard.html', username=session['username'], role=session['role'])

@app.route('/logging')
@login_required
def logging_page():
    return render_template('logging.html', username=session['username'], role=session['role'])

@app.route('/alarm')
@login_required
def alarm_page():
    if session.get('role') != 'admin':
        return redirect(url_for('dashboard'))
    return render_template('alarm.html', username=session['username'], role=session['role'])

@app.route('/debug')
@login_required
def debug():
    if session.get('role') != 'admin':
        return redirect(url_for('dashboard'))
    return render_template('debug.html', username=session['username'], role=session['role'])

@app.route('/settings')
@login_required
def settings():
    if session.get('role') != 'admin':
        return redirect(url_for('dashboard'))
    return render_template('settings.html',
                           username=session['username'],
                           role=session['role'])

@app.route('/users')
@login_required
def users_page():
    if session.get('role') != 'admin':
        return redirect(url_for('dashboard'))
    return render_template('users.html',
                           username=session['username'],
                           role=session['role'])


# ── API: User Management ──────────────────────────────────────

@app.route('/api/users', methods=['GET'])
@login_required
def api_users_get():
    if session.get('role') != 'admin':
        return jsonify({'error': 'Forbidden'}), 403
    return jsonify(get_all_users())

@app.route('/api/users', methods=['POST'])
@login_required
def api_users_add():
    if session.get('role') != 'admin':
        return jsonify({'error': 'Forbidden'}), 403
    data = request.get_json()
    username = data.get('username', '').strip()
    password = data.get('password', '')
    role     = data.get('role', 'user')
    if not username or not password:
        return jsonify({'ok': False, 'msg': 'Username dan password wajib diisi.'}), 400
    ok, msg = add_user(username, password, role)
    return jsonify({'ok': ok, 'msg': msg})

@app.route('/api/users/<int:uid>', methods=['DELETE'])
@login_required
def api_users_delete(uid):
    if session.get('role') != 'admin':
        return jsonify({'error': 'Forbidden'}), 403
    if uid == session['user_id']:
        return jsonify({'ok': False, 'msg': 'Tidak bisa hapus akun sendiri.'}), 400
    delete_user(uid)
    return jsonify({'ok': True})

@app.route('/api/users/<int:uid>/password', methods=['POST'])
@login_required
def api_users_password(uid):
    if session.get('role') != 'admin':
        return jsonify({'error': 'Forbidden'}), 403
    data = request.get_json()
    new_pw = data.get('password', '')
    if not new_pw:
        return jsonify({'ok': False, 'msg': 'Password tidak boleh kosong.'}), 400
    change_password(uid, new_pw)
    return jsonify({'ok': True, 'msg': 'Password berhasil diubah.'})

@app.route('/api/users/<int:uid>/role', methods=['POST'])
@login_required
def api_users_role(uid):
    if session.get('role') != 'admin':
        return jsonify({'error': 'Forbidden'}), 403
    if uid == session['user_id']:
        return jsonify({'ok': False, 'msg': 'Tidak bisa ubah role sendiri.'}), 400
    data = request.get_json()
    change_role(uid, data.get('role', 'user'))
    return jsonify({'ok': True})


# ── API ───────────────────────────────────────────────────────

@app.route('/api/latest')
@login_required
def api_latest():
    return jsonify(last_sensor_data)

@app.route('/api/logs')
@login_required
def api_logs():
    from_dt = request.args.get('from', '')
    to_dt   = request.args.get('to', '')
    if from_dt and to_dt:
        logs = get_sensor_logs_by_range(from_dt, to_dt)
    else:
        logs = get_sensor_logs(limit=200)
    return jsonify(logs)

@app.route('/api/alarms')
@login_required
def api_alarms():
    return jsonify(get_alarms(limit=200))

@app.route('/api/threshold', methods=['GET'])
@login_required
def api_threshold_get():
    return jsonify(get_thresholds())

@app.route('/api/threshold', methods=['POST'])
@login_required
def api_threshold_post():
    data = request.get_json()
    for param, vals in data.items():
        update_threshold(param, vals['min'], vals['max'])
    
    # Broadcast threshold update ke semua client (termasuk ESP32)
    thresholds = get_thresholds()
    socketio.emit('threshold_update', thresholds)
    print(f'[THRESHOLD] Updated and broadcasted: {thresholds}')
    
    return jsonify({'status': 'ok'})


@app.route('/api/stats')
@login_required
def api_stats():
    from_dt = request.args.get('from', '')
    to_dt   = request.args.get('to', '')
    logs = get_sensor_logs_by_range(from_dt, to_dt) if (from_dt and to_dt) else get_sensor_logs(limit=200)
    if not logs:
        return jsonify({})
    result = {}
    for param in ['temperature', 'humidity', 'light']:
        vals = [r[param] for r in logs if r[param] is not None]
        if vals:
            result[param] = {
                'min': round(min(vals), 1),
                'max': round(max(vals), 1),
                'avg': round(sum(vals) / len(vals), 1)
            }
    return jsonify(result)


# ── WebSocket ─────────────────────────────────────────────────

@socketio.on('connect')
def on_connect():
    emit('sensor_update', last_sensor_data)
    # Kirim threshold ke client yang baru connect
    thresholds = get_thresholds()
    emit('threshold_update', thresholds)

@socketio.on('disconnect')
def on_disconnect():
    print(f'[WS] Putus: {request.sid}')

@socketio.on('esp32_data')
def on_esp32_data(data):
    print(f'[ESP32] Data masuk: {data}')
    now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    
    temperature = data.get('temperature')
    humidity = data.get('humidity')
    light = data.get('light')
    
    # Cek apakah ruangan digunakan (berdasarkan cahaya)
    thresholds = get_thresholds()
    light_threshold = thresholds.get('light', {})
    temp_threshold = thresholds.get('temperature', {})
    
    room_is_used = light >= light_threshold.get('min', 100)
    temp_alarm = temperature > temp_threshold.get('max', 35)
    
    # LOGIKA KIPAS: ON jika ruangan digunakan DAN suhu tinggi
    fan_should_be_on = room_is_used and temp_alarm
    
    last_sensor_data.update({
        'temperature': temperature,
        'humidity':    humidity,
        'light':       light,
        'timestamp':   now,
        'device_id':   data.get('device_id', 'ESP32'),
        'connected':   True,
        'room_is_used': room_is_used,
        'fan_status': 'on' if fan_should_be_on else 'off'
    })
    
    save_sensor(now, temperature, humidity, light)
    
    # Kirim perintah kipas ke ESP32
    fan_command = {
        'type': 'command',
        'device': 'fan',
        'action': 'on' if fan_should_be_on else 'off'
    }
    socketio.emit('server_command', fan_command)
    print(f'[FAN CMD] Sent to ESP32: {fan_command["action"].upper()} (Room Used={room_is_used}, Temp Alarm={temp_alarm})')
    
    # Alarm hanya dikirim jika ruangan digunakan
    if room_is_used:
        check_alarm(data, now)
        print(f'[ROOM] Used - Alarm check active')
    else:
        print(f'[ROOM] Unused (Light={light}) - Alarm suppressed')
    
    # Broadcast data sensor
    socketio.emit('sensor_update', last_sensor_data)

@socketio.on('esp32_heartbeat')
def on_heartbeat(data):
    print(f'[ESP32] Heartbeat: uptime={data.get("uptime")}s')
    last_sensor_data['connected'] = True
    socketio.emit('heartbeat', data)

@socketio.on('request_threshold')
def on_request_threshold(data):
    print('[ESP32] Request threshold')
    thresholds = get_thresholds()
    socketio.emit('threshold_update', thresholds)

@socketio.on('browser_command')
def on_browser_command(data):
    print(f'[CMD] Command dari browser: {data}')
    # Forward command ke ESP32
    socketio.emit('server_command', data)


# ── Alarm Engine ──────────────────────────────────────────────

def check_alarm(data, now):
    thresholds = get_thresholds()
    for param in ['temperature', 'humidity', 'light']:
        value = data.get(param)
        if value is None:
            continue
        t = thresholds.get(param)
        if not t:
            continue
        level = 'HIGH' if value > t['max'] else ('LOW' if value < t['min'] else None)
        if level:
            threshold_val = t['max'] if level == 'HIGH' else t['min']
            save_alarm(now, param, value, threshold_val, level)
            print(f'[ALARM] {param} = {value} → {level}')
            socketio.emit('alarm', {'parameter': param, 'value': value, 'level': level, 'timestamp': now})


# ── Main ──────────────────────────────────────────────────────

if __name__ == '__main__':
    init_db()
    import os
    port = int(os.environ.get('PORT', 5000))
    print(f'[BMS] Server starting on port {port}')
    socketio.run(app, host='0.0.0.0', port=port, debug=False)

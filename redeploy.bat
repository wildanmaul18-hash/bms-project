@echo off
echo ========================================
echo  Force Redeploy ke Railway
echo ========================================
echo.

cd /d "%~dp0"

echo [1/2] Creating empty commit to trigger rebuild...
git commit --allow-empty -m "Trigger Railway rebuild with gevent"

echo.
echo [2/2] Pushing to GitHub...
git push

echo.
echo ========================================
echo  SELESAI! Railway akan rebuild otomatis
echo ========================================
echo.
echo Tunggu 2-3 menit, lalu cek Railway logs.
echo.
pause

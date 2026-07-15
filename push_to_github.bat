@echo off
echo ========================================
echo  Push BMS Project ke GitHub
echo ========================================
echo.

cd /d "%~dp0"

echo [1/3] Adding files...
git add .

echo.
echo [2/3] Committing changes...
git commit -m "Fix: Ganti eventlet ke gevent untuk Python 3.13 compatibility"

echo.
echo [3/3] Pushing to GitHub...
git push

echo.
echo ========================================
echo  SELESAI!
echo ========================================
echo.
pause

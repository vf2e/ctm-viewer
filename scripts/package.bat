@echo off
setlocal
cd /d "%~dp0.."

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0package.ps1" %*
set EXITCODE=%ERRORLEVEL%

if %EXITCODE% neq 0 (
    echo.
    echo [ERROR] Package failed, exit code %EXITCODE%
    pause
    exit /b %EXITCODE%
)

echo.
pause
exit /b 0

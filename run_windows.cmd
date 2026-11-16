@echo off
setlocal
cd /d "%~dp0"
call build_windows.cmd
if errorlevel 1 exit /b 1
if not exist results mkdir results
for %%N in (01_threads 02_race 03_mutex_atomic 04_async 05_stop_token 06_platform) do (
    echo.
    echo === %%N ===
    "build\%%N.exe"
    if errorlevel 1 exit /b 1
)
if not exist logs (
    build\generate_logs.exe logs
    if errorlevel 1 exit /b 1
)
build\logscan.exe logs ERROR 7 > results\benchmark_run.txt
if errorlevel 1 exit /b 1
type results\benchmark_run.txt

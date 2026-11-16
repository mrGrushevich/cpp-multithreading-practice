@echo off
setlocal
cd /d "%~dp0"
where cl >nul 2>nul
if errorlevel 1 (
    echo Open x64 Native Tools Command Prompt for VS, then run this script.
    exit /b 1
)
if not exist build mkdir build
for %%N in (01_threads 02_race 03_mutex_atomic 04_async 05_stop_token 06_platform generate_logs logscan) do (
    if "%%N"=="02_race" (
        cl /nologo /std:c++20 /EHsc /W4 /utf-8 /Od "src\%%N.cpp" /Fe:"build\%%N.exe" /Fo:"build\%%N.obj"
    ) else (
        cl /nologo /std:c++20 /EHsc /W4 /utf-8 /O2 "src\%%N.cpp" /Fe:"build\%%N.exe" /Fo:"build\%%N.obj"
    )
    if errorlevel 1 exit /b 1
)
echo Build complete.

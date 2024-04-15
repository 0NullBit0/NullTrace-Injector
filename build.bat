:: build for windows by nullbit

@echo off
setlocal EnableDelayedExpansion

set ABIS="arm64-v8a" "armeabi-v7a" "x86" "x86_64"
set BUILD_DIR=tmp_wbuild

for %%a in (%ABIS%) do (
    echo Building for ABI: %%a
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
    mkdir "%BUILD_DIR%"
    cmake -S . -B "%BUILD_DIR%" -G "Ninja" -DANDROID_ABI=%%a
    cmake --build "%BUILD_DIR%"
)

echo Press any key to continue . . .
pause > nul
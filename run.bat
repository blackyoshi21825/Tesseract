@echo off
REM Quick run script for Tesseract

REM Check if the executable exists in the build directory
if exist "build\tesser.exe" (
    build\tesser.exe %*
    exit /b
)

REM Check if the executable exists in the root directory
if exist "tesser.exe" (
    tesser.exe %*
    exit /b
)

REM If no executable found, build it first
echo Tesseract executable not found. Building...

REM Try CMake build first (faster)
if exist "build_win.bat" (
    call build_win.bat
    if exist "build\tesser.exe" (
        build\tesser.exe %*
        exit /b
    )
)

REM Fall back to make if CMake build failed
echo Trying make build...
make
if exist "tesser.exe" (
    tesser.exe %*
    exit /b
)

echo Failed to build Tesseract. Please check for errors.
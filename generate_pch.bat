@echo off
echo Generating precompiled header...

:: Use optimized flags for faster compilation
gcc -Wall -Wextra -std=c99 -Iinclude -O3 -ffast-math -flto -march=native -c include/tesseract_pch.h -o include/tesseract_pch.h.gch

if %ERRORLEVEL% neq 0 (
    echo Failed to generate precompiled header!
    exit /b %ERRORLEVEL%
)

echo Precompiled header generated successfully.
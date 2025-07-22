@echo off
echo Building Tesseract with CMake...

:: Generate precompiled header first
call generate_pch.bat

if not exist build mkdir build
cd build

cmake -G "MinGW Makefiles" ..
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b %ERRORLEVEL%
)

cmake --build . --config Release -j 8
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

echo Build completed successfully!
echo Executable is located at: %CD%\tesser.exe
cd ..
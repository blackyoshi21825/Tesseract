@echo off
echo Building Tesseract with CMake...

:: Determine number of CPU cores for optimal parallel compilation
for /f "tokens=2 delims=:" %%i in ('wmic cpu get NumberOfCores /value ^| find "NumberOfCores"') do set NUM_CORES=%%i

if not exist build mkdir build
cd build

:: Use Ninja generator if available for faster builds
where ninja >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo Using Ninja build system for faster compilation
    cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
) else (
    echo Using MinGW Makefiles
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
)

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b %ERRORLEVEL%
)

:: Build with detected number of cores
cmake --build . --config Release -j %NUM_CORES%
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

echo Build completed successfully!
echo Executable is located at: %CD%\tesser.exe
cd ..
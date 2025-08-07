@echo off
echo Building Tesseract Package Manager...
gcc -Wall -Wextra -std=c99 -Iinclude -O2 -o tpm.exe packages/tpm.c packages/package_manager.c
if %errorlevel% equ 0 (
    echo TPM built successfully!
) else (
    echo Build failed!
)
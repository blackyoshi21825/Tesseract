@echo off
echo Generating precompiled header...
gcc -Wall -Wextra -std=c99 -Iinclude -O2 -ffast-math -c include/tesseract_pch.h -o include/tesseract_pch.h.gch
echo Done.
@echo off
echo Generating precompiled header...
gcc -c -Iinclude include/tesseract_pch.h -o obj/tesseract_pch.h.gch
echo Done!
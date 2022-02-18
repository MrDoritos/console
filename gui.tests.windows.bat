@echo off

g++ pixel.cpp lodepng.cpp console.windows.cpp gui.tests.cpp -I. -o gui.tests.windows.exe -w -fpermissive -ggdb > out.txt

if "%errorlevel%" NEQ "0" (
	more out.txt
) else ( 
	echo gui.tests.cpp -\> gui.tests.windows.exe
)

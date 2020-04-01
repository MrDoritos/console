@echo off

g++ console.windows.cpp console.tests.cpp -I. -o console.tests.windows.exe %* > out.txt

if "%errorlevel%" NEQ "0" (
	more out.txt
	) else (
	echo console.tests.cpp -\> console.tests.windows.exe
)


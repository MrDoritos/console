@echo off

cmd /c g++ console.windows.cpp console.tests.cpp -I. -o console.tests.windows.exe %*

if "%errorlevel%" NEQ "0" (
	pause
	) else (
	echo console.tests.cpp -\> console.tests.windows.exe
)


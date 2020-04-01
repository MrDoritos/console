#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#define NO_KEY EOF

#if defined __WIN32
#include <Windows.h>
//#include <console.windows.h>

#if defined DLLEXPORT
#define CONSOLE __declspec(dllexport)
#define CONSOLECALL __cdecl

extern "C" {
#else
#define CONSOLE
#define CONSOLECALL
#endif
#elif defined __linux__
//#include <console.linux.h>
#define CONSOLE
#define CONSOLECALL
#endif

#define BBLACK		0b00000000
#define BBLUE		0b00010000
#define BGREEN		0b00100000
#define BYELLOW		0b00110000
#define BRED		0b01000000
#define BMAGENTA	0b01010000
#define BCYAN		0b01100000
#define BWHITE		0b01110000

#define FBLACK		0b00000000
#define FBLUE		0b00000001
#define FGREEN		0b00000010
#define FYELLOW		0b00000011
#define FRED		0b00000100
#define FMAGENTA	0b00000101
#define FCYAN		0b00000110
#define FWHITE		0b00000111

#define IMAGE_WINDOWS 0x01
#define IMAGE_POSIX 0x02
#define IMAGE_LINUX IMAGE_POSIX

class console {
	public:
	CONSOLE static int CONSOLECALL getImage();	
	CONSOLE static void CONSOLECALL setCursorPosition(int x, int y);
	CONSOLE static void CONSOLECALL setCursorLeft(int x);
	CONSOLE static void CONSOLECALL setCursorTop(int y);
	CONSOLE static void CONSOLECALL setConsoleColor(int color); //bitwise OR the colors FG and BG
	CONSOLE static int CONSOLECALL getConsoleWidth();
	CONSOLE static int CONSOLECALL getConsoleHeight();
	CONSOLE static int CONSOLECALL readKey();
	CONSOLE static int CONSOLECALL readKeyAsync();
	CONSOLE static void CONSOLECALL clear();
	CONSOLE static void CONSOLECALL write(int x, int y, char character);
	CONSOLE static void CONSOLECALL write(int x, int y, char character, int color);
	CONSOLE static void CONSOLECALL write(char* fb, char* cb, int length);
	CONSOLE static void CONSOLECALL write(int x, int y, std::string& str);
	CONSOLE static void CONSOLECALL sleep(int millis);
	//CONSOLE static void invokeConstructor() {
	//	console::cons::constructor();
	//}
	//CONSOLE static void invokeDestructor() {
	//	console::cons::~constructor();
	//}
	private:
	friend class constructor;
	
	struct constructor {
		constructor();
		~constructor();
	};
	
	static constructor cons;
	
	private:
	CONSOLE static int _activeColor;
	#if defined __WIN32
	CONSOLE static HANDLE conHandle;
	CONSOLE static HANDLE inHandle;
	CONSOLE static HANDLE ogConHandle;
	CONSOLE static int CONSOLECALL _getCharInfoColor(int color);
	public:
	CONSOLE static void CONSOLECALL _construct() {		
		console::ogConHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		console::conHandle = CreateConsoleScreenBuffer(0x80000000U | 0x40000000U, 0, 0, 0x00000001, 0);
		console::_activeColor = BBLACK | FWHITE;
	
		SetConsoleActiveScreenBuffer(conHandle);
		SetConsoleActiveScreenBuffer(GetStdHandle(STD_INPUT_HANDLE));
		
		inHandle = GetStdHandle(STD_INPUT_HANDLE);
		SetConsoleMode(inHandle, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
	}
	CONSOLE static void CONSOLECALL _destruct() {
		SetConsoleActiveScreenBuffer(ogConHandle);
	}
	private:
	#elif defined __linux__
	static void _refreshSize();
	static struct winsize w;
	#endif
};

#if (defined __WIN32 && defined DLLEXPORT)
}	
#endif
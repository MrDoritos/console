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
//#define CONSOLECALL __cdecl
#define CONSOLECALL __stdcall

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

#ifndef __WIN32
#define VK_ESCAPE KEY_ESCAPE
#define VK_ENTER KEY_ENTER
#define VK_DELETE KEY_DELETE

#define VK_LEFT KEY_LEFT
#define VK_RIGHT KEY_RIGHT
#define VK_UP KEY_UP
#define VK_DOWN KEY_DOWN

//#define VK_CONTROL KEY_CONTROL
//#define VK_SHIFT KEY_SHIFT
#endif

#define VK_LEFT_ 0xCB
#define VK_UP_ 0xC8
#define VK_RIGHT_ 0xCD
#define VK_DOWN_ 0xD0

class console {
	public:
	CONSOLE static int CONSOLECALL getImage();	
	CONSOLE static void CONSOLECALL setCursorPosition(int x, int y);
	CONSOLE static void CONSOLECALL setCursorLeft(int x);
	CONSOLE static void CONSOLECALL setCursorTop(int y);
	CONSOLE static void CONSOLECALL setConsoleColor(int color);
	CONSOLE static int CONSOLECALL getConsoleWidth();
	CONSOLE static int CONSOLECALL getConsoleHeight();
	CONSOLE static int CONSOLECALL readKey();
	CONSOLE static int CONSOLECALL readKeyAsync();
	CONSOLE static void CONSOLECALL clear();
	CONSOLE static void CONSOLECALL write(int x, int y, char character);
	CONSOLE static void CONSOLECALL write(int x, int y, char character, char color);
	CONSOLE static void CONSOLECALL write(char* fb, char* cb, int length);
	CONSOLE static void CONSOLECALL write(wchar_t* fb, char* cb, int length);
	CONSOLE static void CONSOLECALL write(int x, int y, wchar_t character);
	CONSOLE static void CONSOLECALL write(int x, int y, wchar_t character, char color);
	CONSOLE static void CONSOLECALL write(int x, int y, std::string& str);
	CONSOLE static void CONSOLECALL write(int x, int y, std::string& str, char color);
	CONSOLE static void CONSOLECALL write(int x, int y, const char* str);
	CONSOLE static void CONSOLECALL write(int x, int y, const char* str, char color);
	CONSOLE static void CONSOLECALL sleep(int millis);
	CONSOLE static bool ready;
	
	public:
	friend class constructor;
	
	struct constructor {
		constructor();
		~constructor();
	};
	
	static constructor cons;	
	
	private:
	CONSOLE static int _activeColor;
	#if defined __WIN32
	public:
	CONSOLE static void CONSOLECALL write(CHAR_INFO* fb, int length);
	CONSOLE static HANDLE conHandle;
	CONSOLE static HANDLE inHandle;
	CONSOLE static HANDLE ogConHandle;
	private:
	CONSOLE static int CONSOLECALL _getCharInfoColor(int color);
	#elif defined __linux__	
	static void _refreshSize();
	static struct winsize w;
	public:
	static bool useRefresh;
	#endif
};

#if (defined __WIN32 && defined DLLEXPORT)
}	
#endif

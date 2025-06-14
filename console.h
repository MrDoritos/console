#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#if defined(__linux__) || defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define POSIX_COMP 
#endif
#if defined(__WIN32) || defined(_WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
#define WIN_COMP
#endif

#if defined(WIN_COMP)
#include <Windows.h>
#define CONSOLE
#define CONSOLECALL
#elif defined(POSIX_COMP)
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
#define BINST		0b10000000

#define FBLACK		0b00000000
#define FBLUE		0b00000001
#define FGREEN		0b00000010
#define FYELLOW		0b00000011
#define FRED		0b00000100
#define FMAGENTA	0b00000101
#define FCYAN		0b00000110
#define FWHITE		0b00000111
#define FINST		0b00001000	

#define IMAGE_WINDOWS 0x01
#define IMAGE_POSIX 0x02
#define IMAGE_LINUX IMAGE_POSIX

#if defined(POSIX_COMP)
#define VK_ESCAPE 27
#define VK_RETURN '\n' //On windows this would be the carriage return instead
#define VK_DELETE KEY_DELETE
#define VK_TAB '\t'
#define VK_SHIFT '?'
#define wmain main

#define VK_LEFT KEY_LEFT
#define VK_RIGHT KEY_RIGHT
#define VK_UP KEY_UP
#define VK_DOWN KEY_DOWN

#define VK_HOME KEY_HOME
#define VK_END KEY_END
#define VK_INSERT KEY_IC

#define VK_BACKSPACE 263 
#endif

#if defined(WIN_COMP)
#define VK_BACKSPACE VK_BACK
#endif

#define VK_ENTER VK_RETURN
#define NO_KEY EOF

#define __CTRL (0x08 << 24)
#define __NMLK (0x20 << 24)
#define __ALT (0b000001 << 24)
#define NOMOD(x) (x & 0x00FFFFFF)
#define HASMOD(x, mod) ((x & mod) == mod)
#define HASKEY(x, key) (NOMOD(x) == key)

typedef unsigned char color_t;

class console {
	public:
	CONSOLE static int CONSOLECALL getImage();	
	CONSOLE static void CONSOLECALL setCursorPosition(int x, int y);
	CONSOLE static void CONSOLECALL setCursorLeft(int x);
	CONSOLE static void CONSOLECALL setCursorTop(int y);
	CONSOLE static void CONSOLECALL setConsoleColor(color_t color);
	CONSOLE static int CONSOLECALL getConsoleWidth();
	CONSOLE static int CONSOLECALL getConsoleHeight();
	CONSOLE static int CONSOLECALL readKey();
	CONSOLE static int CONSOLECALL readKeyAsync();
	CONSOLE static void CONSOLECALL clear();
	CONSOLE static void CONSOLECALL write(int x, int y, char character);
	CONSOLE static void CONSOLECALL write(int x, int y, char character, color_t color);
	CONSOLE static void CONSOLECALL write(char* fb, color_t* cb, int length);
	CONSOLE static void CONSOLECALL write(wchar_t* fb, color_t* cb, int length);
	CONSOLE static void CONSOLECALL write(int x, int y, wchar_t character);
	CONSOLE static void CONSOLECALL write(int x, int y, wchar_t character, color_t color);
	CONSOLE static void CONSOLECALL write(int x, int y, std::string& str);
	CONSOLE static void CONSOLECALL write(int x, int y, std::string& str, color_t color);
	CONSOLE static void CONSOLECALL write(int x, int y, const char* str);
	CONSOLE static void CONSOLECALL write(int x, int y, const char* str, color_t color);
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
	CONSOLE static color_t _activeColor;
	#if defined(WIN_COMP)
	public:
	CONSOLE static void CONSOLECALL write(CHAR_INFO* fb, int length);
	CONSOLE static HANDLE conHandle;
	CONSOLE static HANDLE inHandle;
	CONSOLE static HANDLE ogConHandle;
	CONSOLE static color_t CONSOLECALL _getCharInfoColor(color_t color);
	private:
	#elif defined(POSIX_COMP)	
	static void _refreshSize();
	static struct winsize w;
	public:
	static bool useRefresh;
	#endif
};
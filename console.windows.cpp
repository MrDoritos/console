//Win32
#include <iostream>
#include <Windows.h>
#include "console.h"
#include <conio.h>
#include <stdio.h>

#include <bitset>

//DLL Creation...
//#if (defined DLLEXPORT && defined __WIN32)
//extern "C" {
//#define CONSOLE __declspec(dllimport)
//#define CONSOLE __declspec(dllexport)
//}
//#endif

//Define constructor
console::constructor console::cons;
HANDLE console::conHandle;
HANDLE console::inHandle;
HANDLE console::ogConHandle;
int console::_activeColor;
bool console::ready;

console::constructor::constructor() {	
    setlocale(LC_ALL, "");
	console::ogConHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	console::conHandle = CreateConsoleScreenBuffer(0x80000000U | 0x40000000U, 0, 0, 0x00000001, 0);
	console::_activeColor = BBLACK | FWHITE;
	
	#ifndef DEBUG
	{
		SetConsoleActiveScreenBuffer(conHandle);
		SetConsoleActiveScreenBuffer(GetStdHandle(STD_INPUT_HANDLE));
	}
	#endif
		
	inHandle = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(inHandle, ENABLE_WINDOW_INPUT);
	
	ready = true;
}

console::constructor::~constructor() {
	SetConsoleActiveScreenBuffer(ogConHandle);
		
	ready = false;
}

void console::sleep(int millis) {
	Sleep(millis);
}

int console::getImage() {
	return IMAGE_WINDOWS;
}

void console::write(int x, int y, std::string& str) {
	int length = str.length();
	//CHAR_INFO framebuffer[length]; //WTF CL.EXE	
	CHAR_INFO* framebuffer = (CHAR_INFO*)alloca(sizeof(CHAR_INFO) * length);
	
	for (int i = 0; i < length; i++) {
		framebuffer[i].Char.UnicodeChar = str[i];
		framebuffer[i].Attributes = console::_activeColor;
	}
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	SMALL_RECT srwin = { short(x), short(y), short(x + length - 1), short(y) };
	WriteConsoleOutput(console::conHandle, framebuffer, csbi.dwSize, {0,0}, &srwin);
}

void console::write(int x, int y, const char* str) {
	int length = strlen(str);
	CHAR_INFO* framebuffer = (CHAR_INFO*)alloca(sizeof(CHAR_INFO) * length);
	
	for (int i = 0; i < length; i++) {
		framebuffer[i].Char.UnicodeChar = str[i];
		framebuffer[i].Attributes = console::_activeColor;
	}
	
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	SMALL_RECT srwin = { short(x), short(y), short(x + length - 1), short(y) };
	WriteConsoleOutput(console::conHandle, framebuffer, csbi.dwSize, {0,0}, &srwin);
}

void console::write(int x, int y, const char* str, char color) {
	console::setConsoleColor(color);
	console::write(x,y,str);
}

void console::write(int x, int y, std::string& str, char color) {
	console::setConsoleColor(color);
	console::write(x,y,str);
}

int console::getConsoleWidth() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	return csbi.dwSize.X;
}

int console::getConsoleHeight() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	return csbi.dwSize.Y;
}

void console::clear() {
	COORD topLeft = { 0, 0 };
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD written;
	
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	FillConsoleOutputCharacterA(console::conHandle, ' ', csbi.dwSize.X * csbi.dwSize.Y, topLeft, &written);
	FillConsoleOutputAttribute(console::conHandle, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE, csbi.dwSize.X * csbi.dwSize.Y, topLeft, &written);
	SetConsoleCursorPosition(console::conHandle, topLeft);
}

void console::setConsoleColor(int color) {
	if (console::_activeColor == _getCharInfoColor(color)) {
		return;
	}
	console::_activeColor = _getCharInfoColor(color);
	SetConsoleTextAttribute(console::conHandle, console::_getCharInfoColor(color));
}

//Cyan and yellow are flipped with windows
int console::_getCharInfoColor(int color) {	
	if ((color & FWHITE) == FCYAN)
		color = (color & ~FWHITE) | FYELLOW;
	else
	if ((color & FWHITE) == FYELLOW)
		color = (color & ~FWHITE) | FCYAN;
	
	if ((color & BWHITE) == BCYAN)
		color = (color & ~BWHITE) | BYELLOW;
	else
	if ((color & BWHITE) == BYELLOW)
		color = (color & ~BWHITE) | BCYAN;	
	
	return color;
}

int console::readKeyAsync() {
	INPUT_RECORD irec;
	KEY_EVENT_RECORD krec;
	DWORD n;
	DWORD c;
	GetNumberOfConsoleInputEvents(console::inHandle, &c);
	if (c > 0) {
		ReadConsoleInput(console::inHandle, &irec, 1, &n);
		if (irec.EventType == KEY_EVENT && ((KEY_EVENT_RECORD&)irec.Event).bKeyDown) {
			char asciiChar;
			char modifiers = irec.Event.KeyEvent.dwControlKeyState;
			krec = (KEY_EVENT_RECORD&)irec.Event;
			asciiChar = krec.uChar.AsciiChar;
			if (asciiChar < 128 && asciiChar > 31)
				return asciiChar | (modifiers << 24);
			
			return irec.Event.KeyEvent.wVirtualKeyCode | (modifiers << 24);
		}
	}
	return 0;
}

int console::readKey() {
	INPUT_RECORD irec;
	KEY_EVENT_RECORD krec;
	DWORD n;
	while (true) {
		ReadConsoleInput(console::inHandle, &irec, 1, &n);
		if (irec.EventType == KEY_EVENT && ((KEY_EVENT_RECORD&)irec.Event).bKeyDown) {
			int asciiChar;
			char modifiers = irec.Event.KeyEvent.dwControlKeyState;
			krec = (KEY_EVENT_RECORD&)irec.Event;
			asciiChar = krec.uChar.AsciiChar;
			std::string p0 = std::bitset<8>(modifiers).to_string();
			std::string p1 = std::bitset<8>(asciiChar).to_string();
			std::string p2 = std::bitset<32>(irec.Event.KeyEvent.wVirtualKeyCode).to_string();
			fprintf(stderr, "mod:%i asc:%c bs:%s as:%s %s\r\n", modifiers, asciiChar, p0.c_str(), p1.c_str(), p2.c_str());
			if (asciiChar < 128 && asciiChar > 31)
				return asciiChar | (modifiers << 24);
			
			return irec.Event.KeyEvent.wVirtualKeyCode | (modifiers << 24);
		}
	}
	return 0;
}

void console::write(char* fb, char* cb, int length) {
	CHAR_INFO* framebuffer = (CHAR_INFO*)alloca(sizeof(CHAR_INFO) * length);
	
	wchar_t b;
	for (int i = 0; i < length; i++) {
		b = fb[i];
		b &= 0x00FF;
		framebuffer[i].Char.UnicodeChar = b;
		framebuffer[i].Attributes = _getCharInfoColor(cb[i]);
	}
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	SMALL_RECT srwin = { 0, 0, short(csbi.dwSize.X - 1), csbi.dwSize.Y };
	WriteConsoleOutput(console::conHandle, framebuffer, csbi.dwSize, {0,0}, &srwin);
}

void console::write(wchar_t* fb, char* cb, int length) {
	{
		CHAR_INFO framebuffer[length];
		for (int i = 0; i < length; i++) {
			framebuffer[i].Char.UnicodeChar = fb[i];
			framebuffer[i].Attributes = _getCharInfoColor(cb[i]);
		}
		console::write(&framebuffer[0], length);
	}
}

void console::write(CHAR_INFO* fb, int length) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	SMALL_RECT srwin = { 0, 0, short(csbi.dwSize.X - 1), csbi.dwSize.Y };
	WriteConsoleOutput(console::conHandle, fb, csbi.dwSize, {0,0}, &srwin);	
}

void console::write(int x, int y, char character, char color) {
	console::setConsoleColor(color);
	console::write(x, y, character);
}

void console::write(int x, int y, char character) {
	console::setCursorPosition(x, y);
	DWORD written;
	WriteConsole(console::conHandle, (const char*)&character, 1, &written, NULL);
}

void console::write(int x, int y, wchar_t character, char color) {
	console::setConsoleColor(color);
	console::write(x, y, character);
}

void console::write(int x, int y, wchar_t character) {
	console::setCursorPosition(x, y);
	DWORD written;
	WriteConsole(console::conHandle, (const wchar_t*)&character, 1, &written, NULL);
}

void console::setCursorPosition(int x, int y) {
	SetConsoleCursorPosition(console::conHandle, { short(x), short(y) });
}

void console::setCursorLeft(int x) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	SetConsoleCursorPosition(console::conHandle, { short(x), csbi.dwCursorPosition.Y });
}

void console::setCursorTop(int y) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	SetConsoleCursorPosition(console::conHandle, { csbi.dwCursorPosition.X, short(y) });
}

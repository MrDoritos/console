//Win32
#include <iostream>
#include <Windows.h>
#include <console.h>
#include <conio.h>
#include <stdio.h>

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

console::constructor::constructor() {
	console::_construct();
}

console::constructor::~constructor() {
	console::_destruct();
}

void console::sleep(int millis) {
	Sleep(millis);
}

int console::getImage() {
	return IMAGE_WINDOWS;
}

void console::write(int x, int y, std::string& str) {
	int length = str.length();
	CHAR_INFO framebuffer[length];
	for (int i = 0; i < length; i++) {
		framebuffer[i].Char.UnicodeChar = str[i];
		framebuffer[i].Attributes = FWHITE | BBLACK;
	}
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	SMALL_RECT srwin = { x, y, x + length - 1, y };
	WriteConsoleOutput(console::conHandle, framebuffer, csbi.dwSize, {0,0}, &srwin);
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

void console::write(int x, int y, char character) {
	//console::write(x, y, character, console::_activeColor);
	console::setCursorPosition(x, y);
	DWORD written;
	WriteConsole(console::conHandle, (const char*)&character, 1, &written, NULL);
}

void console::setConsoleColor(int color) {
	//fprintf(stderr, "Setting color %i->%i", console::_activeColor, color);
	if (console::_activeColor == _getCharInfoColor(color)) {
		return;
	}
	console::_activeColor = color;
	SetConsoleTextAttribute(console::conHandle, console::_getCharInfoColor(color));
}

int console::_getCharInfoColor(int color) {
	int attr = 0;
	//auto color_map = [](int i) {
	//	return i;
	//};
	attr |= (((color & 0b11110000) >> 4) << 4);
//	attr <<= 1;
	attr |= (color & 0b00001111);
	return attr;
}

int console::readKey() {
	//return getchar();
	INPUT_RECORD irec;
	DWORD n;
	ReadConsoleInput(console::inHandle, &irec, 1, &n);
	for (int i = 0; i < n; i++) {
		switch (irec.EventType) {
			case KEY_EVENT: 
			{
				return irec.Event.KeyEvent.uChar.AsciiChar;
			}
			break;
		}
	}
	return -1;
}

void console::write(char* fb, char* cb, int length) {
	CHAR_INFO framebuffer[length];
	wchar_t b;
	for (int i = 0; i < length; i++) {
		b = fb[i];
		b &= 0x00FF;
		framebuffer[i].Char.UnicodeChar = b;
		framebuffer[i].Attributes = cb[i];
	}
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	SMALL_RECT srwin = { 0, 0, csbi.dwSize.X - 1, csbi.dwSize.Y };
	WriteConsoleOutput(console::conHandle, framebuffer, csbi.dwSize, {0,0}, &srwin);
	//length -= 1;
	//for (int i = 0; i < length; i++) {
	//	console::write(i % console::getConsoleHeight(), i / console::getConsoleWidth() , fb[i], cb[i]);
	//}
}

void console::write(int x, int y, char character, int color) {		
	//SetConsoleTextAttribute(console::conHandle, console::_getCharInfoColor(color));
	console::setConsoleColor(color);
	console::write(x, y, character);
	//SetConsoleTextAttribute(console::conHandle, console::_getCharInfoColor(console::_activeColor));
	//CHAR_INFO charInfo;
	//charInfo.Char.AsciiChar = character;
	/*
	FG
	0x0001 == 0b0000001
	0x0002 == 0b0000010
	0x0004 == 0b0000100

	BG
	0x0010 == 0b0010000
	0x0020 == 0b0100000
	0b0040 == 0b1000000
	*/
	
	//charInfo.Attributes = console::_getCharInfoColor(color);
	
	//CONSOLE_SCREEN_BUFFER_INFO csbi;
	//GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	
	//SMALL_RECT srwin = { 0, 0, csbi.dwSize.X - 1, csbi.dwSize.Y - 1 };
	//WriteConsoleOutput(console::conHandle, (CHAR_INFO*)&charInfo, csbi.dwSize, {x, y}, &srwin);
}

void console::setCursorPosition(int x, int y) {
	//CONSOLE_SCREEN_BUFFER_INFO csbi;
	//GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	SetConsoleCursorPosition(console::conHandle, { x, y });
}

void console::setCursorLeft(int x) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	SetConsoleCursorPosition(console::conHandle, { x, csbi.dwCursorPosition.Y });
}

void console::setCursorTop(int y) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console::conHandle, &csbi);
	SetConsoleCursorPosition(console::conHandle, { csbi.dwCursorPosition.X, y });
}

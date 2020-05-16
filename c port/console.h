#include <Windows.h>
#include <conio.h>
#include <stdio.h>

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
#define VK_ESCAPE 0x1B
#define VK_ENTER 0x0D
#define VK_DELETE 0x2E
#define VK_CONTROL 0x11
#define VK_SHIFT 0x10
#endif

static int _activeColor;
static HANDLE conHandle;
static HANDLE inHandle;
static HANDLE ogConHandle;

void setCursorPosition(int x, int y) {
	COORD cd;
	cd.X = x;
	cd.Y = y;
	SetConsoleCursorPosition(conHandle, cd);
}

void setCursorLeft(int x) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(conHandle, &csbi);
	COORD cd;
	cd.X = x;
	cd.Y = csbi.dwCursorPosition.Y;
	SetConsoleCursorPosition(conHandle, cd);
}

void setCursorTop(int y) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(conHandle, &csbi);
	COORD cd;
	cd.Y = y;
	cd.X = csbi.dwCursorPosition.X;
	SetConsoleCursorPosition(conHandle, cd);
}

void writewxyc(int x, int y, wchar_t character) {
	setCursorPosition(x,y);
	DWORD written;
	WriteConsole(conHandle, (const wchar_t*)&character, 2, &written, NULL);
}

void writexyc(int x, int y, char character) {
	//write(x, y, character, _activeColor);
	setCursorPosition(x, y);
	DWORD written;
	WriteConsole(conHandle, (const char*)&character, 1, &written, NULL);
}

//Cyan and yellow are flipped with windows
int _getCharInfoColor(int color) {
	if (color & 0b00000011)
		color = (color ^ 0b00000111) | 0b00000110;
	else
		if (color & 0b00000110)
			color = (color ^ 0b00000111) | 0b00000011;
	
	if (color & 0b00110000)
		color = (color ^ 0b01110000) | 0b01100000;
	else
		if (color & 0b01100000)
			color = (color ^ 0b01110000) | 0b00110000;	
	return color;
}

void setConsoleColor(int color) {
	if (_activeColor == _getCharInfoColor(color)) {
		return;
	}
	_activeColor = color;
	SetConsoleTextAttribute(conHandle, _getCharInfoColor(color));
}

void writexycc(int x, int y, char character, char color) {
	setConsoleColor(color);
	writexyc(x, y, character);
}

void writexystr(int x, int y, const char* str) {
	int length = strlen(str);
	CHAR_INFO* framebuffer = (CHAR_INFO*)alloca(sizeof(CHAR_INFO) * length);
	
	for (int i = 0; i < length; i++) {
		framebuffer[i].Char.UnicodeChar = str[i];
		framebuffer[i].Attributes = _activeColor;
	}
	
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(conHandle, &csbi);
	//SMALL_RECT srwin = { short(x), short(y), short(x + length - 1), short(y) };
	
	SMALL_RECT srwin;
	srwin.Left = x; srwin.Top = y; srwin.Right = x + length - 1;
	srwin.Bottom = y;
	COORD cd;
	cd.X = 0;
	cd.Y = 0;
	WriteConsoleOutput(conHandle, framebuffer, csbi.dwSize, cd, &srwin);
}

void writexystrc(int x, int y, const char* str, char color) {
	setConsoleColor(color);
	writexystr(x,y,str);
}

int getConsoleWidth() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(conHandle, &csbi);
	return csbi.dwSize.X;
}

int getConsoleHeight() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(conHandle, &csbi);
	return csbi.dwSize.Y;
}

void csleep(int millis) {
	Sleep(millis);
}

int getImage() {
	return IMAGE_WINDOWS;
}

void cclear() {
	COORD topLeft;// = { 0, 0 };
	topLeft.X = 0;
	topLeft.Y = 0;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD written;
	
	GetConsoleScreenBufferInfo(conHandle, &csbi);
	FillConsoleOutputCharacterA(conHandle, ' ', csbi.dwSize.X * csbi.dwSize.Y, topLeft, &written);
	FillConsoleOutputAttribute(conHandle, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE, csbi.dwSize.X * csbi.dwSize.Y, topLeft, &written);
	SetConsoleCursorPosition(conHandle, topLeft);
}

void _construct() {
	ogConHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	conHandle = CreateConsoleScreenBuffer(0x80000000U | 0x40000000U, 0, 0, 0x00000001, 0);
	_activeColor = BBLACK | FWHITE;

	SetConsoleActiveScreenBuffer(conHandle);
	SetConsoleActiveScreenBuffer(GetStdHandle(STD_INPUT_HANDLE));
	
	inHandle = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(inHandle, ENABLE_WINDOW_INPUT);
}

void _destruct() {
	SetConsoleActiveScreenBuffer(ogConHandle);
}

int readKey() {
	INPUT_RECORD irec;
	KEY_EVENT_RECORD krec;
	DWORD n;
	while (1) {
		ReadConsoleInput(inHandle, &irec, 1, &n);
		if (irec.EventType == KEY_EVENT && irec.Event.KeyEvent.bKeyDown) {
			krec = irec.Event.KeyEvent;
			return krec.uChar.AsciiChar;
		}
	}
	return -1;
}

void writeci(CHAR_INFO* fb, int length) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(conHandle, &csbi);
	SMALL_RECT srwin;
	srwin.Left = 0; srwin.Top = 0; srwin.Right = csbi.dwSize.X - 1;
	srwin.Bottom = csbi.dwSize.Y;
	COORD cd;
	cd.X = 0;
	cd.Y = 0;
	WriteConsoleOutput(conHandle, fb, csbi.dwSize, cd, &srwin);	
}

void writefcl(char* fb, char* cb, int length) {
	CHAR_INFO* framebuffer = (CHAR_INFO*)alloca(sizeof(CHAR_INFO) * length);
	
	wchar_t b;
	for (int i = 0; i < length; i++) {
		b = fb[i];
		b &= 0x00FF;
		framebuffer[i].Char.UnicodeChar = b;
		framebuffer[i].Attributes = cb[i];
	}
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(conHandle, &csbi);
	//SMALL_RECT srwin = { 0, 0, short(csbi.dwSize.X - 1), csbi.dwSize.Y };
	SMALL_RECT srwin;
	srwin.Left = 0; srwin.Top = 0; srwin.Right = csbi.dwSize.X - 1;
	srwin.Bottom = csbi.dwSize.Y;
	COORD cd;
	cd.X = 0;
	cd.Y = 0;
	WriteConsoleOutput(conHandle, framebuffer, csbi.dwSize, cd, &srwin);
}
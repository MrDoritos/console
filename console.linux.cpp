//Linux
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <console.h>
#include <sys/ioctl.h>
#include <string>

//Define constructor
console::constructor console::cons;
winsize console::w;
int console::_activeColor;

console::constructor::constructor() {
//0 4 2 3 1 5 6 7				   0            1 0b001         2 0b010     3   0b011   4 0b100     5   0b101     6 0b110       7 0b111
	//					   0 		4 0b100		3 0b011     2	0b010	1 0b001	    5	0b101     6 0b110	7 0b111
	//fprintf(stderr, "%i %i %i %i %i %i %i %i", COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_YELLOW, COLOR_RED, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE);
	initscr();
	start_color();
	auto map_color = [](int i) {
		switch (i) {
			case COLOR_BLUE: return COLOR_RED;
			case COLOR_RED: return COLOR_BLUE;
			default: return i;
		}
	};
	for (int i = 0; i < 64; i++) {
		init_pair(i + 1, map_color(i % 8), map_color((i / 8) % 8));
	}
	//for (int i = 0; i < 256; i++) {
	//	init_pair(i + 1, map_color(i % 16), map_color((i / 16) % 16));
	//}
	/*
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_WHITE, COLOR_BLUE);
	init_pair(3, COLOR_WHITE, COLOR_GREEN);
	init_pair(4, COLOR_WHITE, COLOR_YELLOW);
	init_pair(5, COLOR_WHITE, COLOR_RED);
	init_pair(6, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(7, COLOR_WHITE, COLOR_CYAN);
	init_pair(8, COLOR_WHITE, COLOR_WHITE);
	*/
	/*
	init_pair(1, FBLACK, BBLACK);
	init_pair(2, FBLUE, BBLACK);
	init_pair(3, FGREEN, BBLACK);
	init_pair(4, FYELLOW, BBLACK);
	init_pair(5, FRED, BBLACK);
	init_pair(6, FMAGENTA, BBLACK);
	init_pair(7, FCYAN, BBLACK);
	init_pair(8, FWHITE, BBLACK);
	init_pair(9, FBLACK, BBLUE);
	init_pair(10, FBLUE, BBLUE);
	init_pair(11, FGREEN, BBLUE);
	*/
	cbreak();
	noecho();
	console::_activeColor = -1;
}

console::constructor::~constructor() {
	endwin();
}

int console::getImage() {
	return IMAGE_LINUX;
}

int console::readKey() {
	return getch();
}

void console::sleep(int millis) {
	usleep(millis);
}

void console::_refreshSize() {
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
}

int console::getConsoleWidth() {
	console::_refreshSize();
	return int(w.ws_col);
}

int console::getConsoleHeight() {
	console::_refreshSize();
	return int(w.ws_row);
}

void console::setConsoleColor(int color) {
	const auto getCursesColor = [](int i) {
		short fg = i & 0b00000111, bg = i & 0b01110000;
		int c = 0;
		bg >>= 4;
		c = bg * 8;
		c += fg;
		return c + 1;
	};
	if (console::_activeColor == color)
		return;
	if (console::_activeColor != -1)
		//attroff(COLOR_PAIR(getCursesColor(color)));
		attroff(COLOR_PAIR((color & 0b00001111) + ((color & 0b11110000)) + 1));
	attron(COLOR_PAIR((color & 0b00001111) + ((color & 0b11110000)) + 1));
	attron(COLOR_PAIR(getCursesColor(color)));
	console::_activeColor = color;
}

void console::clear() {
	wclear(getwin(stdout));
	refresh();
}

void console::write(char* fb, char* cb, int length) {
	int i = 0;
	int w = console::getConsoleWidth();
	int h = console::getConsoleHeight();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			console::setConsoleColor(cb[i]);
			mvaddch(y, x, fb[i++]);
		}
	}
	refresh();
}

void console::write(int x, int y, char character) {
	mvaddch(y, x, character);
	
	//Call refresh ?
	refresh();
}

void console::write(int x, int y, char character, int color) {
	console::setConsoleColor(color);
	console::write(x, y, character);
}





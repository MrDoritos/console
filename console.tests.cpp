#include "console.h"
#include <iostream>
#include <string.h>

#ifdef __WIN32
int wmain() {
#else
int main() {
#endif
	console::clear();
	char buffer[256];
	memset(buffer, '\0', sizeof(buffer) / sizeof(typeof(buffer[0])));
	sprintf(&buffer[0], "Width: %i, Height: %i", console::getConsoleWidth(), console::getConsoleHeight());
	char cur = buffer[0];
	int i = 0;	
	int cwidth = console::getConsoleWidth(), cheight = console::getConsoleHeight();
	
	for (i = 0; i < 64; i++) {
		console::write(i % cwidth, (i / cwidth) % cheight, '#', (i % 8) | ((((i) / 8) % 8) << 4));
	}
	
	for (i = 0; i < 64; i++) {
		console::write(i % cwidth, ((i / cwidth) + 20) % cheight, '*', (i % 8) | (((i / 8) % 8) << 4));
	}

	for (i = 0; i < 8; i++) {
		console::write(i % cwidth, ((i / cwidth) + 30) % cheight, L'░', i << 4);
	}
	
	console::readKey();
	
	return 0;
}

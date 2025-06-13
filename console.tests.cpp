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

	int ypos = 1;
	for (i = 0; i < 256; i++) {
		console::write(i % cwidth, (i / cwidth + ypos) % cheight, '#', (i % 16) | ((((i) / 16) % 16) << 4));
	}
	ypos += (256 / cwidth) + 2;
	for (i = 0; i < 256; i++) {
		console::write(i % cwidth, (i / cwidth + ypos) % cheight, '*', (i % 16) | (((i / 16) % 16) << 4));
	}
	ypos += (256 / cwidth) + 2;
	for (i = 0; i < 64; i++) {
		console::write(i % cwidth, (i / cwidth + ypos) % cheight, L'░', i << 4);
	}
	
	console::readKey();
	
	return 0;
}

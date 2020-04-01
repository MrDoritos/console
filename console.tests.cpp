#include "console.h"
#include <iostream>
#include <string.h>

#if defined _WIN32
#include <Windows.h>
#define sleep(x) Sleep(x)
#elif defined __linux__
#include <unistd.h>
#define sleep(x) usleep(x)
#endif

int main() {
	console::clear();
	char buffer[256];
	memset(buffer, '\0', sizeof(buffer) / sizeof(typeof(buffer[0])));
	sprintf(&buffer[0], "Width: %i, Height: %i", console::getConsoleWidth(), console::getConsoleHeight());
	char cur = buffer[0];
	int i = 0;	
	//fprintf(stderr, "width: %i", console::getConsoleWidth());
	//fprintf(stderr, "height: %i", console::getConsoleHeight());
	
	int cwidth = console::getConsoleWidth(), cheight = console::getConsoleHeight();
	
	for (i = 0; i < 64; i++) {
		//fprintf(stderr, "[%i] fg: %i bg: %i", i + 1, i % 8, (((i) / 8) % 8));
		console::write(i % cwidth, (i / cwidth) % cheight, '#', (i % 8) | ((((i) / 8) % 8) << 4));
					 //x		   y							each fg   
	}
	
	for (i = 0; i < 64; i++) {
		console::write(i % cwidth, ((i / cwidth) + 20) % cheight, '*', (i % 8) | (((i / 8) % 8) << 4));
	}
	//sleep(-1);
	console::readKey();
}

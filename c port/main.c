#include "console.h"

CHAR_INFO* framebuffer;
int width, height;

int get(int x, int y) {
	return (y * width) + x;
}

int clip(int x, int y) {
	return (x >= 0 && x < width && y >= 0 && y < height);
}

void drawBox(int ox, int oy, int sizex, int sizey, char color) {
	for (int x = ox; x < ox + sizex; x++) {
		for (int y = oy; y < oy + sizey; y++) {
			framebuffer[get(x, y)].Attributes = color;
			framebuffer[get(x, y)].Char.UnicodeChar = L' ';
		}
	}
	for (int x = ox; x < ox + sizex - 1; x++) {
		framebuffer[get(x,oy)].Char.UnicodeChar = L'═';
		framebuffer[get(x,oy + sizey - 1)].Char.UnicodeChar = L'═'; 
	}
	for (int y = oy; y < oy + sizey - 1; y++) {
		framebuffer[get(ox, y)].Char.UnicodeChar = L'║';
		framebuffer[get(ox + sizex - 1, y)].Char.UnicodeChar = L'║';
	}
	framebuffer[get(ox,oy)].Char.UnicodeChar = L'╔';
	framebuffer[get(ox,oy + sizey - 1)].Char.UnicodeChar = L'╚';
	framebuffer[get(ox + sizex - 1, oy)].Char.UnicodeChar = L'╗';
	framebuffer[get(ox + sizex - 1, oy + sizey - 1)].Char.UnicodeChar = L'╝';
}

int wmain() {
	_construct();
	cclear();
	width = getConsoleWidth();
	height = getConsoleHeight();
	framebuffer = (CHAR_INFO*)malloc(width * height * sizeof(CHAR_INFO));
    memset(framebuffer, 0, width * height * sizeof(CHAR_INFO));
	
	drawBox(5,5,30,30,FBLUE | BWHITE | 0b00000000);
	
	drawBox(20,20,30,30,FRED | BWHITE | 0b00000000);
	
	drawBox(35,35,30,30,FGREEN | BWHITE | 0b00000000);
	
	drawBox(50,50,45,30,FCYAN | BWHITE | 0b00001000);
	
	writeci(framebuffer, width * height);
	while (readKey() != 'q');
	_destruct();
	return 0;
}
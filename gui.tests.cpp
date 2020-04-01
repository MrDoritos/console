#include <iostream>
#include "console.h"
#include "eventHandler.h"
#include "screen.h"
#include "element.h"
#include "image.h"

using namespace std;



class box : public element<box> {
	public:
	box(screen& scr)
	:element<box>(scr, 10,4,console::getConsoleWidth()-20,console::getConsoleHeight()-8) {
		events.setReferer(this);
		events.assign(FLG_FRAME, func(box, box::onFrame));
		events.assign(FLG_KEYPRESS, func(box, box::onKeypress));
	}
	
	void onKeypress() {
		exit(_framebuffer._screen->key);
		_framebuffer.write(0,0, _framebuffer._screen->key, FBLUE | BBLACK);
	}
	
	void onFrame() {
			return;
		int i = 0;
		for (int y = 0; y < _framebuffer.getY(); y++)
			for (int x = 0; x < _framebuffer.getX(); x++, i++)
				_framebuffer.write(x, y, '#', (i % 128) + ((i & 0x01) << 4) + 1);				
			
		//_framebuffer.write(0, 0, 'y', FYELLOW | BBLACK);
		//_framebuffer.write(1, 0, 'Y', 0b00001011 | BBLACK);
		
		_framebuffer.write(0, 0, 'i', FMAGENTA | BWHITE);
		_framebuffer.write(1, 0, 'a', FRED | BWHITE);
		_framebuffer.write(2, 0, 'n', FGREEN | BWHITE);
	}
};

int main(int argc, char** argv) {
	screen mainscr;
		
	//puts("debugging in 10 sec");
	//sleep(10000);
	
		
	box first(mainscr);
	
	//png spng(argv[argc - 1]);
	//image first(mainscr, spng, 0, 0, console::getConsoleWidth(), console::getConsoleHeight());
	mainscr.add(&first.events);
	
	first._framebuffer.direct(true);
	//SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));

	mainscr.events.handle(FLG_CREATE);
	
	//fprintf(stderr, "%ix%i\n", spng.getSizeX(), spng.getSizeY());
	
	//mainscr.fElapsedTime = 50.0f;	
		
	while (1) {	
		mainscr.events.handle(FLG_UPDATE);
	
		mainscr.events.handle(FLG_FRAME);
		//sleep(100);
		//if (console::getConsoleWidth() != mainscr._sizex || console::getConsoleHeight() != mainscr._sizey) {
		//	mainscr.events.handle(FLG_RESIZE);
		//}
	}
	sleep(-1);
}

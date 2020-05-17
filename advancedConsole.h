#pragma once
#include "console.h"

#include <thread>
#include <mutex>
#include <cmath>
#include <algorithm>

#include <string.h>

#ifdef __linux__
#include <curses.h>
#endif

#define DRAWINGMODE_BASIC 0
#define DRAWINGMODE_COMPARE 1
#define DRAWINGMODE_AUTO 2
#define DRAWINGMODE_COMPLEX 3

class adv {
	public:
	friend class _constructor;
	
	struct _constructor {
		_constructor() {
			_advancedConsoleConstruct();
		}
		~_constructor() {
			_advancedConsoleDestruct();
		}
	};
	
	static _constructor construct;
	
	static void error(char* err) {
		//console::_destruct();
		_advancedConsoleDestruct();
		while (console::ready) 
			console::sleep(50); //wait for destruct
		printf("Error: %s\r\n", err);
		exit(1);
	}
	
	static void _advancedConsoleConstruct() {
		run = true;
		ready = false;
		modify = true;
		drawingMode = 0;
		doubleSize = false;
		uiloop = std::thread(loop);
	}
	static void _advancedConsoleDestruct() {	
		ready = false;
		run = false;	
		console::constructor::~constructor();
	}
	
	static void setDoubleWidth(bool w) {
		if (w) {
			width = floor(console::getConsoleWidth() / 2.0f);
			height = console::getConsoleHeight();
			if (!allocate(width, height)) {
				error("Could not allocate new buffers for double width mode");
			}
			doubleSize = true;
			clear();
		} else {
			if (!allocate()) {
				error("Could not allocate new buffers for single width mode");
			}
			doubleSize = false;
			clear();
		}
	}
	
	static bool allocate(int width, int height) {
		fb = new wchar_t[width * height];
		cb = new char[width * height];
		if (drawingMode == DRAWINGMODE_COMPARE) {
			oldfb = new wchar_t[width * height];
			oldcb = new char[width * height];
			if (!oldfb || !oldcb)
				return false;
		}
		if (!fb || !cb)
			return false;
		clear();
		return true;
	}
	
	static bool allocate() {
		width = console::getConsoleWidth(), height = console::getConsoleHeight();
		return allocate(width, height);
		/*
		fb = new wchar_t[width * height];
		cb = new char[width * height];
		clear();
		return (fb != nullptr && cb != nullptr);
		*/
	}
		
	static void loop() {		
		while (!console::ready) 
			console::sleep(50);
		
		if (!allocate())
			error("Could not allocate a framebuffer or color buffer");
		
		int c;
		
		ready = true;
		
		while (run) {
			//Other threads can read keys, not this one
			
			//Operate on the buffers
			{
				//std::lock_guard<std::mutex> lk(buffers); //obtain lock on the buffers
				//if (!modify)
				//	goto condition;
				//console::write(fb, cb, width * height);
				draw();
				//modify = false;
			}
			
			//condition:;
			
			while (!modify)
				//console::sleep(33); //~30 fps
				console::sleep(16);
				//std::this_thread::yield();
				//console::sleep(1);
		}
	}
	
	static void setDrawingMode(int mode) {
		std::lock_guard<std::mutex> lk(buffers);
		switch (mode) {
			case DRAWINGMODE_BASIC: {
				drawingMode = DRAWINGMODE_BASIC;
				break;
				}
			case DRAWINGMODE_COMPARE: {
				oldfb = new wchar_t[width * height];
				oldcb = new char[width * height];
				if (!oldfb || !oldcb)
					error("Could not allocate a buffer for DRAWINGMODE_COMPARE");
				drawingMode = DRAWINGMODE_COMPARE;
				break;
				}
			default: {
				break;
				}
		}
	}
	
	static void drawCompare() {
		//Since we refresh on every console::write I think we should have an automatic switch for linux
		#ifdef __linux__
		console::useRefresh = false;
		#endif
		for (int x = 0; x < width; x++)
			for (int y = 0; y < height; y++)
				if (cb[get(x,y)] != oldcb[get(x,y)] || fb[get(x,y)] != oldfb[get(x,y)]) {
					if (doubleSize) {
						console::write(x * 2, y, fb[get(x,y)],cb[get(x,y)]);
						console::write(x * 2 + 1, y, fb[get(x,y)],cb[get(x,y)]);
					} else {
						console::write(x,y,fb[get(x,y)],cb[get(x,y)]);
					}
				}
		#ifdef __linux__
		console::useRefresh = true;
		refresh();
		#endif
	}
	
	static void draw() {
		std::lock_guard<std::mutex> lk(buffers);
		if (!modify)
			return;
		
		if (drawingMode == DRAWINGMODE_BASIC) {
			if (doubleSize) {
				wchar_t buffer[(width * 2) * height];
				char cbuffer[(width * 2) * height];
				for (int x = 0; x < width; x++) {
					for (int y = 0; y < height; y++) {
						 buffer[(y * width * 2) + (x * 2)]     = fb[get(x,y)];
						 buffer[(y * width * 2) + (x * 2 + 1)] = fb[get(x,y)];
						cbuffer[(y * width * 2) + (x * 2)]     = cb[get(x,y)];
						cbuffer[(y * width * 2) + (x * 2 + 1)] = cb[get(x,y)];
					}
				}
				console::write(&buffer[0], &cbuffer[0], (width * 2) * height);
			} else {
				console::write(fb, cb, width * height);
			}
		}
		
		if (drawingMode == DRAWINGMODE_COMPARE)
			//drawCompare();
			error("");
		
		modify = false;
	}
	
	static void write(int x, int y, wchar_t character) {
		if (!bound(x, y))
			return;
		{
			std::lock_guard<std::mutex> lk(buffers);
			fb[get(x, y)] = character;
			cb[get(x, y)] = FWHITE | BBLACK;
			modify = true;
		}
	}
	
	static void write(int x, int y) {
		write(x, y, L'#');
	}
	
	static void write(int x, int y, const char* string) {
		adv::write(x, y, string, FWHITE | BBLACK);
	}
	
	static void write(int x, int y, const char* string, char color) {
		std::lock_guard<std::mutex> lk(buffers);
		int length = strlen(string);
		for (int i = 0; i < length; i++) {
			if (bound(x + i, y)) {
				fb[get(x + i, y)] = string[i];
				cb[get(x + i, y)] = color;
				modify = true;
			}
		}
	}
	
	static void write(int x, int y, wchar_t character, char color) {
		if (!bound(x, y))
			return;
		
		{
			std::lock_guard<std::mutex> lk(buffers);
			fb[get(x, y)] = character;
			cb[get(x, y)] = color;			
			modify = true;
		}
	}
	
	static void clear(wchar_t character = L' ', char color = FWHITE | BBLACK) {
		std::lock_guard<std::mutex> lk(buffers);
		//memset(fb, L' ', width * height * sizeof(wchar_t));
		for (int i = 0; i < width * height; i++) {
			fb[i] = character;
			cb[i] = color;
		}
		//memset(cb, color, width * height * sizeof(char));
		modify = true;
	}
	
	/* Drawing functions */
	
	static void line(int x0, int y0, int x1, int y1) {
		line(x0,y0,x1,y1,'#',FWHITE|BBLACK);
	}
		
	static void line(int x1, int y1, int x2, int y2, wchar_t character, char color) {		
		std::lock_guard<std::mutex> lk(buffers);
		int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;
		
		dx=x2-x1;
		dy=y2-y1;
		
		dx1=fabs(dx);
		dy1=fabs(dy);
		
		px=2*dy1-dx1;
		py=2*dx1-dy1;

		if(dy1<=dx1) {
			if(dx>=0) {
				x=x1;
				y=y1;
				xe=x2;
			} else {
				x=x2;
				y=y2;
				xe=x1;
			}
			//write(x, y, character, color);
			if (bound(x,y)) {
				fb[get(x,y)] = character;
				cb[get(x,y)] = color;
			}
			
			for(i=0;x<xe;i++) {
				x=x+1;
				if(px<0) {
					px=px+2*dy1;
				} else {
					if((dx<0 && dy<0) || (dx>0 && dy>0)) {
						y=y+1;
					} else {
						y=y-1;
					}
					px=px+2*(dy1-dx1);
				}
				
				//write(x, y, character, color);
				if (bound(x,y)) {
					fb[get(x,y)] = character;
					cb[get(x,y)] = color;
				}
			}
		} else {
			if(dy>=0) {				
				x=x1;
				y=y1;
				ye=y2;
			} else {
				x=x2;
				y=y2;
				ye=y1;
			}  
			
			//write(x, y, character, color);
			if (bound(x,y)) {
				fb[get(x,y)] = character;
				cb[get(x,y)] = color;
			}
  
			for(i=0;y<ye;i++) {
				y=y+1;
				if(py<=0) {
					py=py+2*dx1;
				} else {
					if((dx<0 && dy<0) || (dx>0 && dy>0)) {
						x=x+1;
					} else {
						x=x-1;
					}
					
					py=py+2*(dx1-dy1);
				}	
				
				//write(x, y, character, color);
				if (bound(x,y)) {
					fb[get(x,y)] = character;
					cb[get(x,y)] = color;
				}
			}
		}
		
		modify = true;
	}
	
	static void triangle(int x0, int y0, int x1, int y1, int x2, int y2, wchar_t character, char color) {		
		std::lock_guard<std::mutex> lk(buffers);
		
		if (y0 > y1) {
			std::swap(y0, y1);
			std::swap(x0, x1);
		}
		if (y0 > y2) {
			std::swap(y0, y2);
			std::swap(x0, x2);
		}
		if (y1 > y2) {
			std::swap(y1, y2);
			std::swap(x1, x2);
		}
		
		int total_height = y2 - y0;
		
		for (int y = y0; y <= y1; y++) {
			int segment_height = y1-y0+1; 
			float alpha = (float)(y-y0)/total_height; 
			float beta  = (float)(y-y0)/segment_height; // be careful with divisions by zero 
			//Vec2i A = t0 + (t2-t0)*alpha;
			int Ay = y0 + (y2 - y0) * alpha;
			int Ax = x0 + (x2 - x0) * alpha;
			//Vec2i B = t0 + (t1-t0)*beta; 
			int By = y0 + (y1 - y0) * beta;
			int Bx = x0 + (x1 - x0) * beta;
			
			if (Ax>Bx) { std::swap(Ax, Bx); std::swap(Ay, By); }
			
			for (int j=Ax; j<=Bx; j++) { 
				//image.set(j, y, color); // attention, due to int casts t0.y+i != A.y 
				if (bound(j, y)) {
					fb[get(j, y)] = character;
					cb[get(j, y)] = color;
				}
			} 			
		}
				
		for (int y=y1; y<=y2; y++) { 
			int segment_height =  y2-y1+1; 
			float alpha = (float)(y-y0)/total_height; 
			float beta  = (float)(y-y1)/segment_height; // be careful with divisions by zero 			
			int Ay = y0 + (y2 - y0) * alpha;
			int Ax = x0 + (x2 - x0) * alpha;			
			int By = y1 + (y2 - y1) * beta;
			int Bx = x1 + (x2 - x1) * beta;
			
			if (Ax>Bx) { std::swap(Ax, Bx); std::swap(Ay, By); }
						
			for (int j=Ax; j<=Bx; j++) { 
				if (bound(j, y)) {
					fb[get(j, y)] = character;
					cb[get(j, y)] = color;
				}
			} 
		} 
		
		modify = true;
		/*
		line(x0, y0, x1, y1, character, color);
		line(x1, y1, x2, y2, character, color);
		line(x2, y2, x0, y0, character, color);
		*/
	}
	
	static void triangle(int x0, int y0, int x1, int y1, int x2, int y2) {
		triangle(x0, y0, x1, y1, x2, y2, L'#', FWHITE | BBLACK);
	}
	
	static void legacyCircle(int x, int y, int radius, wchar_t character, char color) {
		std::lock_guard<std::mutex> lk(buffers);
		const double double_Pi = 6.28318530d;
		double step = 1 / (radius * double_Pi);
		int xF, yF;
		for (double theta = 0.0d; theta < double_Pi; theta += step) {
			xF = (x + radius * cos(theta));
			yF = (y - radius * sin(theta));
			if (bound(xF, yF)) {
				fb[get(xF, yF)] = character;
				cb[get(xF, yF)] = color;
			}
		}		
		modify = true;
	}
	
	static void circle(int x, int y, int radius) {
		circle(x, y, radius, '#', FWHITE | BBLACK);
	}
	
	static void circle(int x0, int y0, int radius, wchar_t character, char color) {
		std::lock_guard<std::mutex> lk(buffers);
		auto drawCircle = [&](int xc, int yc, int x, int y) {			
			if (bound(xc+x, yc+y)) {
				fb[get(xc+x, yc+y)] = character;
				cb[get(xc+x, yc+y)] = color;
			}
			if (bound(xc-x, yc+y)) {
				fb[get(xc-x, yc+y)] = character;
				cb[get(xc-x, yc+y)] = color;
			}
			if (bound(xc+x, yc-y)) {
				fb[get(xc+x, yc-y)] = character;
				cb[get(xc+x, yc-y)] = color;
			}
			if (bound(xc-x, yc-y)) {
				fb[get(xc-x, yc-y)] = character;
				cb[get(xc-x, yc-y)] = color;
			}
			if (bound(xc+y, yc+x)) {
				fb[get(xc+y, yc+x)] = character;
				cb[get(xc+y, yc+x)] = color;
			}
			if (bound(xc-y, yc+x)) {
				fb[get(xc-y, yc+x)] = character;
				cb[get(xc-y, yc+x)] = color;
			}
			if (bound(xc+y, yc-x)) {
				fb[get(xc+y, yc-x)] = character;
				cb[get(xc+y, yc-x)] = color;
			}	
			if (bound(xc-y, yc-x)) {
				fb[get(xc-y, yc-x)] = character;
				cb[get(xc-y, yc-x)] = color;
			}
		};
		int x = 0, y = radius; 
		int d = 3 - 2 * radius; 
		drawCircle(x0, y0, x, y); 
		while (y >= x) 
		{ 
			x++; 	
			if (d > 0) 
			{ 
				y--;  
				d = d + 4 * (x - y) + 10; 
			} 
			else
				d = d + 4 * x + 6; 
			drawCircle(x0, y0, x, y); 			
		}
		
		modify = true;
	}
	
	static int get(int x, int y) {
		return y * width + x;
	}
	
	static bool bound(int x, int y) {
		return (x > -1 && y > -1 && x < width && y < height);
	}
	
	static void clip(int& x, int& y) {
		x = x > width - 1 ? width - 1 : x;
		x = x < 0 ? 0 : x;
		y = y > height - 1 ? height - 1 : y;
		y = y < 0 ? 0 : y;
	}
	
	static int getOffsetX(float scale, float length) {
		if (length < 1) {
			return (float(width) * scale);
		}
		if (length > width)
			return 0.0f;
		
		return (float(width) * scale) - (length / 2);
	}
	
	static int getOffsetY(float scale, float length) {
		if (length < 1) {
			return (float(height) * scale);
		}
		if (length > height)
			return 0.0f;
		
		return (float(height) * scale) - (length / 2);
	}
	
	struct key {
		bool pressed;
		bool released;
		bool held;		
	} keys[256];
	
	static std::mutex buffers;
	
	static std::thread uiloop;
	static bool run;
	static bool ready;
	static bool modify;
	
	static int width;
	static int height;
	
	static wchar_t* fb;
	static wchar_t* oldfb;
	static char* cb;
	static char* oldcb;
	
	private:
	static int drawingMode;
	static bool doubleSize;
};

adv::_constructor adv::construct;
std::thread adv::uiloop;
std::mutex adv::buffers;
bool adv::run;
bool adv::ready;
bool adv::modify;
int adv::width;
int adv::height;
wchar_t* adv::fb;
wchar_t* adv::oldfb;
char* adv::cb;
char* adv::oldcb;
int adv::drawingMode;
bool adv::doubleSize;

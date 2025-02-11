#pragma once

#ifndef __ADVANCEDCONSOLE
#define __ADVANCEDCONSOLE

#include "console.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <cmath>
#include <algorithm>
#include <chrono>

#include <string.h>

#ifdef __linux__
//#include <curses.h>
#include <ncursesw/ncurses.h>
#endif

#define DRAWINGMODE_BASIC 0
#define DRAWINGMODE_COMPARE 1
#define DRAWINGMODE_AUTO 2
#define DRAWINGMODE_COMPLEX 3

#define BORDER_LINE 2
#define BORDER_DOUBLE 3
#define BORDER_SIMPLE 0
#define BORDER_BLOCK 1
#define BORDER_DITHER 4

//Disable curses garbage
#undef border

class adv {
	public:
	friend class _constructor;
	
	struct _constructor {
		_constructor();
		~_constructor();
	};
	
	static _constructor construct;
	
	static void error(char* err);
	static void error(const char* err);
	
	static void _advancedConsoleConstruct() {
		run = true;
		ready = false;
		modify = true;
		drawingMode = 0;
		doubleSize = false;
		thread = true;
		lastFrame = std::chrono::high_resolution_clock::time_point();
		setFPS(60);
		ascii = false;
		disableThreadSafety = false;

		fb = nullptr;
		cb = nullptr;
		oldfb = nullptr;
		oldcb = nullptr;

		if (!allocate())
			error("Could not allocate a framebuffer or color buffer");
		
		uiloop = std::thread(loop);

		while (!ready && thread && run && uiloop.joinable() && console::ready) {
			console::sleep(1);
		}
	}
	static void _advancedConsoleDestruct() {
		if (!ready)
			return;

		ready = false;
		run = false;
		cvThreadState.notify_all();

		if (uiloop.joinable() || thread) {
			setThreadSafety(true);
			setThreadState(true);
			console::sleep(1);
			cvThreadState.notify_all();
			console::sleep(1);
			if (thread && uiloop.joinable()) {
				uiloop.join();
			}
		}
		
		//console::cons.~constructor();

		cvThreadState.notify_all();
		deallocate();
	}
	
	static void setDoubleWidth(bool w) {
		width = bufferWidth = console::getConsoleWidth();
		height = bufferHeight = console::getConsoleHeight();

		if (w)
			width = bufferWidth / 2;

		if (!allocate(width, height))
			error("Could not reallocate buffers");

		doubleSize = w;
		clear();
	}
	
	static bool isNewSize() {
		int consoleWidth = console::getConsoleWidth();
		int consoleHeight = console::getConsoleHeight();

		if (consoleWidth != bufferWidth || consoleHeight != bufferHeight) {
			setDoubleWidth(doubleSize);
			return true;
		}

		return false;
	}

	static void setThreadSafety(bool state) {
		disableThreadSafety = !state;
	}
	
	static void setThreadState(bool state) {
		if (thread && !adv::ready) {
			std::lock_guard<std::mutex> lk(startLock);
		}
		
		thread = state; //Just pauses/unpauses the drawing thread
		cvThreadState.notify_all();
	}

	static void deallocate() {
		if (fb)
			delete [] fb;
		if (cb)
			delete [] cb;
		if (oldfb)
			delete [] oldfb;
		if (oldcb)
			delete [] oldcb;

		fb = nullptr;
		cb = nullptr;
		oldfb = nullptr;
		oldcb = nullptr;
	}
	
	static bool allocate(int width, int height) {
		deallocate();

		int length = width * height;

		fb = new wchar_t[length];
		cb = new color_t[length];

		if (!fb || !cb)
			return false;

		if (drawingMode == DRAWINGMODE_COMPARE) {
			oldfb = new wchar_t[length];
			oldcb = new color_t[length];

			if (!oldfb || !oldcb)
				return false;
		}

		clear();

		return true;
	}
	
	static bool allocate() {
		bufferWidth = width = console::getConsoleWidth();
		bufferHeight = height = console::getConsoleHeight();
		return allocate(width, height);
	}

	static void setFPS(float fps) {
		setFrametime(1000.0f / fps);
	}

	static void setFrametime(float ft) {
		frametime = ft;
	}
		
	static void loop() {		
		{
			std::lock_guard<std::mutex> lk(startLock);
			
			while (!console::ready) 
				console::sleep(50);
			
			int c;
			
			ready = true;
		}

		cvStart.notify_all();
		
		thread = true;
		
		while (run) {
			draw();
			
			waitForReadyFrame();

			waitForThreadState();
		}
		
		thread = false;
	}
	
	static void waitForReadyFrame() {
		std::chrono::duration<double, std::micro> ft(frametime * 1000.0f - 80.0f);
		std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::micro> diff = now - lastFrame;
		deltaTime = diff.count() / 1000.0f;
		if (diff < ft) {
			//std::this_thread::sleep_for(ft - diff);
			std::this_thread::sleep_until(lastFrame + ft);
		}
		lastFrame = std::chrono::high_resolution_clock::now();
	}

	static void waitForThreadState() {
		if (thread)
			return; //No wait
		while (!thread && run) { //Wait for the drawing thread to be started again
			std::unique_lock<std::mutex> lk(threadStateMux);
			cvThreadState.wait(lk);
		}
	}

	static void setAscii(bool bAscii) {
		ascii = bAscii;
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
				oldcb = new color_t[width * height];
				if (!oldfb || !oldcb || !cb || !fb)
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
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				if (cb[get(x,y)] != oldcb[get(x,y)] || fb[get(x,y)] != oldfb[get(x,y)]) {
					if (doubleSize) {
						console::write(x * 2, y, fb[get(x,y)],cb[get(x,y)]);
						console::write(x * 2 + 1, y, fb[get(x,y)],cb[get(x,y)]);
					} else {
						console::write(x,y,fb[get(x,y)],cb[get(x,y)]);
					}
					oldfb[get(x,y)] = fb[get(x,y)];
					oldcb[get(x,y)] = cb[get(x,y)];
				}
			}
		}
		#ifdef __linux__
		console::useRefresh = true;
		refresh();
		#endif
	}
	
	static void draw() {
		{
		std::lock_guard<std::mutex> lk(buffers);
		if (!modify)
			return;
		
		wchar_t *buffer = fb;
		color_t *cbuffer = cb;
		int length = width * height;

		if (drawingMode == DRAWINGMODE_BASIC) {
			if (doubleSize) {
				length = bufferWidth * bufferHeight;
				buffer = (wchar_t*)alloca(length * sizeof(wchar_t));
				cbuffer = (color_t*)alloca(length * sizeof(color_t));
				
				for (int x = width * 2; x < bufferWidth; x++) {
					for (int y = 0; y < bufferHeight; y++) {
						buffer[y * bufferWidth + x] = L' ';
					   cbuffer[y * bufferWidth + x] = FWHITE|BBLACK;
					}
				}

				for (int x = 0; x < width; x++) {
					for (int y = 0; y < height; y++) {
						int db_offset = y * bufferWidth + x * 2;
						int fb_offset = get(x,y);
						buffer[db_offset]     = fb[fb_offset];
						buffer[db_offset + 1] = fb[fb_offset];
					   cbuffer[db_offset]     = cb[fb_offset];
					   cbuffer[db_offset + 1] = cb[fb_offset];
					}
				}
			}

			if (ascii)
				writeAscii(buffer, cbuffer, length);
			else
				console::write(buffer, cbuffer, length);
		}
		
		if (drawingMode == DRAWINGMODE_COMPARE)
			drawCompare();
		
		modify = false;
		}
	}

	static void writeAscii(wchar_t *framebuffer, color_t *colorbuffer, int length) {
		char *asciiBuffer = (char*)alloca(length);
		for (int i = 0; i < length; i++)
			asciiBuffer[i] = (char)framebuffer[i];
		console::write(&asciiBuffer[0], colorbuffer, length);
	}
	
	static void write(wchar_t* framebuffer, color_t* colorbuffer, int width, int height) {
		std::lock_guard<std::mutex> lk(buffers);
		for (int x = 0; x < width && x < adv::width; x++) {
			for (int y = 0; y < height && y < adv::height; y++) {
				fb[get(x,y)] = framebuffer[y * width + x];
				cb[get(x,y)] = colorbuffer[y * width + x];
			}			
		}
		modify = true;
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
	
	//Normal write
	static void norm(float posx, float posy, const char* string, color_t color) {
		int l = strlen(string);
		int ox = adv::getOffsetX(posx, l);
		int oy = adv::getOffsetY(posy, 1);
		adv::write(ox, oy, string, color);
	}
	
	static void norm(float posx, float posy, const char* string) {
		norm(posx, posy, string, FWHITE | BBLACK);
	}
	
	static void write(int x, int y, const char* string, color_t color) {
		std::lock_guard<std::mutex> lk(buffers);
		int length = strlen(string);
		for (int i = 0; i < length; i++) {
			if (bound(x + i, y)) {
				fb[get(x + i, y)] = string[i];
				cb[get(x + i, y)] = color;
			}
		}
		modify = true;
	}
	
	static void write(int x, int y, const char* string, const color_t* colorbuffer) {
		std::lock_guard<std::mutex> lk(buffers);
		int length = strlen(string);
		for (int i = 0; i < length; i++) {
			if (bound(x + i, y)) {
				fb[get(x + i, y)] = string[i];
				cb[get(x + i, y)] = colorbuffer[i];
			}
		}
		
		modify = true;
	}
	
	static void write(int x, int y, wchar_t character, color_t color) {
		if (!bound(x, y))
			return;
		
		if (disableThreadSafety) {
			fb[get(x, y)] = character;
			cb[get(x,y)] = color;
			modify = true;
		} else {
			std::lock_guard<std::mutex> lk(buffers);
			fb[get(x, y)] = character;
			cb[get(x, y)] = color;			
			modify = true;
		}
	}
	
	static void clear(wchar_t character = L' ', color_t color = FWHITE | BBLACK) {
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
	
	static void rectangle(int x0, int y0, int x1, int y1) {
		rectangle(x0, y0, x1, y1, L'#', FWHITE|BBLACK);
	}
	
	static void rectangle(int x0, int y0, int x1, int y1, wchar_t character, color_t color) {
		line(x0,y0,x1,y0,character,color);
		line(x1,y0,x1,y1,character,color);
		line(x0,y1,x1,y1,character,color);
		line(x0,y0,x0,y1,character,color);
	}
	
	static void border(int x0, int y0, int x1, int y1, color_t color) {
		line(x0,y0,x1,y0,'-', color);
		line(x1,y0,x1,y1,'|', color);
		line(x0,y1,x1,y1,'-', color);
		line(x0,y0,x0,y1,'|', color);
		write(x0,y0,'+', color);
		write(x1,y0,'+', color);
		write(x0,y1,'+', color);
		write(x1,y1,'+', color);
	}
	
	static void fancyBorder(int x0, int y0, int x1, int y1, int type, color_t color) {
		switch (type) {
			case BORDER_BLOCK: {
					line(x0,y0,x1,y0,L'█', color);
					line(x1,y0,x1,y1,L'█', color);
					line(x0,y1,x1,y1,L'█', color);
					line(x0,y0,x0,y1,L'█', color);
				break;
			}
			case BORDER_DITHER: {
					line(x0,y0,x1,y0,L'▒', color);
					line(x1,y0,x1,y1,L'▒', color);
					line(x0,y1,x1,y1,L'▒', color);
					line(x0,y0,x0,y1,L'▒', color);					
					write(x0,y0,L'▓', color);
					write(x1,y0,L'▓', color);
					write(x0,y1,L'▓', color);
					write(x1,y1,L'▓', color);
				break;
			}
			case BORDER_DOUBLE: {
					line(x0,y0,x1,y0,L'═', color);
					line(x1,y0,x1,y1,L'║', color);
					line(x0,y1,x1,y1,L'═', color);
					line(x0,y0,x0,y1,L'║', color);
					write(x0,y0,L'╔', color);
					write(x1,y0,L'╗', color);
					write(x0,y1,L'╚', color);
					write(x1,y1,L'╝', color);		
				break;
			}
			case BORDER_LINE: {
					line(x0,y0,x1,y0,L'─', color);
					line(x1,y0,x1,y1,L'│', color);
					line(x0,y1,x1,y1,L'─', color);
					line(x0,y0,x0,y1,L'│', color);
					write(x0,y0,L'┌', color);
					write(x1,y0,L'┐', color);
					write(x0,y1,L'└', color);
					write(x1,y1,L'┘', color);
				break;
			}
			case BORDER_SIMPLE: {
					border(x0,y0,x1,y1,color);
				break;
			}
		}
	}
	
	static void fill(int x0, int y0, int x1, int y1) {
		fill(x0, y0, x1, y1, L'#', FWHITE|BBLACK);
	}
	
	static void fill(int x0, int y0, int x1, int y1, wchar_t character, color_t color) {
		std::lock_guard<std::mutex> lk(buffers);
		for (int x = x0; x < x1; x++) {
			for (int y = y0; y < y1; y++) {
				if (!bound(x,y))
					continue;
				
				fb[get(x,y)] = character;
				cb[get(x,y)] = color;
			}
		}
		
		modify = true;
	}
	
	static void line(int x0, int y0, int x1, int y1) {
		line(x0,y0,x1,y1,'#',FWHITE|BBLACK);
	}
		
	static void line(int x1, int y1, int x2, int y2, wchar_t character, color_t color) {		
		//if (!bound(x1, y1) ||
		//	!bound(x2, y2))
		//	return;
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
	
	static void triangle(int x0, int y0, int x1, int y1, int x2, int y2, wchar_t character, color_t color) {
		if (!bound(x0, y0) ||
			!bound(x1, y1) ||
			!bound(x2, y2))
			return;
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
			
			for (unsigned int j=Ax; j<=Bx; j++) { 
				//image.set(j, y, color); // attention, due to int casts t0.y+i != A.y 
				if (bound(j, y)) {
					fb[get(j, y)] = character;
					cb[get(j, y)] = color;
				}
			} 			
		}
				
		for (unsigned int y=y1; y<=y2; y++) { 
			int segment_height =  y2-y1+1; 
			float alpha = (float)(y-y0)/total_height; 
			float beta  = (float)(y-y1)/segment_height; // be careful with divisions by zero 			
			int Ay = y0 + (y2 - y0) * alpha;
			int Ax = x0 + (x2 - x0) * alpha;			
			int By = y1 + (y2 - y1) * beta;
			int Bx = x1 + (x2 - x1) * beta;
			
			if (Ax>Bx) { std::swap(Ax, Bx); std::swap(Ay, By); }
						
			for (unsigned int j=Ax; j<=Bx; j++) { 
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
	
	static void legacyCircle(int x, int y, int radius, wchar_t character, color_t color) {
		std::lock_guard<std::mutex> lk(buffers);
		const double double_Pi = 6.28318530;
		double step = 1 / (radius * double_Pi);
		int xF, yF;
		for (double theta = 0.0; theta < double_Pi; theta += step) {
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
	
	static void circle(int x0, int y0, int radius, wchar_t character, color_t color) {
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
	
	static int getOffsetX(float scale) {
		return getOffsetX(scale, 0);
	}
	
	static int getOffsetY(float scale, float length) {
		if (length < 1) {
			return (float(height) * scale);
		}
		if (length > height)
			return 0.0f;
		
		return (float(height) * scale) - (length / 2);
	}
	
	static int getOffsetY(float scale) {
		return getOffsetX(scale, 0);
	}
		
	static std::mutex buffers;
	static std::condition_variable cvStart;
	static std::condition_variable cvThreadState;
	static std::mutex startLock;
	static std::mutex threadStateMux;
	
	static std::thread uiloop;
	static bool run;
	static bool ready;
	static bool modify;
	static bool thread;
	
	static int width;
	static int height;
	static int bufferWidth;
	static int bufferHeight;

	static float frametime;
	static std::chrono::time_point<std::chrono::high_resolution_clock> lastFrame;
	static float deltaTime;
	
	static wchar_t* fb;
	static wchar_t* oldfb;
	static color_t* cb;
	static color_t* oldcb;
	
	static int drawingMode;
	static bool doubleSize;
	static bool disableThreadSafety;
	static bool ascii;
};

#endif

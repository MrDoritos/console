#pragma once
#include <vector>
#include <string.h>
#include "eventHandler.h"
#include "console.h"
#include <ctime>
#include <chrono>

//#include "element.h"

#if defined __linux__
#include <unistd.h>
#define sleep(x) usleep(x)
#elif defined __WIN32
#include <Windows.h>
#define sleep(x) Sleep(x)
#endif

//class element;

class screen {
	public:
	screen() {
		resize(console::getConsoleWidth(), console::getConsoleHeight());
		events.setReferer(this);
		events.assign(FLG_CREATE, func(screen, screen::onCreate));
		events.assign(FLG_FRAME, func(screen, screen::onFrame));
		events.assign(FLG_RESIZE, func(screen, screen::onResize));
		events.assign(FLG_UPDATE, func(screen, screen::onUpdate));
		fElapsedTime = 50.0f;
		tickTime = 1000.0f/30.0f;
	}
	
	void recursiveUpdate(int flag, vector<element*>& _children) {
		for (auto* l : _children) {
			l->handle(flag);
			recursiveUpdate(flag, l->_children);
		}
	}
	
	void onUpdate() {
		int key = console::readKey();
		if (key != NO_KEY) {
			this->key = key;
			recursiveUpdate(FLG_KEYPRESS, _children);
		}
		
		recursiveUpdate(FLG_UPDATE, _children);
	}
	
	void onResize() {
		recursiveUpdate(FLG_RESIZE, _children);
		
		resize(_sizex, _sizey);
	}
	
	void onCreate() {
		recursiveUpdate(FLG_CREATE, _children);
	}
	
	void clear() {
		memset(_framebuffer, ' ', getSize());
		memset(_color, BBLACK | FWHITE, getSize());		
	}
	
	void onFrame() {
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		fElapsedTime = elapsedTime.count();
				
		sprintf(_framebuffer, "%lf fps, frametime: %lf, target time: %lf, sleep time: %lf", (1/(fElapsedTime)), fElapsedTime, tickTime, tickTime - (fElapsedTime * 100));
		
		recursiveUpdate(FLG_FRAME, _children);
		//TO-DO copy framebuffer to main buffer
		
		console::write(_framebuffer, _color, getSize());
		
		if (fElapsedTime * 100 < tickTime) {
			sleep(tickTime - (fElapsedTime * 100));
		}
		
		clear();
		//for (int y = 0; y < _sizey - 1; y++)
		//	for (int x = 0; x < _sizex; x++)
		//		console::write(x, y, _framebuffer[get(x, y)], _color[get(x, y)]);
		
		//console::write(10, 10, 'h', BWHITE | FBLACK);
		//console::write(11, 10, 'i', BWHITE | FBLACK);
	}
	
	int getSize() {
		return _sizex * _sizey;
	}
	
	void write(int x, int y, char character, int color) {
		int _p = get(x, y);
		_framebuffer[_p] = character;
		_color[_p] = char(color);
	}
	
	void resize(int x, int y) {
		_sizex = x;
		_sizey = y;
		if (x == 0 && y == 0)
			return;
		_framebuffer = new char[x * y];
		_color = new char[x * y];
		clear();
	}
	
	int get(int x, int y) {
		return (y * _sizex) + x;
	}
	
	void add(eventHandler* evtchd) {
		_children.push_back(evtchd);
	}
	
	void remove(eventHandler* evtchd) {
		_children.push_back(evtchd);
	}
	
	//eventHandler<screen> events;
	eventHandler events;
	std::vector<eventHandler*> _children;
	int _sizex, _sizey;
	char* _framebuffer;
	char* _color;
	float fElapsedTime;
	float tickTime;
	int key;
	private:
	std::chrono::time_point<std::chrono::system_clock> tp1 = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock> tp2 = std::chrono::system_clock::now();

};

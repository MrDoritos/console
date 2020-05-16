#pragma once
#include "console.h"
#include "screen.h"

class framebuffer {
	public:
	framebuffer(framebuffer* parent, screen& screen, 
				int offsetx, int offsety, 
				int sizex, int sizey) 
	{
		this->_screen = &screen;
		this->_direct = false;
		this->_parent = parent;		
		this->_offsetx = offsetx;
		this->_offsety = offsety;
		resize(sizex, sizey);		
	}
		
	void write(int pos, char character, int color) {
		char* _fb;
		char* _cl;
		if (pos > getSize())
			return;
		
		if (direct()) {
			_screen->_framebuffer[_p] = character;
			_screen->_color[_p] = char(color);
		} else {
			_buffer[_p] = character;
			_color[_p] = char(color);
		}
	}
		
	void write(int x, int y, char character, int color) {
		char* _fb;
		char* _cl;
		//if (get(x, y, _sizex, _sizey) > getSize())
		//	return;
		
		//int _p = get(x, y);
		write(get(x, y), character, color);
	}
		
	int relGet(int x, int y) {
		if (_parent == nullptr) {
			return _screen->get(x, y);
		} else {
			return _parent->get(_offsetx + x, _offsety + y);
		}
	}
		
	int absGet(int x, int y) {
		if (_parent == nullptr) {
			return _screen->get(x + _offsetx, y + _offsety);
		} else {
			return _parent->absGet(_offsetx + x, _offsety + y);
		}
	}
		
		
	void resize(int ox, int oy, int x, int y) {
		_offsetx = ox;
		_offsety = oy;
		resize(x, y);
	}
		
	void resize(int x, int y) {
		_sizex = x;
		_sizey = y;
		if (x == 0 && y == 0)
			return;
		
		_buffer = new char[getSize(x, y)];
		_color = new char[getSize(x, y)];
	}	
		
	static int get(int x, int y, int sizex, int sizey) {
		return (y * sizey) + x;
	}
	
	int get(int x, int y) {
		if (direct()) {
			absGet(x, y);
		} else {
			return get(x, y, _sizex, _sizey);
		}
	}
	
	bool direct() { return _direct; }
	void direct(bool _direct) { this->_direct = _direct; }
	
	static int getSize(int sizex, int sizey) {
		return sizex * sizey;
	}
	
	int getSize() {
		return getSize(_sizex, _sizey);
	}
	
	int getX() {
		return _sizex;
	}
	
	int getY() {
		return _sizey;
	}
	
	int _sizex, _sizey;
	int _offsetx, _offsety;
	framebuffer* _parent;
	bool _direct;
	screen* _screen;
	char* _buffer;
	char* _color;
};

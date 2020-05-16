#pragma once
#include <vector>
#include "eventHandler.h"
#include "screen.h"
#include "framebuffer.h"

template<class _b>
class element {
	public:
	element(screen& screen, 
	int ox, int oy, int sx, int sy) 
	:_framebuffer(nullptr, screen,
	ox, oy, sx, sy)
	{	
	}
	
	element(element* parent, screen& screen,
	int ox, int oy, int sx, int sy)
	:_framebuffer(&parent->_framebuffer, screen,
	ox, oy, sx, sy)
	{		
	}
	
	framebuffer _framebuffer;
	eventHandler events;
	std::vector<element*> _children;
};
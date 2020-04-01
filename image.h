#pragma once
#include "element.h"
#include "png.h"
#include <cmath>

class image : public element<image> {
	public:
	image(screen& scr, png& spng, int offsetx, int offsety, int sizex, int sizey)
	:element<image>(scr, offsetx, offsety, sizex, sizey),
	 simage(&spng)
	{
		events.setReferer(this);
		events.assign(FLG_FRAME, func(image, image::onFrame));
		events.assign(FLG_CREATE, func(image, image::onCreate));
		events.assign(FLG_RESIZE, func(image, image::onResize));
	}
	
	void onResize() {
		limitedResize(console::getConsoleWidth(),console::getConsoleHeight());
	}
	
	void onCreate() {
		limitedResize(_framebuffer.getX(), _framebuffer.getY());
	}
	
	void onFrame() {
		sample();
	}
	
	
	void limitedResize(int sizeX, int sizeY) {
		/*
		double nW, nH;
		double rs = double(sizeX) / double(sizeY);
		double ri = double(simage->getSizeX()) / double(simage->getSizeY());
		
		        if (rs > ri) {
                nW = (double(simage->getSizeX()) * double(sizeY)) / double(simage->getSizeY());
                                              nH = double(sizeY);
                                                  } else {
                                          nW = double(sizeX);
             nH = (double(simage->getSizeY()) * double(sizeX)) / double(simage->getSizeX());
                                                                }
                             //   _framebuffer._offsetx = round((double(sizeX) - nW) / 2);
                             //             _framebuffer._offsety = round((double(sizeY) - nH) / 2);
                             //                _framebuffer._sizex = round(nW);
                            //               _framebuffer._sizey  = round(nH);
				_framebuffer.resize(round((double(sizeX) - nW) / 3),
						    round((double(sizeY) - nH) / 2),
						    round(nW) * 2, round(nH));
				fprintf(stderr, "ox: %i, oy: %i, sx: %i, sy: %i",
						round((double(sizeX) - nW) / 4),
						round((double(sizeY) - nH) / 2),
						round(nW) * 2, round(nH));
		*/
	}
	/*
	void limitedResize(int sizeX, int sizeY) {
		int newW, newH;
		int y_W = sizeX;
		int y_H = sizeY;
		int m_W = simage->getSizeX();
		int m_H = simage->getSizeY();
		float rW = y_W / m_W;
		float rH = y_H / m_H;
	
		if (rW > rH) {
			newH = int(m_H / rW);
			newW = sizeX;
		} else {
			newW = int(m_W / rH);
			newH = sizeY;
		}
	
		_framebuffer.resize(newW, newH);
	
		sample();
	}
	*/
	
	private:
	void sample() {
		const auto get_color = [](pixel* pix) {
			int outc = 0;
			outc = pix->red > 126 ? BRED : 0;
			outc |= pix->green > 126 ? BGREEN : 0;
			outc |= pix->blue > 126 ? BBLUE : 0;
			outc |= pix->value() > 0.6f ? 0b10000000 : 0b00000000;
			return outc;
		};
		for (int x = 0; x < _framebuffer.getX(); x++) {
			for (int y = 0; y < _framebuffer.getY(); y++) {
				pixel* sample = simage->getSample(x, y, _framebuffer.getX(), _framebuffer.getY());
				_framebuffer.write(x, y, ' ', get_color(sample));
				//_framebuffer.write(x, y, getChar(sample->value()), get_color(sample));
			}
		}
	}
	
	inline char getChar(float val) {
		const int size = 70;
		const char scC[71] = { "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\|()1{}[]?-_+~<>i!lI;:,\"^`'. " };
		return scC[int(size * val)];
	}

	png* simage;
};

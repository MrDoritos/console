#define RGBMAXVALUE 765.0f

#pragma once

struct pixel
{
public:
	pixel(char red, char green, char blue, char alpha);
	pixel(char red, char green, char blue);
	pixel();
	unsigned char red, green, blue, alpha;
	inline unsigned int sum() { 
		return red + green + blue; 
	}
	inline float value() { 
		return float (((sum()) / RGBMAXVALUE));
	}
	inline float alphaValue() {
		return value() * alpha;
	}
	inline float value(unsigned int rgb) {
		return float((rgb / RGBMAXVALUE));
	}
	inline float alphaValue(unsigned int rgb) {
		return value(rgb) * alpha;
	}
	inline unsigned int negative() {
		return RGBMAXVALUE - sum();
	}
	inline float negativeValue() {
		return value(negative());
	}
};


#include "pixel.h"

pixel::pixel(char red, char green, char blue, char alpha)
{
	this->red = red;
	this->green = green;
	this->blue = blue;
	this->alpha = alpha;
}

pixel::pixel(char red, char green, char blue)
{
	this->red = red;
	this->green = green;
	this->blue = blue;
	this->alpha = 255;
}

pixel::pixel() {

}

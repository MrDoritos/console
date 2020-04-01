#pragma once
#include "lodepng.h"
#include <iostream>
#include <vector>
#include <string>
#include "pixel.h"

class png {
	public:
	png(const char* path) {
		load(path);
	}	
	
	png(std::vector<unsigned char>& rawBuffer, int width, int height) {
		convert(rawBuffer);
		_sizex = width;
		_sizey = height;
	}
	
	png(std::vector<unsigned char>& data) {
		load(data);
	}
	
	void getPixel(int x, int y, pixel& pixel) {
		pixel = *pixels[get(x, y)];
	}
	
	pixel* getPixel(int x, int y) {
		return pixels[get(x,y)];
	}
	
	pixel* getPixel(int index) {
		return pixels[index];
	}
	
	pixel* getSample(int x, int y, int sizex, int sizey) {
		float scaleX = float(sizex) / float(_sizex), scaleY = float(sizey) / float(_sizey);
		return getPixel((int)(x / scaleX), (int)(y / scaleY));
	}
	
	int getSizeX() { return _sizex; }
	int getSizeY() { return _sizey; }
	int getPixelCount() { return _sizex * _sizey; }
	int get(int x, int y) { return ((y * _sizex) + x); }
	
	void deconvert(std::vector<unsigned char>& rawImage) {
		int size = rawImage.size();
		int pixelSize = pixels.size();
		for (int i = 0, p = 0; i < size && p < pixelSize; i+=4, p++ ) {
			rawImage[i] = pixels[p]->red;
			rawImage[i + 1] = pixels[p]->green;
			rawImage[i + 2] = pixels[p]->blue;
			rawImage[i + 3] = pixels[p]->alpha;
		}
	}
	
	private:
	
	int load(std::vector<unsigned char>& data) {
		std::vector<unsigned char>* image = new std::vector<unsigned char>();
		unsigned width, height, error;
		
		fprintf(stderr, "loading image\r\n");
		error = lodepng::decode(*image, width, height, data);
		
		if (error) {
			fprintf(stderr, "lodepng error %i\r\n", error);
			exit(error);
		}
		
		_sizex = width;
		_sizey = height;
		
		fprintf(stderr, "converting image\r\n");
		convert(*image);

		fprintf(stderr, "done\r\n");

		return 0;
	}
	
	int load(const char* path) {
		std::vector<unsigned char>* image = new std::vector<unsigned char>();
		unsigned width, height, error;
		
		error = lodepng::decode(*image, width, height, path);
		
		if (error) {
			fprintf(stderr, "lodepng error %i\r\n", error);
			exit(error);
		}
		
		_sizex = width;
		_sizey = height;
		
		convert(*image);

		return 0;
	}
	
	
	void convert(std::vector<unsigned char>& image) {
		int size = image.size();
		int pixelcount = size / 4;
		pixels = std::vector<pixel*>(pixelcount);
//		pixels.resize(pixelcount);
		pixel* thesepixels = new pixel[pixelcount];//(pixel*)malloc(sizeof(pixel) * pixelcount);
		//fprintf(stderr, "ggg");
		for (int i = 0, p = 0; i < size; p++, i += 4) {
				pixels[p] = &(thesepixels[p] = pixel(image[i], image[i+1], image[i+2],image[i+3]));
		}
		//fprintf(stderr, "hhh");
	}
	
	std::vector<pixel*> pixels;
	int _sizex, _sizey;
};

//Process a binary PPM file
#include <vector>
#include <string>
#include "../Core/Vector.h"

#ifndef PPM_H
#define PPM_H

class ppm {
	void init();
	//info about the PPM file (height and width)
	unsigned int nr_lines;
	unsigned int nr_columns;

public:
	unsigned int height;
	unsigned int width;
	unsigned int max_col_val;
	//total number of elements (pixels)
	unsigned int size;

	ppm();
	//create a PPM object and fill it with data stored in fname 
	ppm(const std::string &fname);
	//create an "epmty" PPM image with a given width and height;the R,G,B arrays are filled with zeros
	ppm(const unsigned int _width, const unsigned int _height);
	//read the PPM image from fname
	void read(const std::string &fname);
	//write the PPM image in fname
	void write(const std::string &fname);

	void setPixel(const vec2i& aPixel, const vec3i& aColor);

private:
	//arrays for storing the R,G,B values
	std::vector<unsigned char> r;
	std::vector<unsigned char> g;
	std::vector<unsigned char> b;
	//
};

#endif
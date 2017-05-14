/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

Raster::Raster(vector<int>* data, int w, int h) {
	this->data = data;
	width = w;
	height = h;
}

Raster::Raster(int w, int h) {
	data = new vector<int>(w * h, 0);
	width = w;
	height = h;
}

void Raster::drawTile(Raster* srcRaster, int srcx, int srcy, int dstx, int dsty, int w, int h) {
	vector<int>* src = srcRaster->data;
	int src_index;
	int dst_index;
	int tmp;

	for(int y = 0; y < h; ++y) {

		src_index = (srcy + y) * srcRaster->width + srcx;
		dst_index = (dsty + y) * width + dstx;

		for(int x = 0; x < w; ++x) {

			if((tmp = (*src)[src_index]) != 0) {
				(*data)[dst_index] = tmp;
			}

			++src_index;
			++dst_index;

		}
	}
}

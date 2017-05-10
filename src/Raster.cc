/*
SaltyNES Copyright (c) 2012-2014 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
vNES Copyright (c) 2006-2011 Jamie Sanders

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.
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

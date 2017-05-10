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

NameTable::NameTable(int width, int height, string name) {
	this->name = name;
	this->tile = new vector<uint16_t>(width * height, 0);
	this->attrib = new vector<uint16_t>(width * height, 0);
	this->width = width;
	this->height = height;
}

NameTable::~NameTable() {
	delete_n_null(tile);
	delete_n_null(attrib);
}

uint16_t NameTable::getTileIndex(int x, int y) {
	return (*tile)[y * width + x];
}

uint16_t NameTable::getAttrib(int x, int y) {
	return (*attrib)[y * width + x];
}

void NameTable::writeTileIndex(int index, int value) {
	(*tile)[index] = static_cast<uint16_t>(value);
}

void NameTable::writeAttrib(int index, int value) {
	int basex, basey;
	int add;
	int tx, ty;
	//int attindex;
	basex = index % 8;
	basey = index / 8;
	basex *= 4;
	basey *= 4;

	for(int sqy = 0; sqy < 2; ++sqy) {
		for(int sqx = 0; sqx < 2; ++sqx) {
			add = (value >> (2 * (sqy * 2 + sqx))) & 3;
			for(int y = 0; y < 2; ++y) {
				for(int x = 0; x < 2; ++x) {
					tx = basex + sqx * 2 + x;
					ty = basey + sqy * 2 + y;
					//attindex = ty * width + tx;
					(*attrib)[ty * width + tx] = static_cast<uint16_t>((add << 2) & 12);
				////System.out.println("x="+tx+" y="+ty+" value="+attrib[ty*width+tx]+" index="+attindex);
				}
			}
		}
	}
}

void NameTable::stateSave(ByteBuffer* buf) {
	for(int i = 0; i < width * height; ++i) {
		if((*tile)[i] > 255)//System.out.println(">255!!");
		{
			buf->putByte(static_cast<uint8_t>((*tile)[i]));
		}
	}
	for(int i = 0; i < width * height; ++i) {
		buf->putByte(static_cast<uint8_t>((*attrib)[i]));
	}
}

void NameTable::stateLoad(ByteBuffer* buf) {
	for(int i = 0; i < width * height; ++i) {
		(*tile)[i] = buf->readByte();
	}
	for(int i = 0; i < width * height; ++i) {
		(*attrib)[i] = buf->readByte();
	}
}

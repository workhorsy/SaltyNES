/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

NameTable::NameTable() {
	this->name = "";
	this->tile.fill(0);
	this->attrib.fill(0);
	this->width = 32;
	this->height = 32;
}

NameTable::NameTable(string name) {
	this->name = name;
	this->tile.fill(0);
	this->attrib.fill(0);
	this->width = 32;
	this->height = 32;
}

NameTable::~NameTable() {
}

uint16_t NameTable::getTileIndex(int x, int y) {
	return tile[y * width + x];
}

uint16_t NameTable::getAttrib(int x, int y) {
	return attrib[y * width + x];
}

void NameTable::writeTileIndex(int index, int value) {
	tile[index] = static_cast<uint16_t>(value);
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
					attrib[ty * width + tx] = static_cast<uint16_t>((add << 2) & 12);
				////System.out.println("x="+tx+" y="+ty+" value="+attrib[ty*width+tx]+" index="+attindex);
				}
			}
		}
	}
}

void NameTable::stateSave(ByteBuffer* buf) {
	for(int i = 0; i < width * height; ++i) {
		if(tile[i] > 255)//System.out.println(">255!!");
		{
			buf->putByte(static_cast<uint8_t>(tile[i]));
		}
	}
	for(int i = 0; i < width * height; ++i) {
		buf->putByte(static_cast<uint8_t>(attrib[i]));
	}
}

void NameTable::stateLoad(ByteBuffer* buf) {
	for(int i = 0; i < width * height; ++i) {
		tile[i] = buf->readByte();
	}
	for(int i = 0; i < width * height; ++i) {
		attrib[i] = buf->readByte();
	}
}

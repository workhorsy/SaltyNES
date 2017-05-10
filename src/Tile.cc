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


Tile::Tile() {
	// Tile data:
	pix = vector<int>(64, 0);
	fbIndex = 0;
	tIndex = 0;
	x = 0;
	y = 0;
	w = 0;
	h = 0;
	incX = 0;
	incY = 0;
	palIndex = 0;
	tpri = 0;
	c = 0;
	initialized = false;
	opaque = vector<bool>(8, false);
}

void Tile::setBuffer(vector<uint16_t>* scanline) {
	for(y = 0; y < 8; ++y) {
		setScanline(y, (*scanline)[y], (*scanline)[y + 8]);
	}
}

void Tile::setScanline(int sline, uint16_t b1, uint16_t b2) {
	initialized = true;
	tIndex = sline << 3;
	for(x = 0; x < 8; ++x) {
		pix[tIndex + x] = ((b1 >> (7 - x)) & 1) + (((b2 >> (7 - x)) & 1) << 1);
		if(pix[tIndex + x] == 0) {
			opaque[sline] = false;
		}
	}
}

void Tile::renderSimple(int dx, int dy, vector<int>* fBuffer, int palAdd, int* palette) {
	tIndex = 0;
	fbIndex = (dy << 8) + dx;
	for(y = 8; y != 0; --y) {
		for(x = 8; x != 0; --x) {
			palIndex = pix[tIndex];
			if(palIndex != 0) {
				(*fBuffer)[fbIndex] = palette[palIndex + palAdd];
			}
			++fbIndex;
			++tIndex;
		}
		fbIndex -= 8;
		fbIndex += 256;
	}
}

void Tile::renderSmall(int dx, int dy, vector<int>* buffer, int palAdd, int* palette) {
	tIndex = 0;
	fbIndex = (dy << 8) + dx;
	for(y = 0; y < 4; ++y) {
		for(x = 0; x < 4; ++x) {

			c = (palette[pix[tIndex] + palAdd] >> 2) & 0x003F3F3F;
			c += (palette[pix[tIndex + 1] + palAdd] >> 2) & 0x003F3F3F;
			c += (palette[pix[tIndex + 8] + palAdd] >> 2) & 0x003F3F3F;
			c += (palette[pix[tIndex + 9] + palAdd] >> 2) & 0x003F3F3F;
			(*buffer)[fbIndex] = c;
			++fbIndex;
			tIndex += 2;
		}
		tIndex += 8;
		fbIndex += 252;
	}

}

void Tile::render(int srcx1, int srcy1, int srcx2, int srcy2, int dx, int dy, vector<int>* fBuffer, int palAdd, vector<int>* palette, bool flipHorizontal, bool flipVertical, int pri, vector<int>* priTable) {
	if(dx < -7 || dx >= 256 || dy < -7 || dy >= 240) {
		return;
	}

	w = srcx2 - srcx1;
	h = srcy2 - srcy1;

	if(dx < 0) {
		srcx1 -= dx;
	}
	if(dx + srcx2 >= 256) {
		srcx2 = 256 - dx;
	}

	if(dy < 0) {
		srcy1 -= dy;
	}
	if(dy + srcy2 >= 240) {
		srcy2 = 240 - dy;
	}

	if(!flipHorizontal && !flipVertical) {

		fbIndex = (dy << 8) + dx;
		tIndex = 0;
		for(y = 0; y < 8; ++y) {
			for(x = 0; x < 8; ++x) {
				if(x >= srcx1 && x < srcx2 && y >= srcy1 && y < srcy2) {
					palIndex = pix[tIndex];
					tpri = (*priTable)[fbIndex];
					if(palIndex != 0 && pri <= (tpri & 0xFF)) {
						(*fBuffer)[fbIndex] = (*palette)[palIndex + palAdd];
						tpri = (tpri & 0xF00) | pri;
						(*priTable)[fbIndex] = tpri;
					}
				}
				++fbIndex;
				++tIndex;
			}
			fbIndex -= 8;
			fbIndex += 256;
		}

	} else if(flipHorizontal && !flipVertical) {

		fbIndex = (dy << 8) + dx;
		tIndex = 7;
		for(y = 0; y < 8; ++y) {
			for(x = 0; x < 8; ++x) {
				if(x >= srcx1 && x < srcx2 && y >= srcy1 && y < srcy2) {
					palIndex = pix[tIndex];
					tpri = (*priTable)[fbIndex];
					if(palIndex != 0 && pri <= (tpri & 0xFF)) {
						(*fBuffer)[fbIndex] = (*palette)[palIndex + palAdd];
						tpri = (tpri & 0xF00) | pri;
						(*priTable)[fbIndex] = tpri;
					}
				}
				++fbIndex;
				--tIndex;
			}
			fbIndex -= 8;
			fbIndex += 256;
			tIndex += 16;
		}

	} else if(flipVertical && !flipHorizontal) {

		fbIndex = (dy << 8) + dx;
		tIndex = 56;
		for(y = 0; y < 8; ++y) {
			for(x = 0; x < 8; ++x) {
				if(x >= srcx1 && x < srcx2 && y >= srcy1 && y < srcy2) {
					palIndex = pix[tIndex];
					tpri = (*priTable)[fbIndex];
					if(palIndex != 0 && pri <= (tpri & 0xFF)) {
						(*fBuffer)[fbIndex] = (*palette)[palIndex + palAdd];
						tpri = (tpri & 0xF00) | pri;
						(*priTable)[fbIndex] = tpri;
					}
				}
				++fbIndex;
				++tIndex;
			}
			fbIndex -= 8;
			fbIndex += 256;
			tIndex -= 16;
		}

	} else {

		fbIndex = (dy << 8) + dx;
		tIndex = 63;
		for(y = 0; y < 8; ++y) {
			for(x = 0; x < 8; ++x) {
				if(x >= srcx1 && x < srcx2 && y >= srcy1 && y < srcy2) {
					palIndex = pix[tIndex];
					tpri = (*priTable)[fbIndex];
					if(palIndex != 0 && pri <= (tpri & 0xFF)) {
						(*fBuffer)[fbIndex] = (*palette)[palIndex + palAdd];
						tpri = (tpri & 0xF00) | pri;
						(*priTable)[fbIndex] = tpri;
					}
				}
				++fbIndex;
				--tIndex;
			}
			fbIndex -= 8;
			fbIndex += 256;
		}

	}
}

bool Tile::isTransparent(int x, int y) {
	return (pix[(y << 3) + x] == 0);
}

void Tile::dumpData(string file) {
	try {

		ofstream writer(file.c_str(), ios::out|ios::binary);
		string chunk;
		for(int y = 0; y < 8; ++y) {
			for(int x = 0; x < 8; ++x) {
				chunk = Misc::hex8(pix[(y << 3) + x]).substr(1);
				writer.write(chunk.c_str(), chunk.length());
			}
			chunk = "\r\n";
			writer.write(chunk.c_str(), chunk.length());
		}

		writer.close();
	//System.out.println("Tile data dumped to file "+file);

	} catch (exception& e) {
		//System.out.println("Unable to dump tile to file.");
//			e.printStackTrace();
	}
}

void Tile::stateSave(ByteBuffer* buf) {
	buf->putBoolean(initialized);
	for(int i = 0; i < 8; ++i) {
		buf->putBoolean(opaque[i]);
	}
	for(int i = 0; i < 64; ++i) {
		buf->putByte(static_cast<uint8_t>(pix[i]));
	}
}

void Tile::stateLoad(ByteBuffer* buf) {
	initialized = buf->readBoolean();
	for(int i = 0; i < 8; ++i) {
		opaque[i] = buf->readBoolean();
	}
	for(int i = 0; i < 64; ++i) {
		pix[i] = buf->readByte();
	}
}

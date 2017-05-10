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

void Mapper007::init(NES* nes) {
	this->base_init(nes);
	currentOffset = 0;
	currentMirroring = -1;

	// Get ref to ROM:
	ROM* rom = nes->getRom();

	// Read out all PRG rom:
	int bc = rom->getRomBankCount();
	prgrom = vector<uint16_t>(bc * 16384, 0);
	for(int i = 0; i < bc; ++i) {
		arraycopy_short(rom->getRomBank(i), 0, &prgrom, i * 16384, 16384);
	}
}

uint16_t Mapper007::load(int address) {
	if(address < 0x8000) {
		// Register read
		return this->base_load(address);
	} else {
		if((address + currentOffset) >= 262144) {
			return prgrom[(address + currentOffset) - 262144];
		} else {
			return prgrom[address + currentOffset];
		}
	}
}

void Mapper007::write(int address, uint16_t value) {
	if(address < 0x8000) {
		// Let the base mapper take care of it.
		this->base_write(address, value);
	} else {
		// Set PRG offset:
		currentOffset = ((value & 0xF) - 1) << 15;

		// Set mirroring:
		if(currentMirroring != (value & 0x10)) {

			currentMirroring = value & 0x10;
			if(currentMirroring == 0) {
				nes->getPpu()->setMirroring(ROM::SINGLESCREEN_MIRRORING);
			} else {
				nes->getPpu()->setMirroring(ROM::SINGLESCREEN_MIRRORING2);
			}

		}

	}
}

void Mapper007::mapperInternalStateLoad(ByteBuffer* buf) {
	this->base_mapperInternalStateLoad(buf);
	// Check version:
	if(buf->readByte() == 1) {
		currentMirroring = buf->readByte();
		currentOffset = buf->readInt();
	}
}

void Mapper007::mapperInternalStateSave(ByteBuffer* buf) {
	this->base_mapperInternalStateSave(buf);

	// Version:
	buf->putByte(static_cast<uint16_t>(1));

	// State:
	buf->putByte(static_cast<uint16_t>(currentMirroring));
	buf->putInt(currentOffset);
}

void Mapper007::reset() {
	this->base_reset();
	currentOffset = 0;
	currentMirroring = -1;
}

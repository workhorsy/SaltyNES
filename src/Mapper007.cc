/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

Mapper007::Mapper007() : MapperDefault() {

}

shared_ptr<MapperDefault> Mapper007::Init(shared_ptr<NES> nes) {
	currentOffset = 0;
	currentMirroring = -1;

	// Get ref to ROM:
	shared_ptr<ROM> rom = nes->getRom();

	// Read out all PRG rom:
	int bc = rom->getRomBankCount();
	prgrom = vector<uint16_t>(bc * 16384, 0);
	for(int i = 0; i < bc; ++i) {
		array_copy(rom->getRomBank(i), 0, &prgrom, i * 16384, 16384);
	}

	this->base_init(nes);
	return shared_from_this();
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

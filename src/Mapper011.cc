/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

Mapper011::Mapper011() : MapperDefault() {

}

shared_ptr<MapperDefault> Mapper011::Init(shared_ptr<NES> nes) {
	this->base_init(nes);
	return shared_from_this();
}

void Mapper011::write(int address, uint16_t value) {
	if (address < 0x8000) {
		// Let the base mapper take care of it
		this->base_write(address, value);
	} else {
		// Swap in the given PRG-ROM bank:
		int prgbank1 = ((value & 0xF) * 2) % this->nes->rom->romCount;
		int prgbank2 = ((value & 0xF) * 2 + 1) % this->nes->rom->romCount;

		loadRomBank(prgbank1, 0x8000);
		loadRomBank(prgbank2, 0xC000);

		if (this->nes->rom->vromCount > 0) {
			// Swap in the given VROM bank at 0x0000:
			int bank = ((value >> 4) * 2) % (this->nes->rom->vromCount);
			loadVromBank(bank, 0x0000);
			loadVromBank(bank + 1, 0x1000);
		}
	}
}


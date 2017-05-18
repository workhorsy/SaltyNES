/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

Mapper003::Mapper003() : MapperDefault() {

}

shared_ptr<MapperDefault> Mapper003::Init(shared_ptr<NES> nes) {
	this->base_init(nes);
	return shared_from_this();
}

void Mapper003::write(int address, uint16_t value) {
	if(address < 0x8000) {
		// Let the base mapper take care of it.
		this->base_write(address, value);
	} else {
		// This is a VROM bank select command.
		// Swap in the given VROM bank at 0x0000:
		int bank = (value % (nes->getRom()->getVromBankCount() / 2)) * 2;
		loadVromBank(bank, 0x0000);
		loadVromBank(bank + 1, 0x1000);
		load8kVromBank(value * 2, 0x0000);
	}
}

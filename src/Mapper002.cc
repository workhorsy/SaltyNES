/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

Mapper002::Mapper002() : MapperDefault() {

}

shared_ptr<MapperDefault> Mapper002::Init(shared_ptr<NES> nes) {
	this->base_init(nes);
	return shared_from_this();
}

void Mapper002::write(int address, uint16_t value) {
	if(address < 0x8000) {
		// Let the base mapper take care of it.
		this->base_write(address, value);
	} else {
		// This is a ROM bank select command.
		// Swap in the given ROM bank at 0x8000:
		loadRomBank(value, 0x8000);
	}
}

void Mapper002::loadROM(shared_ptr<ROM> rom) {
	if(!rom->isValid()) {
		//System.out.println("UNROM: Invalid ROM! Unable to load.");
		return;
	}

	//System.out.println("UNROM: loading ROM..");

	// Load PRG-ROM:
	loadRomBank(0, 0x8000);
	loadRomBank(rom->getRomBankCount() - 1, 0xC000);

	// Load CHR-ROM:
	loadCHRROM();

	// Do Reset-Interrupt:
	//nes.getCpu().doResetInterrupt();
	nes->getCpu()->requestIrq(CPU::IRQ_RESET);
}

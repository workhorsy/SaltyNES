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


void Mapper002::init(NES* nes) {
	this->base_init(nes);
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

void Mapper002::loadROM(ROM* rom) {
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

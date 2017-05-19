/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

Mapper009::Mapper009() : MapperDefault() {

}

shared_ptr<MapperDefault> Mapper009::Init(shared_ptr<NES> nes) {
	latchLo = 0;
	latchHi = 0;
	latchLoVal1 = 0;
	latchLoVal2 = 0;
	latchHiVal1 = 0;
	latchHiVal2 = 0;

	reset();

	this->base_init(nes);
	return shared_from_this();
}

void Mapper009::write(int address, uint16_t value) {
	if(address < 0x8000) {

		// Handle normally.
		this->base_write(address, value);

	} else {

		// MMC2 write.

		value &= 0xFF;
		address &= 0xF000;
		switch (address >> 12) {
			case 0xA: {

				// Select 8k ROM bank at 0x8000
				load8kRomBank(value, 0x8000);
				return;

			}
			case 0xB: {

				// Select 4k VROM bank at 0x0000, $FD mode
				latchLoVal1 = value;
				if(latchLo == 0xFD) {
					loadVromBank(value, 0x0000);
				}
				return;

			}
			case 0xC: {

				// Select 4k VROM bank at 0x0000, $FE mode
				latchLoVal2 = value;
				if(latchLo == 0xFE) {
					loadVromBank(value, 0x0000);
				}
				return;

			}
			case 0xD: {

				// Select 4k VROM bank at 0x1000, $FD mode
				latchHiVal1 = value;
				if(latchHi == 0xFD) {
					loadVromBank(value, 0x1000);
				}
				return;

			}
			case 0xE: {

				// Select 4k VROM bank at 0x1000, $FE mode
				latchHiVal2 = value;
				if(latchHi == 0xFE) {
					loadVromBank(value, 0x1000);
				}
				return;

			}
			case 0xF: {

				// Select mirroring
				if((value & 0x1) == 0) {

					// Vertical mirroring
					nes->getPpu()->setMirroring(ROM::VERTICAL_MIRRORING);

				} else {

					// Horizontal mirroring
					nes->getPpu()->setMirroring(ROM::HORIZONTAL_MIRRORING);

				}
				return;
			}
		}
	}
}

void Mapper009::loadROM(shared_ptr<ROM> rom) {
	//System.out.println("Loading ROM.");

	if(!rom->isValid()) {
		//System.out.println("MMC2: Invalid ROM! Unable to load.");
		return;
	}

	// Get number of 8K banks:
	int num_8k_banks = rom->getRomBankCount() * 2;

	// Load PRG-ROM:
	load8kRomBank(0, 0x8000);
	load8kRomBank(num_8k_banks - 3, 0xA000);
	load8kRomBank(num_8k_banks - 2, 0xC000);
	load8kRomBank(num_8k_banks - 1, 0xE000);

	// Load CHR-ROM:
	loadCHRROM();

	// Load Battery RAM (if present):
	loadBatteryRam();

	// Do Reset-Interrupt:
	nes->getCpu()->requestIrq(CPU::IRQ_RESET);
}

void Mapper009::latchAccess(int address) {
	if((address & 0x1FF0) == 0x0FD0 && latchLo != 0xFD) {
		// Set $FD mode
		loadVromBank(latchLoVal1, 0x0000);
		latchLo = 0xFD;
	//System.out.println("LO FD");
	} else if((address & 0x1FF0) == 0x0FE0 && latchLo != 0xFE) {
		// Set $FE mode
		loadVromBank(latchLoVal2, 0x0000);
		latchLo = 0xFE;
	//System.out.println("LO FE");
	} else if((address & 0x1FF0) == 0x1FD0 && latchHi != 0xFD) {
		// Set $FD mode
		loadVromBank(latchHiVal1, 0x1000);
		latchHi = 0xFD;
	//System.out.println("HI FD");
	} else if((address & 0x1FF0) == 0x1FE0 && latchHi != 0xFE) {
		// Set $FE mode
		loadVromBank(latchHiVal2, 0x1000);
		latchHi = 0xFE;
	//System.out.println("HI FE");
	}
}

void Mapper009::mapperInternalStateLoad(ByteBuffer* buf) {
	this->base_mapperInternalStateLoad(buf);

	// Check version:
	if(buf->readByte() == 1) {

		latchLo = buf->readByte();
		latchHi = buf->readByte();
		latchLoVal1 = buf->readByte();
		latchLoVal2 = buf->readByte();
		latchHiVal1 = buf->readByte();
		latchHiVal2 = buf->readByte();

	}
}

void Mapper009::mapperInternalStateSave(ByteBuffer* buf) {
	this->base_mapperInternalStateSave(buf);

	// Version:
	buf->putByte(static_cast<uint16_t>(1));

	// State:
	buf->putByte(static_cast<uint8_t>(latchLo));
	buf->putByte(static_cast<uint8_t>(latchHi));
	buf->putByte(static_cast<uint8_t>(latchLoVal1));
	buf->putByte(static_cast<uint8_t>(latchLoVal2));
	buf->putByte(static_cast<uint8_t>(latchHiVal1));
	buf->putByte(static_cast<uint8_t>(latchHiVal2));
}

void Mapper009::reset() {
	// Set latch to $FE mode:
	latchLo = 0xFE;
	latchHi = 0xFE;
	latchLoVal1 = 0;
	latchLoVal2 = 4;
	latchHiVal1 = 0;
	latchHiVal2 = 0;
}

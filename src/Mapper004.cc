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



Mapper004::Mapper004() {
	prgAddressChanged = false;
}

void Mapper004::init(NES* nes) {
	this->base_init(nes);
}

void Mapper004::mapperInternalStateLoad(ByteBuffer* buf) {
	this->base_mapperInternalStateLoad(buf);

	// Check version:
	if(buf->readByte() == 1) {
		command = buf->readInt();
		prgAddressSelect = buf->readInt();
		chrAddressSelect = buf->readInt();
		pageNumber = buf->readInt();
		irqCounter = buf->readInt();
		irqLatchValue = buf->readInt();
		irqEnable = buf->readInt();
		prgAddressChanged = buf->readBoolean();
	}
}

void Mapper004::mapperInternalStateSave(ByteBuffer* buf) {
	this->base_mapperInternalStateSave(buf);

	// Version:
	buf->putByte(static_cast<uint16_t>(1));

	// State:
	buf->putInt(command);
	buf->putInt(prgAddressSelect);
	buf->putInt(chrAddressSelect);
	buf->putInt(pageNumber);
	buf->putInt(irqCounter);
	buf->putInt(irqLatchValue);
	buf->putInt(irqEnable);
	buf->putBoolean(prgAddressChanged);

}

void Mapper004::write(int address, uint16_t value) {
	if(address < 0x8000) {
		// Normal memory write.
		this->base_write(address, value);
		return;
	}

	if(address == 0x8000) {
		// Command/Address Select register
		command = value & 7;
		int tmp = (value >> 6) & 1;
		if(tmp != prgAddressSelect) {
			prgAddressChanged = true;
		}
		prgAddressSelect = tmp;
		chrAddressSelect = (value >> 7) & 1;
	} else if(address == 0x8001) {
		// Page number for command
		executeCommand(command, value);
	} else if(address == 0xA000) {
		// Mirroring select
		if((value & 1) != 0) {
			nes->getPpu()->setMirroring(ROM::HORIZONTAL_MIRRORING);
		} else {
			nes->getPpu()->setMirroring(ROM::VERTICAL_MIRRORING);
		}
	} else if(address == 0xA001) {
		// SaveRAM Toggle
		nes->getRom()->setSaveState((value & 1) != 0);
	} else if(address == 0xC000) {
		// IRQ Counter register
		irqCounter = value;
	//nes.ppu.mapperIrqCounter = 0;
	} else if(address == 0xC001) {
		// IRQ Latch register
		irqLatchValue = value;
	} else if(address == 0xE000) {
		// IRQ Control Reg 0 (disable)
		//irqCounter = irqLatchValue;
		irqEnable = 0;
	} else if(address == 0xE001) {
		// IRQ Control Reg 1 (enable)
		irqEnable = 1;
	} else {
		// Not a MMC3 register.
		// The game has probably crashed,
		// since it tries to write to ROM..
		// IGNORE.
	}
}

void Mapper004::executeCommand(int cmd, int arg) {
	if(cmd == CMD_SEL_2_1K_VROM_0000) {

		// Select 2 1KB VROM pages at 0x0000:
		if(chrAddressSelect == 0) {
			load1kVromBank(arg, 0x0000);
			load1kVromBank(arg + 1, 0x0400);
		} else {
			load1kVromBank(arg, 0x1000);
			load1kVromBank(arg + 1, 0x1400);
		}

	} else if(cmd == CMD_SEL_2_1K_VROM_0800) {

		// Select 2 1KB VROM pages at 0x0800:
		if(chrAddressSelect == 0) {
			load1kVromBank(arg, 0x0800);
			load1kVromBank(arg + 1, 0x0C00);
		} else {
			load1kVromBank(arg, 0x1800);
			load1kVromBank(arg + 1, 0x1C00);
		}

	} else if(cmd == CMD_SEL_1K_VROM_1000) {

		// Select 1K VROM Page at 0x1000:
		if(chrAddressSelect == 0) {
			load1kVromBank(arg, 0x1000);
		} else {
			load1kVromBank(arg, 0x0000);
		}

	} else if(cmd == CMD_SEL_1K_VROM_1400) {

		// Select 1K VROM Page at 0x1400:
		if(chrAddressSelect == 0) {
			load1kVromBank(arg, 0x1400);
		} else {
			load1kVromBank(arg, 0x0400);
		}

	} else if(cmd == CMD_SEL_1K_VROM_1800) {

		// Select 1K VROM Page at 0x1800:
		if(chrAddressSelect == 0) {
			load1kVromBank(arg, 0x1800);
		} else {
			load1kVromBank(arg, 0x0800);
		}

	} else if(cmd == CMD_SEL_1K_VROM_1C00) {

		// Select 1K VROM Page at 0x1C00:
		if(chrAddressSelect == 0) {
			load1kVromBank(arg, 0x1C00);
		} else {
			load1kVromBank(arg, 0x0C00);
		}

	} else if(cmd == CMD_SEL_ROM_PAGE1) {

		//Globals.println("cmd=SEL_ROM_PAGE1");
		if(prgAddressChanged) {
			//Globals.println("PRG Address has changed.");
			// Load the two hardwired banks:
			if(prgAddressSelect == 0) {
				load8kRomBank(((nes->getRom()->getRomBankCount() - 1) * 2), 0xC000);
			} else {

				load8kRomBank(((nes->getRom()->getRomBankCount() - 1) * 2), 0x8000);
			}
			prgAddressChanged = false;
		}

		// Select first switchable ROM page:
		//Globals.println("prgAddressSelect = "+prgAddressSelect+" arg="+arg);
		if(prgAddressSelect == 0) {
			load8kRomBank(arg, 0x8000);
		} else {
			load8kRomBank(arg, 0xC000);
		}

	} else if(cmd == CMD_SEL_ROM_PAGE2) {

		//Globals.println("cmd=SEL_ROM_PAGE2");
		//Globals.println("prgAddressSelect = "+prgAddressSelect+" arg="+arg);

		// Select second switchable ROM page:
		load8kRomBank(arg, 0xA000);

		// hardwire appropriate bank:
		if(prgAddressChanged) {
			//Globals.println("PRG Address has changed.");
			// Load the two hardwired banks:
			if(prgAddressSelect == 0) {
				load8kRomBank(((nes->getRom()->getRomBankCount() - 1) * 2), 0xC000);
			} else {

				load8kRomBank(((nes->getRom()->getRomBankCount() - 1) * 2), 0x8000);
			}
			prgAddressChanged = false;
		}
	}
}

void Mapper004::loadROM(ROM* rom) {
	//System.out.println("Loading ROM.");

	if(!rom->isValid()) {
		//System.out.println("MMC3: Invalid ROM! Unable to load.");
		return;
	}

	// Load hardwired PRG banks (0xC000 and 0xE000):
	load8kRomBank(((nes->getRom()->getRomBankCount() - 1) * 2), 0xC000);
	load8kRomBank(((nes->getRom()->getRomBankCount() - 1) * 2) + 1, 0xE000);

	// Load swappable PRG banks (0x8000 and 0xA000):
	load8kRomBank(0, 0x8000);
	load8kRomBank(1, 0xA000);

	// Load CHR-ROM:
	loadCHRROM();

	// Load Battery RAM (if present):
	loadBatteryRam();

	// Do Reset-Interrupt:
	//nes.getCpu().doResetInterrupt();
	nes->getCpu()->requestIrq(CPU::IRQ_RESET);
}

void Mapper004::clockIrqCounter() {
	if(irqEnable == 1) {
		--irqCounter;
		if(irqCounter < 0) {

			// Trigger IRQ:
			//nes.getCpu().doIrq();
			nes->getCpu()->requestIrq(CPU::IRQ_NORMAL);
			irqCounter = irqLatchValue;

		}
	}
}

void Mapper004::reset() {
	command = 0;
	prgAddressSelect = 0;
	chrAddressSelect = 0;
	pageNumber = 0;
	irqCounter = 0;
	irqLatchValue = 0;
	irqEnable = 0;
	prgAddressChanged = false;
}

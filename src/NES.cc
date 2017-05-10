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

// Creates the NES system.
#ifdef SDL
NES::NES(InputHandler* joy1, InputHandler* joy2) {
	_joy1 = joy1;
	_joy2 = joy2;
#endif
#ifdef NACL
NES::NES(SaltyNES* salty_nes) {
	_salty_nes = salty_nes;
	_joy1 = salty_nes->_joy1;
	_joy2 = salty_nes->_joy2;
#endif
	this->_is_paused = false;
	this->_isRunning = false;

	// Create memory:
	cpuMem = new Memory(this, 0x10000);	// Main memory (internal to CPU)
	ppuMem = new Memory(this, 0x8000);	// VRAM memory (internal to PPU)
	sprMem = new Memory(this, 0x100);	// Sprite RAM  (internal to PPU)

	// Create system units:
	cpu = new CPU(this);
	palTable = new PaletteTable();
	ppu = new PPU(this);
	papu = new PAPU(this);
	memMapper = nullptr;
	rom = nullptr;

	// Init sound registers:
	for(int i = 0; i < 0x14; ++i) {
		if(i == 0x10) {
			papu->writeReg(0x4010, static_cast<uint16_t>(0x10));
		} else {
			papu->writeReg(0x4000 + i, static_cast<uint16_t>(0));
		}
	}

	// Grab Controller Setting for Player 1:
	this->_joy1->mapKey(InputHandler::KEY_A, Globals::keycodes[Globals::controls["p1_a"]]);
	this->_joy1->mapKey(InputHandler::KEY_B, Globals::keycodes[Globals::controls["p1_b"]]);
	this->_joy1->mapKey(InputHandler::KEY_START, Globals::keycodes[Globals::controls["p1_start"]]);
	this->_joy1->mapKey(InputHandler::KEY_SELECT, Globals::keycodes[Globals::controls["p1_select"]]);
	this->_joy1->mapKey(InputHandler::KEY_UP, Globals::keycodes[Globals::controls["p1_up"]]);
	this->_joy1->mapKey(InputHandler::KEY_DOWN, Globals::keycodes[Globals::controls["p1_down"]]);
	this->_joy1->mapKey(InputHandler::KEY_LEFT, Globals::keycodes[Globals::controls["p1_left"]]);
	this->_joy1->mapKey(InputHandler::KEY_RIGHT, Globals::keycodes[Globals::controls["p1_right"]]);

	// Grab Controller Setting for Player 2:
	this->_joy2->mapKey(InputHandler::KEY_A, Globals::keycodes[Globals::controls["p2_a"]]);
	this->_joy2->mapKey(InputHandler::KEY_B, Globals::keycodes[Globals::controls["p2_b"]]);
	this->_joy2->mapKey(InputHandler::KEY_START, Globals::keycodes[Globals::controls["p2_start"]]);
	this->_joy2->mapKey(InputHandler::KEY_SELECT, Globals::keycodes[Globals::controls["p2_select"]]);
	this->_joy2->mapKey(InputHandler::KEY_UP, Globals::keycodes[Globals::controls["p2_up"]]);
	this->_joy2->mapKey(InputHandler::KEY_DOWN, Globals::keycodes[Globals::controls["p2_down"]]);
	this->_joy2->mapKey(InputHandler::KEY_LEFT, Globals::keycodes[Globals::controls["p2_left"]]);
	this->_joy2->mapKey(InputHandler::KEY_RIGHT, Globals::keycodes[Globals::controls["p2_right"]]);

	// Load NTSC palette:
	if(!palTable->loadNTSCPalette()) {
		//System.out.println("Unable to load palette file. Using default.");
		palTable->loadDefaultPalette();
	}

	// Initialize units:
	cpu->init();
	ppu->init();

	// Enable sound:
	enableSound(true);

	// Clear CPU memory:
	clearCPUMemory();
}

NES::~NES() {
	delete_n_null(cpu);
	delete_n_null(ppu);
	delete_n_null(papu);
	delete_n_null(cpuMem);
	delete_n_null(ppuMem);
	delete_n_null(sprMem);
	delete_n_null(memMapper);
	delete_n_null(rom);
	delete_n_null(palTable);
}

void NES::dumpRomMemory(ofstream* writer) {
	//ofstream writer("rom_mem_cpp.txt", ios::out|ios::binary);
	for(size_t i = 0;i<rom->rom->size(); ++i) {
		for(size_t j = 0;j<(*rom->rom)[i]->size(); ++j) {
			stringstream out;
			out << "@" << j << " " << (*(*rom->rom)[i])[j] << "\n";
			writer->write(out.str().c_str(), out.str().length());
		}
	}
	//writer.close();
	//exit(0);
}

void NES::dumpCPUMemory(ofstream* writer) {
	//ofstream writer("cpu_mem_cpp.txt", ios::out|ios::binary);
	for(size_t i = 0;i<cpuMem->mem.size(); ++i) {
		stringstream out;
		out << "-" << i << " " << cpuMem->mem[i] << "\n";
		writer->write(out.str().c_str(), out.str().length());
	}
	//writer.close();
	//exit(0);
}

bool NES::stateLoad(ByteBuffer* buf) {
	bool continueEmulation = false;
	bool success;

	// Pause emulation:
	continueEmulation = true;
	stopEmulation();

	// Check version:
	if(buf->readByte() == 1) {

		// Let units load their state from the buffer:
		cpuMem->stateLoad(buf);
		ppuMem->stateLoad(buf);
		sprMem->stateLoad(buf);
		cpu->stateLoad(buf);
		memMapper->stateLoad(buf);
		ppu->stateLoad(buf);
		success = true;

	} else {

		//System.out.println("State file has wrong format. version="+buf->readByte(0));
		success = false;

	}

	// Continue emulation:
	if(continueEmulation) {
		startEmulation();
	}

	return success;
}

void NES::stateSave(ByteBuffer* buf) {
	bool continueEmulation = isRunning();
	stopEmulation();

	// Version:
	buf->putByte(static_cast<uint16_t>(1));

	// Let units save their state:
	cpuMem->stateSave(buf);
	ppuMem->stateSave(buf);
	sprMem->stateSave(buf);
	cpu->stateSave(buf);
	memMapper->stateSave(buf);
	ppu->stateSave(buf);

	// Continue emulation:
	if(continueEmulation) {
		startEmulation();
	}

}

bool NES::isRunning() {
	return _isRunning;
}

void NES::startEmulation() {
	if(Globals::enableSound && !papu->isRunning()) {
		papu->lock_mutex();
		papu->synchronized_start();
		papu->unlock_mutex();
	}
	{
		if(rom != nullptr && rom->isValid()) {
			_isRunning = true;
		}
	}
}

void NES::stopEmulation() {
	_isRunning = false;
	cpu->stop();

	if(Globals::enableSound && papu->isRunning()) {
		papu->stop();
	}
}

void NES::clearCPUMemory() {
	uint16_t flushval = Globals::memoryFlushValue;
	for(int i = 0; i < 0x2000; ++i) {
		cpuMem->mem[i] = flushval;
	}
	for(int p = 0; p < 4; ++p) {
		int i = p * 0x800;
		cpuMem->mem[i + 0x008] = 0xF7;
		cpuMem->mem[i + 0x009] = 0xEF;
		cpuMem->mem[i + 0x00A] = 0xDF;
		cpuMem->mem[i + 0x00F] = 0xBF;
	}
}

void NES::setGameGenieState(bool enable) {
	if(memMapper != nullptr) {
		memMapper->setGameGenieState(enable);
	}
}

// Returns CPU object.
CPU* NES::getCpu() {
	return cpu;
}

// Returns PPU object.
PPU* NES::getPpu() {
	return ppu;
}

// Returns pAPU object.
PAPU* NES::getPapu() {
	return papu;
}

// Returns CPU Memory.
Memory* NES::getCpuMemory() {
	return cpuMem;
}

// Returns PPU Memory.
Memory* NES::getPpuMemory() {
	return ppuMem;
}

// Returns Sprite Memory.
Memory* NES::getSprMemory() {
	return sprMem;
}

// Returns the currently loaded ROM.
ROM* NES::getRom() {
	return rom;
}

// Returns the memory mapper.
MapperDefault* NES::getMemoryMapper() {
	return memMapper;
}

bool NES::load_rom_from_data(string rom_name, uint8_t* data, size_t length, vector<uint16_t>* save_ram) {
	// Can't load ROM while still running.
	if(_isRunning) {
		stopEmulation();
	}

	{
		// Load ROM file:

		rom = new ROM(this);
		rom->load_from_data(rom_name, data, length, save_ram);
		
		if(rom->isValid()) {

			// The CPU will load
			// the ROM into the CPU
			// and PPU memory.

			reset();

			memMapper = rom->createMapper();
			memMapper->init(this);
			cpu->setMapper(memMapper);
			memMapper->loadROM(rom);
			ppu->setMirroring(rom->getMirroringType());
		}
		return rom->isValid();
	}
}

// Resets the system.
void NES::reset() {
	if(rom != nullptr) {
		rom->closeRom();
	}
	if(memMapper != nullptr) {
		memMapper->reset();
	}

	cpuMem->reset();
	ppuMem->reset();
	sprMem->reset();
	
	clearCPUMemory();

	cpu->reset();
	cpu->init();
	ppu->reset();
	palTable->reset();
	papu->reset();

//		InputHandler* joy1 = gui->getJoy1();
//		if(joy1 != nullptr) {
//			joy1->reset();
//		}
}

// Enable or disable sound playback.
void NES::enableSound(bool enable) {
	bool wasRunning = isRunning();
	if(wasRunning) {
		stopEmulation();
	}

	if(enable) {
		papu->lock_mutex();
		papu->synchronized_start();
		papu->unlock_mutex();
	} else {
		papu->stop();
	}

	//System.out.println("** SOUND ENABLE = "+enable+" **");
	Globals::enableSound = enable;

	if(wasRunning) {
		startEmulation();
	}
}
/*
void NES::setFramerate(int rate) {
	Globals::preferredFrameRate = rate;
	Globals::frameTime = 1000000 / rate;
	
	papu->lock_mutex();
	papu->synchronized_setSampleRate(papu->getSampleRate(), false);
	papu->unlock_mutex();
}
*/


/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

NES::NES() : enable_shared_from_this<NES>() {
}

shared_ptr<NES> NES::Init(shared_ptr<InputHandler> joy1, shared_ptr<InputHandler> joy2) {
	_joy1 = joy1;
	_joy2 = joy2;

	this->_is_paused = false;
	this->_isRunning = false;

	// Create memory:
	cpuMem = make_shared<Memory>()->Init(shared_from_this(), 0x10000);	// Main memory (internal to CPU)
	ppuMem = make_shared<Memory>()->Init(shared_from_this(), 0x8000);	// VRAM memory (internal to PPU)
	sprMem = make_shared<Memory>()->Init(shared_from_this(), 0x100);	// Sprite RAM  (internal to PPU)

	// Create system units:
	cpu = make_shared<CPU>()->Init(shared_from_this());
	palTable = make_shared<PaletteTable>()->Init();
	ppu = make_shared<PPU>()->Init(shared_from_this());
	papu = make_shared<PAPU>()->Init(shared_from_this());
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

	return shared_from_this();
}

NES::~NES() {
}

void NES::dumpRomMemory(ofstream* writer) {
	//ofstream writer("rom_mem_cpp.txt", ios::out|ios::binary);
	for(size_t i = 0;i<rom->rom.size(); ++i) {
		for(size_t j = 0;j<rom->rom[i].size(); ++j) {
			stringstream out;
			out << "@" << j << " " << rom->rom[i][j] << "\n";
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
shared_ptr<CPU> NES::getCpu() {
	return cpu;
}

// Returns PPU object.
shared_ptr<PPU> NES::getPpu() {
	return ppu;
}

// Returns pAPU object.
shared_ptr<PAPU> NES::getPapu() {
	return papu;
}

// Returns CPU Memory.
shared_ptr<Memory> NES::getCpuMemory() {
	return cpuMem;
}

// Returns PPU Memory.
shared_ptr<Memory> NES::getPpuMemory() {
	return ppuMem;
}

// Returns Sprite Memory.
shared_ptr<Memory> NES::getSprMemory() {
	return sprMem;
}

// Returns the currently loaded ROM.
shared_ptr<ROM> NES::getRom() {
	return rom;
}

// Returns the memory mapper.
shared_ptr<MapperDefault> NES::getMemoryMapper() {
	return memMapper;
}

bool NES::load_rom_from_data(string rom_name, vector<uint8_t>* data, array<uint16_t, 0x2000>* save_ram) {
	// Can't load ROM while still running.
	if(_isRunning) {
		stopEmulation();
	}

	{
		// Load ROM file:

		rom = make_shared<ROM>()->Init(shared_from_this());
		rom->load_from_data(rom_name, data, save_ram);

		if(rom->isValid()) {

			// The CPU will load
			// the ROM into the CPU
			// and PPU memory.

			reset();

			memMapper = rom->createMapper();
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

//		shared_ptr<InputHandler> joy1 = gui->getJoy1();
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

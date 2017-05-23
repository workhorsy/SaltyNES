/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/


#include "SaltyNES.h"

SaltyNES::SaltyNES() {
	samplerate = 0;
	progress = 0;
	nes = nullptr;
}

SaltyNES::~SaltyNES() {
	stop();

	_rom_name.clear();
}

void SaltyNES::init() {
	initKeyCodes();
	readParams();

	Globals::memoryFlushValue = 0x00; // make SMB1 hacked version work.

	auto joy1 = make_shared<InputHandler>(0);
	auto joy2 = make_shared<InputHandler>(1);
	nes = make_shared<NES>()->Init(joy1, joy2);
	nes->enableSound(true);
	nes->reset();
}

void SaltyNES::load_rom(string rom_name, vector<uint8_t>* rom_data, array<uint16_t, 0x2000>* save_ram) {
	_rom_name = rom_name;
	nes->load_rom_from_data(rom_name, rom_data, save_ram);
}

void SaltyNES::run() {
	// Can start painting:

	if(nes->rom->isValid()) {
		// Start emulation:
		//System.out.println("vNES is now starting the processor.");
		nes->getCpu()->start();
	} else {
		// ROM file was invalid.
		printf("vNES was unable to find (%s).\n", _rom_name.c_str());
	}
}

void SaltyNES::stop() {
	if (nes) {
		nes->stopEmulation();
		//System.out.println("vNES has stopped the processor.");
		nes->getPapu()->stop();
	}
}

void SaltyNES::readParams() {
	/* Controller Setup for Player 1 */
	Globals::controls["p1_up"] = Parameters::p1_up;
	Globals::controls["p1_down"] = Parameters::p1_down;
	Globals::controls["p1_left"] = Parameters::p1_left;
	Globals::controls["p1_right"] = Parameters::p1_right;
	Globals::controls["p1_a"] = Parameters::p1_a;
	Globals::controls["p1_b"] = Parameters::p1_b;
	Globals::controls["p1_select"] = Parameters::p1_select;
	Globals::controls["p1_start"] = Parameters::p1_start;

	/* Controller Setup for Player 2 */
	Globals::controls["p2_up"] = Parameters::p2_up;
	Globals::controls["p2_down"] = Parameters::p2_down;
	Globals::controls["p2_left"] = Parameters::p2_left;
	Globals::controls["p2_right"] = Parameters::p2_right;
	Globals::controls["p2_a"] = Parameters::p2_a;
	Globals::controls["p2_b"] = Parameters::p2_b;
	Globals::controls["p2_select"] = Parameters::p2_select;
	Globals::controls["p2_start"] = Parameters::p2_start;
}

void SaltyNES::initKeyCodes() {
	Globals::keycodes["VK_SPACE"] = 32;
	Globals::keycodes["VK_PAGE_UP"] = 33;
	Globals::keycodes["VK_PAGE_DOWN"] = 34;
	Globals::keycodes["VK_END"] = 35;
	Globals::keycodes["VK_HOME"] = 36;
	Globals::keycodes["VK_DELETE"] = 127;
	Globals::keycodes["VK_INSERT"] = 155;
	Globals::keycodes["VK_LEFT"] = 37;
	Globals::keycodes["VK_UP"] = 38;
	Globals::keycodes["VK_RIGHT"] = 39;
	Globals::keycodes["VK_DOWN"] = 40;
	Globals::keycodes["VK_0"] = 48;
	Globals::keycodes["VK_1"] = 49;
	Globals::keycodes["VK_2"] = 50;
	Globals::keycodes["VK_3"] = 51;
	Globals::keycodes["VK_4"] = 52;
	Globals::keycodes["VK_5"] = 53;
	Globals::keycodes["VK_6"] = 54;
	Globals::keycodes["VK_7"] = 55;
	Globals::keycodes["VK_8"] = 56;
	Globals::keycodes["VK_9"] = 57;
	Globals::keycodes["VK_A"] = 65;
	Globals::keycodes["VK_B"] = 66;
	Globals::keycodes["VK_C"] = 67;
	Globals::keycodes["VK_D"] = 68;
	Globals::keycodes["VK_E"] = 69;
	Globals::keycodes["VK_F"] = 70;
	Globals::keycodes["VK_G"] = 71;
	Globals::keycodes["VK_H"] = 72;
	Globals::keycodes["VK_I"] = 73;
	Globals::keycodes["VK_J"] = 74;
	Globals::keycodes["VK_K"] = 75;
	Globals::keycodes["VK_L"] = 76;
	Globals::keycodes["VK_M"] = 77;
	Globals::keycodes["VK_N"] = 78;
	Globals::keycodes["VK_O"] = 79;
	Globals::keycodes["VK_P"] = 80;
	Globals::keycodes["VK_Q"] = 81;
	Globals::keycodes["VK_R"] = 82;
	Globals::keycodes["VK_S"] = 83;
	Globals::keycodes["VK_T"] = 84;
	Globals::keycodes["VK_U"] = 85;
	Globals::keycodes["VK_V"] = 86;
	Globals::keycodes["VK_W"] = 87;
	Globals::keycodes["VK_X"] = 88;
	Globals::keycodes["VK_Y"] = 89;
	Globals::keycodes["VK_Z"] = 90;
	Globals::keycodes["VK_NUMPAD0"] = 96;
	Globals::keycodes["VK_NUMPAD1"] = 97;
	Globals::keycodes["VK_NUMPAD2"] = 98;
	Globals::keycodes["VK_NUMPAD3"] = 99;
	Globals::keycodes["VK_NUMPAD4"] = 100;
	Globals::keycodes["VK_NUMPAD5"] = 101;
	Globals::keycodes["VK_NUMPAD6"] = 102;
	Globals::keycodes["VK_NUMPAD7"] = 103;
	Globals::keycodes["VK_NUMPAD8"] = 104;
	Globals::keycodes["VK_NUMPAD9"] = 105;
	Globals::keycodes["VK_MULTIPLY"] = 106;
	Globals::keycodes["VK_ADD"] = 107;
	Globals::keycodes["VK_SUBTRACT"] = 109;
	Globals::keycodes["VK_DECIMAL"] = 110;
	Globals::keycodes["VK_DIVIDE"] = 111;
	Globals::keycodes["VK_BACK_SPACE"] = 8;
	Globals::keycodes["VK_TAB"] = 9;
	Globals::keycodes["VK_ENTER"] = 10;
	Globals::keycodes["VK_SHIFT"] = 16;
	Globals::keycodes["VK_CONTROL"] = 17;
	Globals::keycodes["VK_ALT"] = 18;
	Globals::keycodes["VK_PAUSE"] = 19;
	Globals::keycodes["VK_ESCAPE"] = 27;
	Globals::keycodes["VK_OPEN_BRACKET"] = 91;
	Globals::keycodes["VK_BACK_SLASH"] = 92;
	Globals::keycodes["VK_CLOSE_BRACKET"] = 93;
	Globals::keycodes["VK_SEMICOLON"] = 59;
	Globals::keycodes["VK_QUOTE"] = 222;
	Globals::keycodes["VK_COMMA"] = 44;
	Globals::keycodes["VK_MINUS"] = 45;
	Globals::keycodes["VK_PERIOD"] = 46;
	Globals::keycodes["VK_SLASH"] = 47;
}

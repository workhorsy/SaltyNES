/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

using namespace std;

SaltyNES salty_nes;

#ifdef WEB

extern "C" int toggle_sound() {
	shared_ptr<PAPU> papu = salty_nes.nes->papu;
	papu->_is_muted = ! papu->_is_muted;
	return (papu->_is_muted ? 0 : 1);
}

void onGameDownloaded(void* userData, void* buffer, int size) {
	printf("!!! onGameDownloaded\n");

	// Run the emulator
	salty_nes.init_data((uint8_t*)buffer, size);
	salty_nes.pre_run_setup(nullptr);
	salty_nes.run();
}

void onGameFailed(void* userData) {
	printf("!!! onGameFailed\n");
}

void onMainLoop() {
	if (salty_nes.nes && ! salty_nes.nes->getCpu()->stopRunning) {
		while (! salty_nes.nes->getCpu()->emulate()) {
			// ..
		}
		if (salty_nes.nes->getCpu()->stopRunning) {
			// Clanup the SDL resources then exit
			SDL_Quit();
			emscripten_cancel_main_loop();
		}
	}
}

void runMainLoop(string file_name) {
	// Start downloading the game file
	emscripten_async_wget_data(file_name.c_str(), nullptr, onGameDownloaded, onGameFailed);

	// Run the main loop
	emscripten_set_main_loop(onMainLoop, 0, true);
}

#endif

#ifdef DESKTOP

void runMainLoop(string file_name) {
	salty_nes.init(file_name);
	salty_nes.pre_run_setup(nullptr);
	salty_nes.run();

	while (! salty_nes.nes->getCpu()->stopRunning) {
		while (! salty_nes.nes->getCpu()->emulate()) {
			// ..
		}
	}
	if (salty_nes.nes->getCpu()->stopRunning) {
		// Clanup the SDL resources then exit
		SDL_Quit();
	}
}

#endif

int main(int argc, char* argv[]) {
	printf("%s\n", "");
	printf("%s\n", "SaltyNES is a browser based NES emulator derived from vNES.");
	printf("%s\n", "SaltyNES (C) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>");
	printf("%s\n", "vNES 2.14 (C) 2006-2011 Jamie Sanders thatsanderskid.com");
	printf("%s\n", "Use of this program subject to GNU GPL, Version 3.");
	printf("%s\n", "");

	// Make sure there is a rom file name
	if (argc < 2) {
		fprintf(stderr, "No rom file argument provided. Exiting ...\n");
		return -1;
	}
	std::string file_name = argv[1];

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	// Create a SDL window
	Globals::g_window = SDL_CreateWindow(
		"SaltyNES",
		0, 0, 256, 240,
		0
	);
	if (! Globals::g_window) {
		fprintf(stderr, "Couldn't create a window: %s\n", SDL_GetError());
		return -1;
	}

	// Create a SDL renderer
	Globals::g_renderer = SDL_CreateRenderer(
		Globals::g_window,
		-1,
		SDL_RENDERER_ACCELERATED
	);
	if (! Globals::g_renderer) {
		fprintf(stderr, "Couldn't create a renderer: %s\n", SDL_GetError());
		return -1;
	}

	// Create the SDL screen
	Globals::g_screen = SDL_CreateTexture(Globals::g_renderer,
			SDL_PIXELFORMAT_BGR888, SDL_TEXTUREACCESS_STATIC, 256, 240);
	if (! Globals::g_screen) {
		fprintf(stderr, "Couldn't create a teture: %s\n", SDL_GetError());
		return -1;
	}

	runMainLoop(file_name);

	return 0;
}

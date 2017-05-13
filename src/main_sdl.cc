/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/nes_wasm
*/

#include "SaltyNES.h"

using namespace std;

vNES vnes;

#ifdef WEB

void onGameDownloaded(void* userData, void* buffer, int size) {
	printf("!!! onGameDownloaded\n");

	// Run the emulator
	vnes.init_data((uint8_t*)buffer, size);
	vnes.pre_run_setup(nullptr);
	vnes.run();
}

void onGameFailed(void* userData) {
	printf("!!! onGameFailed\n");
}

void onMainLoop() {
	if (vnes.started) {
		while (! vnes.nes->getCpu()->emulate()) {
			// ..
		}
		if (vnes.nes->getCpu()->stopRunning) {
			// Clanup the SDL resources then exit
			SDL_Quit();
			emscripten_cancel_main_loop();
		}
	} else {
		printf("!!! main loop idle ...\n");
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
	vnes.init(file_name);
	vnes.pre_run_setup(nullptr);
	vnes.run();

	while (vnes.started) {
		while (! vnes.nes->getCpu()->emulate()) {
			// ..
		}
	}
	if (vnes.nes->getCpu()->stopRunning) {
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
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n",
			SDL_GetError());
		return -1;
	}

	// Grab a SDL surface from the screen
	Globals::sdl_screen = SDL_SetVideoMode(256, 240, 32, SDL_SWSURFACE|SDL_ANYFORMAT);
	if(!Globals::sdl_screen) {
		fprintf(stderr, "Couldn't create a surface: %s\n",
			SDL_GetError());
		return -1;
	}

	runMainLoop(file_name);

	return 0;
}

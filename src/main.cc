/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"
#include <emscripten/bind.h>

using namespace std;

SaltyNES salty_nes;

#ifdef WEB


vector<uint8_t> g_game_data;

void set_game_vector_size(size_t size) {
	g_game_data.resize(size);
	std::fill(g_game_data.begin(), g_game_data.end(), 0);
}

void set_game_vector_data(size_t index, uint8_t data) {
	g_game_data[index] = data;
}

void start_emu() {
	// Run the emulator
	salty_nes.init_data(g_game_data.data(), g_game_data.size());
	salty_nes.pre_run_setup(nullptr);
	salty_nes.run();
}

EMSCRIPTEN_BINDINGS(Wrappers) {
	emscripten::function("set_game_vector_size", &set_game_vector_size);
	emscripten::function("set_game_vector_data", &set_game_vector_data);
	emscripten::function("start_emu", &start_emu);
};

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

void runMainLoop() {
	emscripten_set_main_loop(onMainLoop, 0, true);
}

extern "C" int toggle_sound() {
	shared_ptr<PAPU> papu = salty_nes.nes->papu;
	papu->_is_muted = ! papu->_is_muted;
	return (papu->_is_muted ? 0 : 1);
}

#endif

#ifdef DESKTOP

void runMainLoop() {
	salty_nes.init("");
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

int main() {
	printf("%s\n", "");
	printf("%s\n", "SaltyNES is a NES emulator in WebAssembly");
	printf("%s\n", "SaltyNES (C) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>");
	printf("%s\n", "vNES 2.14 (C) 2006-2011 Jamie Sanders thatsanderskid.com");
	printf("%s\n", "This program is licensed under GPLV3 or later");
	printf("%s\n", "");

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

	runMainLoop();
	return 0;
}

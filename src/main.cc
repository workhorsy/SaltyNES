/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

using namespace std;

SaltyNES salty_nes;
vector<uint8_t> g_game_data;
string g_game_file_name;

bool toggle_sound() {
	shared_ptr<PAPU> papu = salty_nes.nes->papu;
	papu->_is_muted = ! papu->_is_muted;
	return !papu->_is_muted;
}

void start_emu() {
	// Run the emulator
	salty_nes.init();
	salty_nes.load_rom(g_game_file_name, &g_game_data, nullptr);
	salty_nes.run();
}

void set_game_vector_size(size_t size) {
	g_game_data.resize(size);
	std::fill(g_game_data.begin(), g_game_data.end(), 0);
}

void set_game_vector_data(size_t index, uint8_t data) {
	g_game_data[index] = data;
}

void set_game_vector_data_from_file(string file_name) {
	ifstream reader(file_name.c_str(), ios::in|ios::binary);
	if(reader.fail()) {
		fprintf(stderr, "Error while loading rom '%s': %s\n", file_name.c_str(), strerror(errno));
		exit(1);
	}

	reader.seekg(0, ios::end);
	size_t length = reader.tellg();
	reader.seekg(0, ios::beg);
	assert(length > 0);
	g_game_data.resize(length);
	reader.read(reinterpret_cast<char*>(g_game_data.data()), g_game_data.size());
	reader.close();
	g_game_file_name = file_name;
}

#ifdef WEB

EMSCRIPTEN_BINDINGS(Wrappers) {
	emscripten::function("set_game_vector_size", &set_game_vector_size);
	emscripten::function("set_game_vector_data", &set_game_vector_data);
	emscripten::function("start_emu", &start_emu);
	emscripten::function("toggle_sound", &toggle_sound);
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
	// Tell the web app that everything is loaded
	EM_ASM_ARGS({
		onReady();
	}, 0);

	emscripten_set_main_loop(onMainLoop, 0, true);
}

#endif

#ifdef DESKTOP

void runMainLoop() {
	salty_nes.init();
	salty_nes.load_rom(g_game_file_name, &g_game_data, nullptr);
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
	printf("%s\n", "SaltyNES is a NES emulator in WebAssembly");
	printf("%s\n", "SaltyNES (C) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>");
	printf("%s\n", "vNES 2.14 (C) 2006-2011 Jamie Sanders thatsanderskid.com");
	printf("%s\n", "This program is licensed under GPLV3 or later");
	printf("%s\n", "https://github.com/workhorsy/SaltyNES");
	printf("%s\n", "");

	// Make sure there is a rom file name
	#ifdef DESKTOP
		if (argc < 2) {
			fprintf(stderr, "No rom file argument provided. Exiting ...\n");
			return -1;
		}
		set_game_vector_data_from_file(argv[1]);
	#endif
	#ifdef WEB
		g_game_file_name = "rom_from_browser.nes";
	#endif

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

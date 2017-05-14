/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/


#include "SaltyNES.h"

const float InputHandler::AXES_DEAD_ZONE = 0.2;
const string InputHandler::KEYS[] = { "up", "down", "right", "left", "start", "select", "a", "b" };
const size_t InputHandler::KEYS_LENGTH = 8;
bool InputHandler::_is_configuring_gamepad = false;
string InputHandler::_configuring_gamepad_button = "";

InputHandler::InputHandler(int id) :
	_id(id),
	_keys(255),
	_map(InputHandler::NUM_KEYS) {

	_is_gamepad_connected = false;
	_is_gamepad_used = false;
	_is_keyboard_used = false;

	_is_input_pressed["up"] = false;
	_is_input_pressed["down"] = false;
	_is_input_pressed["right"] = false;
	_is_input_pressed["left"] = false;
	_is_input_pressed["start"] = false;
	_is_input_pressed["select"] = false;
	_is_input_pressed["a"] = false;
	_is_input_pressed["b"] = false;
}

InputHandler::~InputHandler() {

}

uint16_t InputHandler::getKeyState(int padKey) {
	return static_cast<uint16_t>(_keys[_map[padKey]] ? 0x41 : 0x40);
}

void InputHandler::mapKey(int padKey, int kbKeycode) {
	_map[padKey] = kbKeycode;
}

void InputHandler::poll_for_key_events() {
#ifdef WEB
	const bool is_left = EM_ASM_INT({ return g_get_key_left; }, 0) > 0;
	const bool is_right = EM_ASM_INT({ return g_get_key_right; }, 0) > 0;
	const bool is_up = EM_ASM_INT({ return g_get_key_up; }, 0) > 0;
	const bool is_down = EM_ASM_INT({ return g_get_key_down; }, 0) > 0;
	const bool is_b = EM_ASM_INT({ return g_get_key_b; }, 0) > 0;
	const bool is_a = EM_ASM_INT({ return g_get_key_a; }, 0) > 0;
	const bool is_start = EM_ASM_INT({ return g_get_key_start; }, 0) > 0;
	const bool is_select = EM_ASM_INT({ return g_get_key_select; }, 0) > 0;

	_keys[_map[InputHandler::KEY_UP]] =     is_up;
	_keys[_map[InputHandler::KEY_DOWN]] =   is_down;
	_keys[_map[InputHandler::KEY_RIGHT]] =  is_right;
	_keys[_map[InputHandler::KEY_LEFT]] =   is_left;
	_keys[_map[InputHandler::KEY_START]] =  is_start;
	_keys[_map[InputHandler::KEY_SELECT]] = is_select;
	_keys[_map[InputHandler::KEY_B]] =      is_b;
	_keys[_map[InputHandler::KEY_A]] =      is_a;
#endif

#ifdef DESKTOP
	int numberOfKeys;
	uint8_t* keystate = SDL_GetKeyState(&numberOfKeys);

	_keys[_map[InputHandler::KEY_UP]] =     keystate[SDLK_w];
	_keys[_map[InputHandler::KEY_DOWN]] =   keystate[SDLK_s];
	_keys[_map[InputHandler::KEY_RIGHT]] =  keystate[SDLK_d];
	_keys[_map[InputHandler::KEY_LEFT]] =   keystate[SDLK_a];
	_keys[_map[InputHandler::KEY_START]] =  keystate[SDLK_RETURN];
	_keys[_map[InputHandler::KEY_SELECT]] = keystate[SDLK_RSHIFT];
	_keys[_map[InputHandler::KEY_B]] =      keystate[SDLK_j];
	_keys[_map[InputHandler::KEY_A]] =      keystate[SDLK_k];
#endif

	// Can't hold both left & right or up & down at same time:
	if(_keys[_map[InputHandler::KEY_LEFT]]) {
		_keys[_map[InputHandler::KEY_RIGHT]] = false;
	} else if(_keys[_map[InputHandler::KEY_RIGHT]]) {
		_keys[_map[InputHandler::KEY_LEFT]] = false;
	}

	if(_keys[_map[InputHandler::KEY_UP]]) {
		_keys[_map[InputHandler::KEY_DOWN]] = false;
	} else if(_keys[_map[InputHandler::KEY_DOWN]]) {
		_keys[_map[InputHandler::KEY_UP]] = false;
	}
}

void InputHandler::reset() {
	size_t size = _keys.size();
	_keys.clear();
	_keys.resize(size);
}

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
	_map(InputHandler::NUM_KEYS), enable_shared_from_this<InputHandler>() {

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
	// Check for keyboard input
	int numberOfKeys;
	const uint8_t* keystate = SDL_GetKeyboardState(&numberOfKeys);

	_keys[_map[InputHandler::KEY_UP]] =     keystate[SDL_SCANCODE_W];
	_keys[_map[InputHandler::KEY_DOWN]] =   keystate[SDL_SCANCODE_S];
	_keys[_map[InputHandler::KEY_RIGHT]] =  keystate[SDL_SCANCODE_D];
	_keys[_map[InputHandler::KEY_LEFT]] =   keystate[SDL_SCANCODE_A];
	_keys[_map[InputHandler::KEY_START]] =  keystate[SDL_SCANCODE_RETURN];
	_keys[_map[InputHandler::KEY_SELECT]] = keystate[SDL_SCANCODE_RSHIFT];
	_keys[_map[InputHandler::KEY_B]] =      keystate[SDL_SCANCODE_J];
	_keys[_map[InputHandler::KEY_A]] =      keystate[SDL_SCANCODE_K];

	bool is_using_keyboard = (
		keystate[SDL_SCANCODE_W] |
		keystate[SDL_SCANCODE_S] |
		keystate[SDL_SCANCODE_D] |
		keystate[SDL_SCANCODE_A] |
		keystate[SDL_SCANCODE_RETURN] |
		keystate[SDL_SCANCODE_RSHIFT] |
		keystate[SDL_SCANCODE_J] |
		keystate[SDL_SCANCODE_K]) != 0;

	// Check for gamepad input
	if (! is_using_keyboard) {
		for (auto const& pair : Globals::joysticks) {
			int id = pair.first;
			SDL_Joystick* joy = pair.second;
			//printf("????????????? joy id: %d\n", id);

			if (joy != nullptr && SDL_JoystickGetAttached(joy)) {
				//printf("????????????? joy attached i: %d\n", SDL_JoystickGetAttached(joy));

				if (Globals::is_windows) {
					_keys[_map[InputHandler::KEY_START]] = SDL_JoystickGetButton(joy, 9);
					_keys[_map[InputHandler::KEY_SELECT]] = SDL_JoystickGetButton(joy, 8);
					_keys[_map[InputHandler::KEY_B]] = SDL_JoystickGetButton(joy, 0);
					_keys[_map[InputHandler::KEY_A]] = SDL_JoystickGetButton(joy, 1);
					_keys[_map[InputHandler::KEY_UP]] = SDL_JoystickGetButton(joy, 12);
					_keys[_map[InputHandler::KEY_DOWN]] = SDL_JoystickGetButton(joy, 13);
					_keys[_map[InputHandler::KEY_RIGHT]] = SDL_JoystickGetButton(joy, 15);
					_keys[_map[InputHandler::KEY_LEFT]] = SDL_JoystickGetButton(joy, 14);
				} else {
					_keys[_map[InputHandler::KEY_START]] = SDL_JoystickGetButton(joy, 7);
					_keys[_map[InputHandler::KEY_SELECT]] = SDL_JoystickGetButton(joy, 6);
					_keys[_map[InputHandler::KEY_B]] = SDL_JoystickGetButton(joy, 0);
					_keys[_map[InputHandler::KEY_A]] = SDL_JoystickGetButton(joy, 1);
					_keys[_map[InputHandler::KEY_UP]] = SDL_JoystickGetButton(joy, 13);
					_keys[_map[InputHandler::KEY_DOWN]] = SDL_JoystickGetButton(joy, 14);
					_keys[_map[InputHandler::KEY_RIGHT]] = SDL_JoystickGetButton(joy, 12);
					_keys[_map[InputHandler::KEY_LEFT]] = SDL_JoystickGetButton(joy, 11);
				}
			}
		}
	}

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

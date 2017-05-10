/*
SaltyNES Copyright (c) 2012-2014 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>

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

#ifdef NACL

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <map>
#include <sstream>
#include <pthread.h>


#include <cassert>
#include <cmath>
#include <cstring>
#include <string>
#include <sys/time.h>
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_array_buffer.h"

#include "SaltyNES.h"

using namespace std;

// Globals
SaltyNES* SaltyNES::g_salty_nes = nullptr;
const int kPthreadMutexSuccess = 0;

// This is called by the browser when the 2D context has been flushed to the
// browser window.
void FlushCallback(void* data, int32_t result) {
	static_cast<SaltyNES*>(data)->set_flush_pending(false);
}

// A small helper RAII class that implementes a scoped pthread_mutex lock.
class ScopedMutexLock {
	pthread_mutex_t* mutex_; // Weak reference.

public:
	explicit ScopedMutexLock(pthread_mutex_t* mutex) : 
		mutex_(mutex) {
		if(pthread_mutex_lock(mutex_) != kPthreadMutexSuccess) {
			mutex_ = nullptr;
		}
	}
	
	~ScopedMutexLock() {
		if(mutex_)
			pthread_mutex_unlock(mutex_);
	}
	
	bool is_valid() const {
		return mutex_ != nullptr;
	}
};

// A small helper RAII class used to acquire and release the pixel lock.
class ScopedPixelLock {
	SaltyNES* image_owner_;	// Weak reference.
	uint32_t* pixels_;	// Weak reference.

	ScopedPixelLock();	// Not implemented, do not use.

public:
	explicit ScopedPixelLock(SaltyNES* image_owner) : 
		image_owner_(image_owner), 
		pixels_(image_owner->LockPixels()) {
	}

	~ScopedPixelLock() {
		pixels_ = nullptr;
		image_owner_->UnlockPixels();
	}

	uint32_t* pixels() const {
		return pixels_;
	}
};

SaltyNES::SaltyNES(PP_Instance instance)
		: pp::Instance(instance),
			graphics_2d_context_(nullptr),
			pixel_buffer_(nullptr),
			flush_pending_(false),
			quit_(false),
			thread_(0),
			thread_is_running_(false) {
	pthread_mutex_init(&pixel_buffer_mutex_, nullptr);
	vnes = nullptr;
	
	// Create the gamepads
	_joy1 = new InputHandler(0);
	_joy2 = new InputHandler(1);

	if(SaltyNES::g_salty_nes == nullptr)
		SaltyNES::g_salty_nes = this;

	// Setup nacl so it can use gamepads
	pp::Module* module = pp::Module::Get();
	assert(module);
	gamepad_ = static_cast<const PPB_Gamepad*>(
		module->GetBrowserInterface(PPB_GAMEPAD_INTERFACE));
	assert(gamepad_);
	
	// Poll gamepad 1 to populate its info
	_joy1->_is_gamepad_used = true;
	this->poll_gamepad();
}

SaltyNES::~SaltyNES() {
	quit_ = true;

	if(vnes) {
		vnes->stop();
		if(thread_is_running_) {
			pthread_join(thread_, nullptr);
			thread_is_running_ = false;
		}
		delete_n_null(vnes);
	}
	
	DestroyContext();
	delete_n_null(pixel_buffer_);
	pthread_mutex_destroy(&pixel_buffer_mutex_);
}

void SaltyNES::DidChangeView(const pp::View& view) {
	pp::Rect position = view.GetRect();
	if(pixel_buffer_ && position.size() == pixel_buffer_->size())
		return;	// Size didn't change, no need to update anything.

	// Create a new device context with the new size.
	DestroyContext();
	CreateContext(position.size());
	// Delete the old pixel buffer and create a new one.
	ScopedMutexLock scoped_mutex(&pixel_buffer_mutex_);
	delete_n_null(pixel_buffer_);
	if(graphics_2d_context_ != nullptr) {
		pixel_buffer_ = new pp::ImageData(
			this,
			PP_IMAGEDATAFORMAT_RGBA_PREMUL,
			graphics_2d_context_->size(),
			false
		);
	}
}

bool SaltyNES::Init(uint32_t argc, const char* argn[], const char* argv[]) {
	return true;
}

void SaltyNES::HandleMessage(const pp::Var& var_message) {
	string message;
	if(var_message.is_string())
		message = var_message.AsString();

	// Handle messages that work with vnes running or not
	if(message.find("load_rom:") == 0) {
		// Convert the rom from base64 to bytes
		size_t rom_pos = message.find(" rom:");
		string rom_base64 = message.substr(rom_pos + 5);
		string rom_data = base64_decode(rom_base64);

		// Convert the save from hex to uint16_t vector
		vector<uint16_t>* saveRam = nullptr;
		if(rom_pos > 32768) {
			size_t save_pos = message.find("load_rom:");
			string save_data = message.substr(save_pos + 9, 32768);
			saveRam = Misc::from_hex_string_to_vector(save_data);
		}

		// Make sure the ROM is valid
		if(rom_data[0] != 'N' || rom_data[1] != 'E' || rom_data[2] != 'S' || rom_data[3] != 0x1A) {
			log_to_browser("Invalid ROM file!");
		} else {
			// Stop any previously running NES
			if(vnes) {
				vnes->stop();
				if(thread_) {
					pthread_join(thread_, nullptr);
					thread_is_running_ = false;
				}
				delete_n_null(vnes);
			}

			// Run the ROM
			vnes = new vNES();
			vnes->init_data((uint8_t*) rom_data.c_str(), static_cast<size_t>(rom_data.length()), this);
			vnes->pre_run_setup(saveRam);
			log_to_browser("running");
			pthread_create(&thread_, nullptr, start_main_loop, this);
			thread_is_running_ = true;
		}
		
		return;
	// Start configuring gamepad keys
	} else if(message.find("start_configure_key:") == 0) {
		size_t sep_pos = message.find_first_of(":");
		string button = message.substr(sep_pos + 1);

		InputHandler::_is_configuring_gamepad = true;
		InputHandler::_configuring_gamepad_button = "";
		
		// Remove all the previous keys
		_joy1->_input_map_button[button].clear();
		_joy1->_input_map_axes_pos[button].clear();
		_joy1->_input_map_axes_neg[button].clear();

		return;
	// Get configured gamepad keys
	} else if(message == "get_configure_key") {
		PP_GamepadsSampleData gamepad_data;
		gamepad_->Sample(pp_instance(), &gamepad_data);
		_joy1->update_gamepad(gamepad_data);
		
		stringstream out;
		out << "get_configure_key:" << InputHandler::_configuring_gamepad_button;
		log_to_browser(out.str());

		return;
	// End configuring gamepad keys
	} else if(message == "end_configure_key") {
		InputHandler::_is_configuring_gamepad = false;

		return;
	} else if(message == "get_gamepad_status") {
		// Get current gamepad data.
		PP_GamepadsSampleData gamepad_data;
		gamepad_->Sample(pp_instance(), &gamepad_data);
		_joy1->update_gamepad(gamepad_data);
		
		_joy1->_is_gamepad_connected = false;
		for(size_t i=0; i<gamepad_data.length; ++i) {
			if(gamepad_data.items[i].connected)
				_joy1->_is_gamepad_connected = true;
		}
	
		// Tell the browser if the gamepad is connected or not
		stringstream out;
		out << "get_gamepad_status:";
		out << (_joy1->_is_gamepad_connected ? "yes" : "no");
		out << ":" << _joy1->_gamepad_vendor_id << _joy1->_gamepad_product_id;
		log_to_browser(out.str());
		return;
	} else if(message.find("key_down:") == 0) {
		size_t sep_pos = message.find_first_of(":");
		uint32_t button = 0;
		string str_button = message.substr(sep_pos + 1);
		istringstream(str_button) >> button;

		_joy1->key_down(button);
		return;
	} else if(message.find("key_up:") == 0) {
		size_t sep_pos = message.find_first_of(":");
		uint32_t button = 0;
		string str_button = message.substr(sep_pos + 1);
		istringstream(str_button) >> button;

		_joy1->key_up(button);
		return;
	} else if(message.find("set_input_") == 0) {
		size_t sep_pos = message.find_first_of(":");
		size_t axes_pos = message.find("axes");
		size_t sign_pos = message.find_first_of("+-");
		string input = message.substr(10, sep_pos-10);

		// Axes
		if(axes_pos != string::npos && sign_pos != string::npos) {
			string str_axes = message.substr(sign_pos + 1);
			string sign = message.substr(sign_pos, 1);
			size_t axes = 0;
			istringstream(str_axes) >> axes;
			
			if(sign == "+") {
				_joy1->_input_map_axes_pos[input].push_back(axes);
			} else if(sign == "-") {
				_joy1->_input_map_axes_neg[input].push_back(axes);
			}
		// Button
		} else {
			string str_button = message.substr(sep_pos + 1);
			int button = 0;
			istringstream(str_button) >> button;
			_joy1->_input_map_button[input].push_back(button);
		}
		return;
	}

	// Just return if any messages require vnes, but it is not running
	if(vnes == nullptr) return;

	// Handle messages that require vnes running
	if(message == "paint") {
		if(!vnes->nes->_is_paused)
			Paint();
	} else if(message == "pause") {
		vnes->nes->_is_paused = !vnes->nes->_is_paused;
	} else if(message.find("zoom:") == 0) {
		size_t sep_pos = message.find_first_of(":");
		uint32_t zoom = 0;
		string str_zoom = message.substr(sep_pos + 1);
		istringstream(str_zoom) >> zoom;
		vnes->nes->ppu->_zoom = zoom;
	} else if(message == "get_fps") {
		stringstream out;
		out << "get_fps:";
		out << get_fps();
		log_to_browser(out.str());
	} else if(message == "quit") {
		if(vnes != nullptr) {
			vnes->stop();
			if(thread_is_running_) {
				pthread_join(thread_, nullptr);
				thread_is_running_ = false;
			}
			delete_n_null(vnes);
		}
		log_to_browser("quit");
	} else {
		stringstream out;
		out << "unknown message:";
		out << message;
		log_to_browser(out.str());
	}

}

uint32_t* SaltyNES::LockPixels() {
	void* pixels = nullptr;
	// Do not use a ScopedMutexLock here, since the lock needs to be held until
	// the matching UnlockPixels() call.
	if(pthread_mutex_lock(&pixel_buffer_mutex_) == kPthreadMutexSuccess) {
		if(pixel_buffer_ != nullptr && !pixel_buffer_->is_null()) {
			pixels = pixel_buffer_->data();
		}
	}
	return reinterpret_cast<uint32_t*>(pixels);
}

void SaltyNES::UnlockPixels() const {
	pthread_mutex_unlock(&pixel_buffer_mutex_);
}

void SaltyNES::Paint() {
	ScopedMutexLock scoped_mutex(&pixel_buffer_mutex_);
	if(!scoped_mutex.is_valid()) {
		return;
	}

	this->poll_gamepad();
	
	FlushPixelBuffer();
}

void SaltyNES::poll_gamepad() {
	PP_GamepadsSampleData gamepad_data;
	gamepad_->Sample(pp_instance(), &gamepad_data);
	_joy1->update_gamepad(gamepad_data);
}

void SaltyNES::CreateContext(const pp::Size& size) {
	ScopedMutexLock scoped_mutex(&pixel_buffer_mutex_);
	if(!scoped_mutex.is_valid()) {
		return;
	}
	if(IsContextValid())
		return;
	graphics_2d_context_ = new pp::Graphics2D(this, size, false);
	if(!BindGraphics(*graphics_2d_context_)) {
		printf("Couldn't bind the device context\n");
	}
}

void SaltyNES::DestroyContext() {
	ScopedMutexLock scoped_mutex(&pixel_buffer_mutex_);
	if(!scoped_mutex.is_valid()) {
		return;
	}
	if(!IsContextValid())
		return;
	delete_n_null(graphics_2d_context_);
}

void SaltyNES::FlushPixelBuffer() {
	if(!IsContextValid())
		return;
	// Note that the pixel lock is held while the buffer is copied into the
	// device context and then flushed.
	graphics_2d_context_->PaintImageData(*pixel_buffer_, pp::Point());
	if(flush_pending())
		return;
	set_flush_pending(true);
	graphics_2d_context_->Flush(pp::CompletionCallback(&FlushCallback, this));
}

void* SaltyNES::start_main_loop(void* param) {
	log_to_browser("start_main_loop");

	SaltyNES* salty_nes = reinterpret_cast<SaltyNES*>(param);
	salty_nes->vnes->run();
	return 0;
}

void SaltyNES::log(string message) {
	PostMessage(message);
}

#endif


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

#ifdef SDL
SDL_Surface* Globals::sdl_screen = nullptr;
#endif

double Globals::CPU_FREQ_NTSC = 1789772.5;
double Globals::CPU_FREQ_PAL = 1773447.4;
int Globals::preferredFrameRate = 60;

// Microseconds per frame:
const double Globals::MS_PER_FRAME = 1000000.0 / 60.0;
// What value to flush memory with on power-up:
uint16_t Globals::memoryFlushValue = 0xFF;

bool Globals::disableSprites = false;
bool Globals::palEmulation = false;
bool Globals::enableSound = true;

std::map<string, uint32_t> Globals::keycodes; //Java key codes
std::map<string, string> Globals::controls; //vNES controls codes


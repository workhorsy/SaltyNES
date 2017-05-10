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

const size_t PPU::UNDER_SCAN = 8;

vector<int>* PPU::get_screen_buffer() {
	return &_screen_buffer;
}

vector<int>* PPU::get_pattern_buffer() {
	return nullptr;
}

vector<int>* PPU::get_name_buffer() {
	return nullptr;
}

vector<int>* PPU::get_img_palette_buffer() {
	return nullptr;
}

vector<int>* PPU::get_spr_palette_buffer() {
	return nullptr;
}

PPU::PPU(NES* nes) {
	this->nes = nes;
	_zoom = 1;
	_frame_start.tv_usec = 0;
	_frame_start.tv_sec = 0;
	_frame_end.tv_usec = 0;
	_frame_end.tv_sec = 0;
	_ticks_since_second = 0.0;
	frameCounter = 0;
	ppuMem = nullptr;
	sprMem = nullptr;

	// Rendering Options:
	showSpr0Hit = false;

	// Control Flags Register 1:
	f_nmiOnVblank = 0;
	f_spriteSize = 0;
	f_bgPatternTable = 0;
	f_spPatternTable = 0;
	f_addrInc = 0;
	f_nTblAddress = 0;

	// Control Flags Register 2:
	f_color = 0;
	f_spVisibility = 0;
	f_bgVisibility = 0;
	f_spClipping = 0;
	f_bgClipping = 0;
	f_dispType = 0;
	
	// Status flags:
	STATUS_VRAMWRITE = 4;
	STATUS_SLSPRITECOUNT = 5;
	STATUS_SPRITE0HIT = 6;
	STATUS_VBLANK = 7;
	
	// VRAM I/O:
	vramAddress = 0;
	vramTmpAddress = 0;
	vramBufferedReadValue = 0;
	firstWrite = true;
	vramMirrorTable = vector<int>(0x8000, 0);
	i = 0;

	// SPR-RAM I/O:
	sramAddress = 0;

	// Counters:
	cntFV = 0;
	cntV = 0;
	cntH = 0;
	cntVT = 0;
	cntHT = 0;

	// Registers:
	regFV = 0;
	regV = 0;
	regH = 0;
	regVT = 0;
	regHT = 0;
	regFH = 0;
	regS = 0;

	// VBlank extension for PAL emulation:
	vblankAdd = 0;
	curX = 0;
	scanline = 0;
	lastRenderedScanline = 0;
	mapperIrqCounter = 0;

	// Sprite data:
	sprX = vector<int>(64, 0);
	sprY = vector<int>(64, 0);
	sprTile = vector<int>(64, 0);
	sprCol = vector<int>(64, 0);
	vertFlip = vector<bool>(64, false);
	horiFlip = vector<bool>(64, false);
	bgPriority = vector<bool>(64, false);
	spr0HitX = 0;
	spr0HitY = 0;
	hitSpr0 = false;

	// Tiles:
	ptTile = vector<Tile*>(512, nullptr);
	// Name table data:
	ntable1 = vector<int>(4, 0);
	nameTable = vector<NameTable*>(4, nullptr);
	currentMirroring = -1;

	// Palette data:
	sprPalette = vector<int>(16, 0);
	imgPalette = vector<int>(16, 0);

	// Misc:
	scanlineAlreadyRendered = false;
	requestEndFrame = false;
	nmiOk = false;
	nmiCounter = 0;
	tmp = 0;
	dummyCycleToggle = false;

	// Vars used when updating regs/address:
	address = 0;
	b1 = 0;
	b2 = 0;

	// Variables used when rendering:
	attrib = vector<int>(32, 0);
	bgbuffer = vector<int>(256 * 240, 0);
	pixrendered = vector<int>(256 * 240, 0);
	//dummyPixPriTable = vector<int>(256 * 240, 0);
	tpix = nullptr;
	requestRenderAll = false;
	validTileData = false;
	att = 0;
	scantile = vector<Tile*>(32, nullptr);
	t = nullptr;

	// These are temporary variables used in rendering and sound procedures.
	// Their states outside of those procedures can be ignored.
	curNt = 0;
	destIndex = 0;
	x = 0;
	y = 0;
	sx = 0;
	si = 0;
	ei = 0;
	tile = 0;
	col = 0;
	baseTile = 0;
	tscanoffset = 0;
	srcy1 = 0;
	srcy2 = 0;
	bufferSize = 0;
	available = 0;
	cycles = 0;
	_screen_buffer = vector<int>(256 * 240, 0);
}

PPU::~PPU() {
	for(size_t i = 0; i < nameTable.size(); ++i) {
		delete_n_null(nameTable[i]);
	}

	nes = nullptr;
	ppuMem = nullptr;
	sprMem = nullptr;
}

void PPU::init() {
	// Get the memory:
	ppuMem = nes->getPpuMemory();
	sprMem = nes->getSprMemory();

	updateControlReg1(0);
	updateControlReg2(0);

	// Initialize misc vars:
	scanline = 0;

	// Create pattern table tile buffers:
	for(size_t i = 0; i < ptTile.size(); ++i) {
		ptTile[i] = new Tile();
	}

	// Create nametable buffers:
	for(size_t i = 0; i < nameTable.size(); ++i) {
		stringstream name;
		name << "Nt" << i;
		nameTable[i] = new NameTable(32, 32, name.str());
	}

	// Initialize mirroring lookup table:
	for(size_t i = 0; i < 0x8000; ++i) {
		vramMirrorTable[i] = i;
	}

	lastRenderedScanline = -1;
	curX = 0;
}

// Sets Nametable mirroring.
void PPU::setMirroring(int mirroring) {
	if(mirroring == currentMirroring) {
		return;
	}

	currentMirroring = mirroring;
	triggerRendering();

	// Remove mirroring:
	for(size_t i = 0; i < 0x8000; ++i) {
		vramMirrorTable[i] = i;
	}

	// Palette mirroring:
	defineMirrorRegion(0x3f20, 0x3f00, 0x20);
	defineMirrorRegion(0x3f40, 0x3f00, 0x20);
	defineMirrorRegion(0x3f80, 0x3f00, 0x20);
	defineMirrorRegion(0x3fc0, 0x3f00, 0x20);

	// Additional mirroring:
	defineMirrorRegion(0x3000, 0x2000, 0xf00);
	defineMirrorRegion(0x4000, 0x0000, 0x4000);

	if(mirroring == ROM::HORIZONTAL_MIRRORING) {


		// Horizontal mirroring.

		ntable1[0] = 0;
		ntable1[1] = 0;
		ntable1[2] = 1;
		ntable1[3] = 1;

		defineMirrorRegion(0x2400, 0x2000, 0x400);
		defineMirrorRegion(0x2c00, 0x2800, 0x400);

	} else if(mirroring == ROM::VERTICAL_MIRRORING) {

		// Vertical mirroring.

		ntable1[0] = 0;
		ntable1[1] = 1;
		ntable1[2] = 0;
		ntable1[3] = 1;

		defineMirrorRegion(0x2800, 0x2000, 0x400);
		defineMirrorRegion(0x2c00, 0x2400, 0x400);

	} else if(mirroring == ROM::SINGLESCREEN_MIRRORING) {

		// Single Screen mirroring

		ntable1[0] = 0;
		ntable1[1] = 0;
		ntable1[2] = 0;
		ntable1[3] = 0;

		defineMirrorRegion(0x2400, 0x2000, 0x400);
		defineMirrorRegion(0x2800, 0x2000, 0x400);
		defineMirrorRegion(0x2c00, 0x2000, 0x400);

	} else if(mirroring == ROM::SINGLESCREEN_MIRRORING2) {


		ntable1[0] = 1;
		ntable1[1] = 1;
		ntable1[2] = 1;
		ntable1[3] = 1;

		defineMirrorRegion(0x2400, 0x2400, 0x400);
		defineMirrorRegion(0x2800, 0x2400, 0x400);
		defineMirrorRegion(0x2c00, 0x2400, 0x400);

	} else {

		// Assume Four-screen mirroring.

		ntable1[0] = 0;
		ntable1[1] = 1;
		ntable1[2] = 2;
		ntable1[3] = 3;

	}

}


// Define a mirrored area in the address lookup table.
// Assumes the regions don't overlap.
// The 'to' region is the region that is physically in memory.
void PPU::defineMirrorRegion(size_t fromStart, size_t toStart, size_t size) {
	for(size_t i = 0; i < size; ++i) {
		vramMirrorTable[fromStart + i] = toStart + i;
	}
}

// Emulates PPU cycles
void PPU::emulateCycles() {
	//int n = (!requestEndFrame && curX+cycles<341 && (scanline-20 < spr0HitY || scanline-22 > spr0HitY))?cycles:1;
	for(; cycles > 0; --cycles) {

		if(scanline - 21 == spr0HitY) {

			if((curX == spr0HitX) && (f_spVisibility == 1)) {
				// Set sprite 0 hit flag:
				setStatusFlag(STATUS_SPRITE0HIT, true);
			}

		}

		if(requestEndFrame) {
			--nmiCounter;
			if(nmiCounter == 0) {
				requestEndFrame = false;
				startVBlank();
			}
		}

		++curX;
		if(curX == 341) {

			curX = 0;
			endScanline();
		}
	}
}

void PPU::startVBlank() {
	// Start VBlank period:

	// Do NMI:
	nes->getCpu()->requestIrq(CPU::IRQ_NMI);

	// Make sure everything is rendered:
	if(lastRenderedScanline < 239) {
		renderFramePartially(lastRenderedScanline + 1, 240 - lastRenderedScanline);
	}

	endFrame();

	nes->papu->writeBuffer();
#ifdef NACL
//		log_to_browser("startVBlank");
		int color32;
		int zoomed_x = 0, zoomed_y = 0;
		int zoom = _zoom;
		uint32_t* pixel_bits = nes->_salty_nes->LockPixels();
		
		if(is_safe_to_paint()) {
			// Each column
			for(size_t y=UNDER_SCAN; y<240-UNDER_SCAN; ++y) {
				// Each row
				for(size_t x=UNDER_SCAN; x<256-UNDER_SCAN; ++x) {
					color32 = _screen_buffer[x + (y * (256))];
					color32 |= (0xFF << 24); // Add full alpha
					
					// Each pixel zoomed
					for(size_t y_offset=0; y_offset<zoom; ++y_offset) {
						zoomed_y = (y * zoom) + y_offset;
						for(size_t x_offset=0; x_offset<zoom; ++x_offset) {
							zoomed_x = (x * zoom) + x_offset;
							pixel_bits[zoomed_x + (zoomed_y * (256 * zoom))] = color32;
						}
					}
				}
			}
		}
		
		nes->_salty_nes->UnlockPixels();
#endif
#ifdef SDL
	// Lock the screen, if needed
	if(SDL_MUSTLOCK(Globals::sdl_screen)) {
		if(SDL_LockSurface(Globals::sdl_screen) < 0)
			return;
	}

	// Actually draw the screen
	uint8_t r = 0, g = 0, b = 0;
	int color;
	int color32;
	int* pixels = reinterpret_cast<int*>(Globals::sdl_screen->pixels);
	for(size_t y=UNDER_SCAN; y<240-UNDER_SCAN; ++y) {
		for(size_t x=UNDER_SCAN; x<256-UNDER_SCAN; ++x) {
			color32 = _screen_buffer[x + (y * (256))];
			b = (color32 >> 16) & 0x000000FF;
			g = (color32 >> 8) & 0x000000FF;
			r = (color32 >> 0) & 0x000000FF;
			
			color = SDL_MapRGB(Globals::sdl_screen->format, r, g, b);
			pixels[x + (y * (256))] = color;
		}
	}

	// Unlock the screen if needed
	if(SDL_MUSTLOCK(Globals::sdl_screen)) {
		SDL_UnlockSurface(Globals::sdl_screen);
	}

	SDL_Flip(Globals::sdl_screen);
#endif
	// Reset scanline counter:
	lastRenderedScanline = -1;

	startFrame();

#ifdef SDL
	// Check for quiting
	SDL_Event sdl_event;
	while(SDL_PollEvent(&sdl_event) == 1) {
		if(sdl_event.type == SDL_QUIT) {
			nes->cpu->stopRunning = true;
		}
	}
#endif
	// Check for key presses
	nes->_joy1->poll_for_key_events();
	//nes->_joy2->poll_for_key_events();

	// Figure out how much time we spent, and how much we have left
	gettimeofday(&_frame_end, nullptr);
	double e = _frame_end.tv_usec + (_frame_end.tv_sec * 1000000.0);
	double s = _frame_start.tv_usec + (_frame_start.tv_sec * 1000000.0);
	double diff = e - s;
	
	// Sleep if there is still time left over, after drawing this frame
	double wait = 0;
	if(diff < Globals::MS_PER_FRAME) {
		wait = Globals::MS_PER_FRAME - diff;
		usleep(wait);
	}
	
	// Print the frame rate
	_ticks_since_second += diff + wait;
	if(_ticks_since_second >= 1000000.0) {
		printf("FPS: %d\n", frameCounter);
#ifdef NACL
		this->nes->_salty_nes->set_fps(frameCounter);
#endif
		_ticks_since_second = 0;
		frameCounter = 0;
	}
	++frameCounter;
	
	// Get the start time of the next frame
	gettimeofday(&_frame_start, nullptr);
}

void PPU::endScanline() {
	if(scanline < 19 + vblankAdd) {

		// VINT
		// do nothing.
	} else if(scanline == 19 + vblankAdd) {

		// Dummy scanline->
		// May be variable length:
		if(dummyCycleToggle) {

			// Remove dead cycle at end of scanline,
			// for next scanline:
			curX = 1;
			dummyCycleToggle = !dummyCycleToggle;

		}

	} else if(scanline == 20 + vblankAdd) {


		// Clear VBlank flag:
		setStatusFlag(STATUS_VBLANK, false);

		// Clear Sprite #0 hit flag:
		setStatusFlag(STATUS_SPRITE0HIT, false);
		hitSpr0 = false;
		spr0HitX = -1;
		spr0HitY = -1;

		if(f_bgVisibility == 1 || f_spVisibility == 1) {

			// Update counters:
			cntFV = regFV;
			cntV = regV;
			cntH = regH;
			cntVT = regVT;
			cntHT = regHT;

			if(f_bgVisibility == 1) {
				// Render dummy scanline:
				renderBgScanline(&_screen_buffer, 0);
			}

		}

		if(f_bgVisibility == 1 && f_spVisibility == 1) {

			// Check sprite 0 hit for first scanline:
			checkSprite0(0);

		}

		if(f_bgVisibility == 1 || f_spVisibility == 1) {
			// Clock mapper IRQ Counter:
			nes->memMapper->clockIrqCounter();
		}

	} else if(scanline >= 21 + vblankAdd && scanline <= 260) {

		// Render normally:
		if(f_bgVisibility == 1) {

			if(!scanlineAlreadyRendered) {
				// update scroll:
				cntHT = regHT;
				cntH = regH;
				renderBgScanline(&bgbuffer, scanline + 1 - 21);
			}
			scanlineAlreadyRendered = false;

			// Check for sprite 0 (next scanline):
			if(!hitSpr0 && f_spVisibility == 1) {
				if(sprX[0] >= -7 && sprX[0] < 256 && sprY[0] + 1 <= (scanline - vblankAdd + 1 - 21) && (sprY[0] + 1 + (f_spriteSize == 0 ? 8 : 16)) >= (scanline - vblankAdd + 1 - 21)) {
					if(checkSprite0(scanline + vblankAdd + 1 - 21)) {
						////System.out.println("found spr0. curscan="+scanline+" hitscan="+spr0HitY);
						hitSpr0 = true;
					}
				}
			}

		}

		if(f_bgVisibility == 1 || f_spVisibility == 1) {
			// Clock mapper IRQ Counter:
			nes->memMapper->clockIrqCounter();
		}

	} else if(scanline == 261 + vblankAdd) {

		// Dead scanline, no rendering.
		// Set VINT:
		setStatusFlag(STATUS_VBLANK, true);
		requestEndFrame = true;
		nmiCounter = 9;

		// Wrap around:
		scanline = -1;	// will be incremented to 0

	}

	++scanline;
	regsToAddress();
	cntsToAddress();

}

void PPU::startFrame() {
	// Set background color:
	int bgColor = 0;

	if(f_dispType == 0) {

		// Color display.
		// f_color determines color emphasis.
		// Use first entry of image palette as BG color.
		bgColor = imgPalette[0];

	} else {

		// Monochrome display.
		// f_color determines the bg color.
		switch (f_color) {

			case 0: {
				// Black
				bgColor = 0x00000;
				break;
			}
			case 1: {
				// Green
				bgColor = 0x00FF00;
			}
			case 2: {
				// Blue
				bgColor = 0xFF0000;
			}
			case 3: {
				// Invalid. Use black.
				bgColor = 0x000000;
			}
			case 4: {
				// Red
				bgColor = 0x0000FF;
			}
			default: {
				// Invalid. Use black.
				bgColor = 0x0;
			}
		}

	}

	std::fill(_screen_buffer.begin(), _screen_buffer.end(), bgColor);
	std::fill(pixrendered.begin(), pixrendered.end(), 65);
}

void PPU::endFrame() {
	// Draw spr#0 hit coordinates:
	if(showSpr0Hit) {
		// Spr 0 position:
		if(sprX[0] >= 0 && sprX[0] < 256 && sprY[0] >= 0 && sprY[0] < 240) {
			for(size_t i = 0; i < 256; ++i) {
				_screen_buffer[(sprY[0] << 8) + i] = 0xFF5555;
			}
			for(size_t i = 0; i < 240; ++i) {
				_screen_buffer[(i << 8) + sprX[0]] = 0xFF5555;
			}
		}
		// Hit position:
		if(spr0HitX >= 0 && spr0HitX < 256 && spr0HitY >= 0 && spr0HitY < 240) {
			for(size_t i = 0; i < 256; ++i) {
				_screen_buffer[(spr0HitY << 8) + i] = 0x55FF55;
			}
			for(size_t i = 0; i < 240; ++i) {
				_screen_buffer[(i << 8) + spr0HitX] = 0x55FF55;
			}
		}
	}
}

void PPU::updateControlReg1(int value) {
	triggerRendering();

	f_nmiOnVblank = (value >> 7) & 1;
	f_spriteSize = (value >> 5) & 1;
	f_bgPatternTable = (value >> 4) & 1;
	f_spPatternTable = (value >> 3) & 1;
	f_addrInc = (value >> 2) & 1;
	f_nTblAddress = value & 3;

	regV = (value >> 1) & 1;
	regH = value & 1;
	regS = (value >> 4) & 1;
}

void PPU::updateControlReg2(int value) {
	triggerRendering();

	f_color = (value >> 5) & 7;
	f_spVisibility = (value >> 4) & 1;
	f_bgVisibility = (value >> 3) & 1;
	f_spClipping = (value >> 2) & 1;
	f_bgClipping = (value >> 1) & 1;
	f_dispType = value & 1;

	if(f_dispType == 0) {
		nes->palTable->setEmphasis(f_color);
	}
	updatePalettes();
}

void PPU::setStatusFlag(int flag, bool value) {
	int n = 1 << flag;
	int memValue = nes->getCpuMemory()->load(0x2002);
	memValue = ((memValue & (255 - n)) | (value ? n : 0));
	nes->getCpuMemory()->write(0x2002, static_cast<uint16_t>(memValue));
}


// CPU Register $2002:
// Read the Status Register.
uint16_t PPU::readStatusRegister() {
	tmp = nes->getCpuMemory()->load(0x2002);

	// Reset scroll & VRAM Address toggle:
	firstWrite = true;

	// Clear VBlank flag:
	setStatusFlag(STATUS_VBLANK, false);

	// Fetch status data:
	return tmp;
}

// CPU Register $2003:
// Write the SPR-RAM address that is used for sramWrite (Register 0x2004 in CPU memory map)
void PPU::writeSRAMAddress(uint16_t address) {
	sramAddress = address;
}

// CPU Register $2004 (R):
// Read from SPR-RAM (Sprite RAM).
// The address should be set first.
uint16_t PPU::sramLoad() {
	uint16_t tmp = sprMem->load(sramAddress);
	/*++sramAddress; // Increment address
	sramAddress%=0x100;*/
	return tmp;
}


// CPU Register $2004 (W):
// Write to SPR-RAM (Sprite RAM).
// The address should be set first.
void PPU::sramWrite(uint16_t value) {
	sprMem->write(sramAddress, value);
	spriteRamWriteUpdate(sramAddress, value);
	++sramAddress; // Increment address
	sramAddress %= 0x100;
}


// CPU Register $2005:
// Write to scroll registers.
// The first write is the vertical offset, the second is the
// horizontal offset:
void PPU::scrollWrite(uint16_t value) {
	triggerRendering();
	if(firstWrite) {

		// First write, horizontal scroll:
		regHT = (value >> 3) & 31;
		regFH = value & 7;

	} else {

		// Second write, vertical scroll:
		regFV = value & 7;
		regVT = (value >> 3) & 31;

	}
	firstWrite = !firstWrite;
}

// CPU Register $2006:
// Sets the adress used when reading/writing from/to VRAM.
// The first write sets the high byte, the second the low byte.
void PPU::writeVRAMAddress(int address) {
	if(firstWrite) {
		regFV = (address >> 4) & 3;
		regV = (address >> 3) & 1;
		regH = (address >> 2) & 1;
		regVT = (regVT & 7) | ((address & 3) << 3);

	} else {

		triggerRendering();

		regVT = (regVT & 24) | ((address >> 5) & 7);
		regHT = address & 31;

		cntFV = regFV;
		cntV = regV;
		cntH = regH;
		cntVT = regVT;
		cntHT = regHT;

		checkSprite0(scanline - vblankAdd + 1 - 21);

	}

	firstWrite = !firstWrite;

	// Invoke mapper latch:
	cntsToAddress();
	if(vramAddress < 0x2000) {
		nes->memMapper->latchAccess(vramAddress);
	}
}

// CPU Register $2007(R):
// Read from PPU memory. The address should be set first.
uint16_t PPU::vramLoad() {
	cntsToAddress();
	regsToAddress();

	// If address is in range 0x0000-0x3EFF, return buffered values:
	if(vramAddress <= 0x3EFF) {

		uint16_t tmp = vramBufferedReadValue;

		// Update buffered value:
		if(vramAddress < 0x2000) {
			vramBufferedReadValue = ppuMem->load(vramAddress);
		} else {
			vramBufferedReadValue = mirroredLoad(vramAddress);
		}

		// Mapper latch access:
		if(vramAddress < 0x2000) {
			nes->memMapper->latchAccess(vramAddress);
		}

		// Increment by either 1 or 32, depending on d2 of Control Register 1:
		vramAddress += (f_addrInc == 1 ? 32 : 1);

		cntsFromAddress();
		regsFromAddress();
		return tmp; // Return the previous buffered value.

	}

	// No buffering in this mem range. Read normally.
	uint16_t tmp = mirroredLoad(vramAddress);

	// Increment by either 1 or 32, depending on d2 of Control Register 1:
	vramAddress += (f_addrInc == 1 ? 32 : 1);

	cntsFromAddress();
	regsFromAddress();

	return tmp;
}

// CPU Register $2007(W):
// Write to PPU memory. The address should be set first.
void PPU::vramWrite(uint16_t value) {
	triggerRendering();
	cntsToAddress();
	regsToAddress();

	if(vramAddress >= 0x2000) {
		// Mirroring is used.
		mirroredWrite(vramAddress, value);
	} else {

		// Write normally.
		writeMem(vramAddress, value);

		// Invoke mapper latch:
		nes->memMapper->latchAccess(vramAddress);

	}

	// Increment by either 1 or 32, depending on d2 of Control Register 1:
	vramAddress += (f_addrInc == 1 ? 32 : 1);
	regsFromAddress();
	cntsFromAddress();
}

// CPU Register $4014:
// Write 256 bytes of main memory
// into Sprite RAM.
void PPU::sramDMA(uint16_t value) {
	Memory* cpuMem = nes->getCpuMemory();
	int baseAddress = value * 0x100;
	uint16_t data;
	for(size_t i = sramAddress; i < 256; ++i) {
		data = cpuMem->load(baseAddress + i);
		sprMem->write(i, data);
		spriteRamWriteUpdate(i, data);
	}

	nes->getCpu()->haltCycles(513);
}

// Updates the scroll registers from a new VRAM address.
void PPU::regsFromAddress() {
	address = (vramTmpAddress >> 8) & 0xFF;
	regFV = (address >> 4) & 7;
	regV = (address >> 3) & 1;
	regH = (address >> 2) & 1;
	regVT = (regVT & 7) | ((address & 3) << 3);

	address = vramTmpAddress & 0xFF;
	regVT = (regVT & 24) | ((address >> 5) & 7);
	regHT = address & 31;
}

// Updates the scroll registers from a new VRAM address.
void PPU::cntsFromAddress() {
	address = (vramAddress >> 8) & 0xFF;
	cntFV = (address >> 4) & 3;
	cntV = (address >> 3) & 1;
	cntH = (address >> 2) & 1;
	cntVT = (cntVT & 7) | ((address & 3) << 3);

	address = vramAddress & 0xFF;
	cntVT = (cntVT & 24) | ((address >> 5) & 7);
	cntHT = address & 31;
}

void PPU::regsToAddress() {
	b1 = (regFV & 7) << 4;
	b1 |= (regV & 1) << 3;
	b1 |= (regH & 1) << 2;
	b1 |= (regVT >> 3) & 3;

	b2 = (regVT & 7) << 5;
	b2 |= regHT & 31;

	vramTmpAddress = ((b1 << 8) | b2) & 0x7FFF;
}

void PPU::cntsToAddress() {
	b1 = (cntFV & 7) << 4;
	b1 |= (cntV & 1) << 3;
	b1 |= (cntH & 1) << 2;
	b1 |= (cntVT >> 3) & 3;

	b2 = (cntVT & 7) << 5;
	b2 |= cntHT & 31;

	vramAddress = ((b1 << 8) | b2) & 0x7FFF;
}

void PPU::incTileCounter(int count) {
	for(i = count; i != 0; --i) {
		++cntHT;
		if(cntHT == 32) {
			cntHT = 0;
			++cntVT;
			if(cntVT >= 30) {
				++cntH;
				if(cntH == 2) {
					cntH = 0;
					++cntV;
					if(cntV == 2) {
						cntV = 0;
						++cntFV;
						cntFV &= 0x7;
					}
				}
			}
		}
	}
}

// Reads from memory, taking into account
// mirroring/mapping of address ranges.
uint16_t PPU::mirroredLoad(int address) {
	return ppuMem->load(vramMirrorTable[address]);
}

// Writes to memory, taking into account
// mirroring/mapping of address ranges.
void PPU::mirroredWrite(int address, uint16_t value) {
	if(address >= 0x3f00 && address < 0x3f20) {

		// Palette write mirroring.

		if(address == 0x3F00 || address == 0x3F10) {

			writeMem(0x3F00, value);
			writeMem(0x3F10, value);

		} else if(address == 0x3F04 || address == 0x3F14) {

			writeMem(0x3F04, value);
			writeMem(0x3F14, value);

		} else if(address == 0x3F08 || address == 0x3F18) {

			writeMem(0x3F08, value);
			writeMem(0x3F18, value);

		} else if(address == 0x3F0C || address == 0x3F1C) {

			writeMem(0x3F0C, value);
			writeMem(0x3F1C, value);

		} else {

			writeMem(address, value);

		}

	} else {

		// Use lookup table for mirrored address:
		if(address < static_cast<int>(vramMirrorTable.size())) {
			writeMem(vramMirrorTable[address], value);
		} else {
			//System.out.println("Invalid VRAM address: "+Misc.hex16(address));
			nes->getCpu()->setCrashed(true);
		}

	}
}

void PPU::triggerRendering() {
	if(scanline - vblankAdd >= 21 && scanline - vblankAdd <= 260) {

		// Render sprites, and combine:
		renderFramePartially(lastRenderedScanline + 1, scanline - vblankAdd - 21 - lastRenderedScanline);

		// Set last rendered scanline:
		lastRenderedScanline = scanline - vblankAdd - 21;
	}
}

void PPU::renderFramePartially(int startScan, int scanCount) {
	if(f_spVisibility == 1 && !Globals::disableSprites) {
		renderSpritesPartially(startScan, scanCount, true);
	}

	if(f_bgVisibility == 1) {
		si = startScan << 8;
		ei = (startScan + scanCount) << 8;
		if(ei > 0xF000) {
			ei = 0xF000;
		}
		for(destIndex = si; destIndex < ei; ++destIndex) {
			if(pixrendered[destIndex] > 0xFF) {
				_screen_buffer[destIndex] = bgbuffer[destIndex];
			}
		}
	}

	if(f_spVisibility == 1 && !Globals::disableSprites) {
		renderSpritesPartially(startScan, scanCount, false);
	}

	validTileData = false;
}

void PPU::renderBgScanline(vector<int>* buffer, int scan) {
	baseTile = (regS == 0 ? 0 : 256);
	destIndex = (scan << 8) - regFH;
	curNt = ntable1[cntV + cntV + cntH];

	cntHT = regHT;
	cntH = regH;
	curNt = ntable1[cntV + cntV + cntH];

	if(scan < 240 && (scan - cntFV) >= 0) {

		tscanoffset = cntFV << 3;
		y = scan - cntFV;
		for(tile = 0; tile < 32; ++tile) {

			if(scan >= 0) {

				// Fetch tile & attrib data:
				if(validTileData) {
					// Get data from array:
					t = scantile[tile];
					tpix = &t->pix;
					att = attrib[tile];
				} else {
					// Fetch data:
					t = ptTile[baseTile + nameTable[curNt]->getTileIndex(cntHT, cntVT)];
					tpix = &t->pix;
					att = nameTable[curNt]->getAttrib(cntHT, cntVT);
					scantile[tile] = t;
					attrib[tile] = att;
				}

				// Render tile scanline:
				sx = 0;
				x = (tile << 3) - regFH;
				if(x > -8) {
					if(x < 0) {
						destIndex -= x;
						sx = -x;
					}
					if(t->opaque[cntFV]) {
						for(; sx < 8; ++sx) {
							(*buffer)[destIndex] = imgPalette[(*tpix)[tscanoffset + sx] + att];
							pixrendered[destIndex] |= 256;
							++destIndex;
						}
					} else {
						for(; sx < 8; ++sx) {
							col = (*tpix)[tscanoffset + sx];
							if(col != 0) {
								(*buffer)[destIndex] = imgPalette[col + att];
								pixrendered[destIndex] |= 256;
							}
							++destIndex;
						}
					}
				}

			}

			// Increase Horizontal Tile Counter:
			++cntHT;
			if(cntHT == 32) {
				cntHT = 0;
				++cntH;
				cntH %= 2;
				curNt = ntable1[(cntV << 1) + cntH];
			}


		}

		// Tile data for one row should now have been fetched,
		// so the data in the array is valid.
		validTileData = true;

	}

	// update vertical scroll:
	++cntFV;
	if(cntFV == 8) {
		cntFV = 0;
		++cntVT;
		if(cntVT == 30) {
			cntVT = 0;
			++cntV;
			cntV %= 2;
			curNt = ntable1[(cntV << 1) + cntH];
		} else if(cntVT == 32) {
			cntVT = 0;
		}

		// Invalidate fetched data:
		validTileData = false;

	}
}

void PPU::renderSpritesPartially(int startscan, int scancount, bool bgPri) {
	if(f_spVisibility == 1) {

		for(size_t i = 0; i < 64; ++i) {
			if(bgPriority[i] == bgPri && sprX[i] >= 0 && sprX[i] < 256 && sprY[i] + 8 >= startscan && sprY[i] < startscan + scancount) {
				// Show sprite.
				if(f_spriteSize == 0) {
					// 8x8 sprites

					srcy1 = 0;
					srcy2 = 8;

					if(sprY[i] < startscan) {
						srcy1 = startscan - sprY[i] - 1;
					}

					if(sprY[i] + 8 > startscan + scancount) {
						srcy2 = startscan + scancount - sprY[i] + 1;
					}

					if(f_spPatternTable == 0) {
						ptTile[sprTile[i]]->render(0, srcy1, 8, srcy2, sprX[i], sprY[i] + 1, &_screen_buffer, sprCol[i], &sprPalette, horiFlip[i], vertFlip[i], i, &pixrendered);
					} else {
						ptTile[sprTile[i] + 256]->render(0, srcy1, 8, srcy2, sprX[i], sprY[i] + 1, &_screen_buffer, sprCol[i], &sprPalette, horiFlip[i], vertFlip[i], i, &pixrendered);
					}
				} else {
					// 8x16 sprites
					int top = sprTile[i];
					if((top & 1) != 0) {
						top = sprTile[i] - 1 + 256;
					}

					srcy1 = 0;
					srcy2 = 8;

					if(sprY[i] < startscan) {
						srcy1 = startscan - sprY[i] - 1;
					}

					if(sprY[i] + 8 > startscan + scancount) {
						srcy2 = startscan + scancount - sprY[i];
					}

					ptTile[top + (vertFlip[i] ? 1 : 0)]->render(0, srcy1, 8, srcy2, sprX[i], sprY[i] + 1, &_screen_buffer, sprCol[i], &sprPalette, horiFlip[i], vertFlip[i], i, &pixrendered);

					srcy1 = 0;
					srcy2 = 8;

					if(sprY[i] + 8 < startscan) {
						srcy1 = startscan - (sprY[i] + 8 + 1);
					}

					if(sprY[i] + 16 > startscan + scancount) {
						srcy2 = startscan + scancount - (sprY[i] + 8);
					}

					ptTile[top + (vertFlip[i] ? 0 : 1)]->render(0, srcy1, 8, srcy2, sprX[i], sprY[i] + 1 + 8, &_screen_buffer, sprCol[i], &sprPalette, horiFlip[i], vertFlip[i], i, &pixrendered);

				}
			}
		}
	}
}

bool PPU::checkSprite0(int scan) {
	spr0HitX = -1;
	spr0HitY = -1;

	int toffset;
	int tIndexAdd = (f_spPatternTable == 0 ? 0 : 256);
	int x, y;
	int bufferIndex;
	//int col;
	//bool bgPri;
	Tile* t;

	x = sprX[0];
	y = sprY[0] + 1;


	if(f_spriteSize == 0) {

		// 8x8 sprites.

		// Check range:
		if(y <= scan && y + 8 > scan && x >= -7 && x < 256) {

			// Sprite is in range.
			// Draw scanline:
			t = ptTile[sprTile[0] + tIndexAdd];
			//col = sprCol[0];
			//bgPri = bgPriority[0];

			if(vertFlip[0]) {
				toffset = 7 - (scan - y);
			} else {
				toffset = scan - y;
			}
			toffset *= 8;

			bufferIndex = scan * 256 + x;
			if(horiFlip[0]) {
				for(int i = 7; i >= 0; --i) {
					if(x >= 0 && x < 256) {
						if(bufferIndex >= 0 && bufferIndex < 61440 && pixrendered[bufferIndex] != 0) {
							if(t->pix[toffset + i] != 0) {
								spr0HitX = bufferIndex % 256;
								spr0HitY = scan;
								return true;
							}
						}
					}
					++x;
					++bufferIndex;
				}

			} else {

				for(size_t i = 0; i < 8; ++i) {
					if(x >= 0 && x < 256) {
						if(bufferIndex >= 0 && bufferIndex < 61440 && pixrendered[bufferIndex] != 0) {
							if(t->pix[toffset + i] != 0) {
								spr0HitX = bufferIndex % 256;
								spr0HitY = scan;
								return true;
							}
						}
					}
					++x;
					++bufferIndex;
				}

			}

		}


	} else {

		// 8x16 sprites:

		// Check range:
		if(y <= scan && y + 16 > scan && x >= -7 && x < 256) {

			// Sprite is in range.
			// Draw scanline:

			if(vertFlip[0]) {
				toffset = 15 - (scan - y);
			} else {
				toffset = scan - y;
			}

			if(toffset < 8) {
				// first half of sprite.
				t = ptTile[sprTile[0] + (vertFlip[0] ? 1 : 0) + ((sprTile[0] & 1) != 0 ? 255 : 0)];
			} else {
				// second half of sprite.
				t = ptTile[sprTile[0] + (vertFlip[0] ? 0 : 1) + ((sprTile[0] & 1) != 0 ? 255 : 0)];
				if(vertFlip[0]) {
					toffset = 15 - toffset;
				} else {
					toffset -= 8;
				}
			}
			toffset *= 8;
			col = sprCol[0];
			//bgPri = bgPriority[0];

			bufferIndex = scan * 256 + x;
			if(horiFlip[0]) {

				for(int i = 7; i >= 0; --i) {
					if(x >= 0 && x < 256) {
						if(bufferIndex >= 0 && bufferIndex < 61440 && pixrendered[bufferIndex] != 0) {
							if(t->pix[toffset + i] != 0) {
								spr0HitX = bufferIndex % 256;
								spr0HitY = scan;
								return true;
							}
						}
					}
					++x;
					++bufferIndex;
				}

			} else {

				for(size_t i = 0; i < 8; ++i) {
					if(x >= 0 && x < 256) {
						if(bufferIndex >= 0 && bufferIndex < 61440 && pixrendered[bufferIndex] != 0) {
							if(t->pix[toffset + i] != 0) {
								spr0HitX = bufferIndex % 256;
								spr0HitY = scan;
								return true;
							}
						}
					}
					++x;
					++bufferIndex;
				}

			}

		}

	}

	return false;
}
/*
// Renders the contents of the
// pattern table into an image.
void PPU::renderPattern() {
	vector<int>* buffer = get_pattern_buffer();

	int tIndex = 0;
	for(size_t j = 0; j < 2; ++j) {
		for(size_t y = 0; y < 16; ++y) {
			for(size_t x = 0; x < 16; ++x) {
				ptTile[tIndex]->renderSimple(j * 128 + x * 8, y * 8, buffer, 0, sprPalette);
				++tIndex;
			}
		}
	}
//		nes->getGui()->getPatternView()->imageReady(false);

}

void PPU::renderNameTables() {
	vector<int>* buffer = get_name_buffer();
	if(f_bgPatternTable == 0) {
		baseTile = 0;
	} else {
		baseTile = 256;
	}

	int ntx_max = 2;
	int nty_max = 2;

	if(currentMirroring == ROM::HORIZONTAL_MIRRORING) {
		ntx_max = 1;
	} else if(currentMirroring == ROM::VERTICAL_MIRRORING) {
		nty_max = 1;
	}

	for(int nty = 0; nty < nty_max; ++nty) {
		for(int ntx = 0; ntx < ntx_max; ++ntx) {

			int nt = ntable1[nty * 2 + ntx];
			int x = ntx * 128;
			int y = nty * 120;

			// Render nametable:
			for(int ty = 0; ty < 30; ++ty) {
				for(int tx = 0; tx < 32; ++tx) {
					//ptTile[baseTile+nameTable[nt].getTileIndex(tx,ty)].render(0,0,4,4,x+tx*4,y+ty*4,buffer,nameTable[nt].getAttrib(tx,ty), &imgPalette,false,false,0,dummyPixPriTable);
					ptTile[baseTile + nameTable[nt]->getTileIndex(tx, ty)]->renderSmall(x + tx * 4, y + ty * 4, buffer, nameTable[nt]->getAttrib(tx, ty), &imgPalette);
				}
			}

		}
	}

	if(currentMirroring == ROM::HORIZONTAL_MIRRORING) {
		// double horizontally:
		for(int y = 0; y < 240; ++y) {
			for(int x = 0; x < 128; ++x) {
				buffer[(y << 8) + 128 + x] = buffer[(y << 8) + x];
			}
		}
	} else if(currentMirroring == ROM::VERTICAL_MIRRORING) {
		// double vertically:
		for(int y = 0; y < 120; ++y) {
			for(int x = 0; x < 256; ++x) {
				buffer[(y << 8) + 0x7800 + x] = buffer[(y << 8) + x];
			}
		}
	}

//		nes->getGui()->getNameTableView()->imageReady(false);

}

void PPU::renderPalettes() {
	vector<int>* buffer = get_img_palette_buffer();
	for(int i = 0; i < 16; ++i) {
		for(int y = 0; y < 16; ++y) {
			for(int x = 0; x < 16; ++x) {
				(*buffer)[y * 256 + i * 16 + x] = imgPalette[i];
			}
		}
	}

	buffer = get_spr_palette_buffer();
	for(int i = 0; i < 16; ++i) {
		for(int y = 0; y < 16; ++y) {
			for(int x = 0; x < 16; ++x) {
				(*buffer)[y * 256 + i * 16 + x] = sprPalette[i];
			}
		}
	}

//		nes->getGui()->getImgPalView()->imageReady(false);
//		nes->getGui()->getSprPalView()->imageReady(false);

}
*/

// This will write to PPU memory, and
// update internally buffered data
// appropriately.
void PPU::writeMem(int address, uint16_t value) {
	ppuMem->write(address, value);

	// Update internally buffered data:
	if(address < 0x2000) {

		ppuMem->write(address, value);
		patternWrite(address, value);

	} else if(address >= 0x2000 && address < 0x23c0) {

		nameTableWrite(ntable1[0], address - 0x2000, value);

	} else if(address >= 0x23c0 && address < 0x2400) {

		attribTableWrite(ntable1[0], address - 0x23c0, value);

	} else if(address >= 0x2400 && address < 0x27c0) {

		nameTableWrite(ntable1[1], address - 0x2400, value);

	} else if(address >= 0x27c0 && address < 0x2800) {

		attribTableWrite(ntable1[1], address - 0x27c0, value);

	} else if(address >= 0x2800 && address < 0x2bc0) {

		nameTableWrite(ntable1[2], address - 0x2800, value);

	} else if(address >= 0x2bc0 && address < 0x2c00) {

		attribTableWrite(ntable1[2], address - 0x2bc0, value);

	} else if(address >= 0x2c00 && address < 0x2fc0) {

		nameTableWrite(ntable1[3], address - 0x2c00, value);

	} else if(address >= 0x2fc0 && address < 0x3000) {

		attribTableWrite(ntable1[3], address - 0x2fc0, value);

	} else if(address >= 0x3f00 && address < 0x3f20) {

		updatePalettes();

	}
}

// Reads data from $3f00 to $f20
// into the two buffered palettes.
void PPU::updatePalettes() {
	for(size_t i = 0; i < 16; ++i) {
		if(f_dispType == 0) {
			imgPalette[i] = nes->palTable->getEntry(ppuMem->load(0x3f00 + i) & 63);
		} else {
			imgPalette[i] = nes->palTable->getEntry(ppuMem->load(0x3f00 + i) & 32);
		}
	}
	for(size_t i = 0; i < 16; ++i) {
		if(f_dispType == 0) {
			sprPalette[i] = nes->palTable->getEntry(ppuMem->load(0x3f10 + i) & 63);
		} else {
			sprPalette[i] = nes->palTable->getEntry(ppuMem->load(0x3f10 + i) & 32);
		}
	}

//renderPalettes();

}


// Updates the internal pattern
// table buffers with this new byte.
void PPU::patternWrite(int address, uint16_t value) {
	int tileIndex = address / 16;
	int leftOver = address % 16;
	if(leftOver < 8) {
		ptTile[tileIndex]->setScanline(leftOver, value, ppuMem->load(address + 8));
	} else {
		ptTile[tileIndex]->setScanline(leftOver - 8, ppuMem->load(address - 8), value);
	}
}

void PPU::patternWrite(int address, vector<uint16_t>* value, size_t offset, size_t length) {
	int tileIndex;
	int leftOver;

	for(size_t i = 0; i < length; ++i) {

		tileIndex = (address + i) >> 4;
		leftOver = (address + i) % 16;

		if(leftOver < 8) {
			ptTile[tileIndex]->setScanline(leftOver, (*value)[offset + i], ppuMem->load(address + 8 + i));
		} else {
			ptTile[tileIndex]->setScanline(leftOver - 8, ppuMem->load(address - 8 + i), (*value)[offset + i]);
		}

	}
}

void PPU::invalidateFrameCache() {
	// Clear the no-update scanline buffer:
	requestRenderAll = true;
}

// Updates the internal name table buffers
// with this new byte.
void PPU::nameTableWrite(int index, int address, uint16_t value) {
	nameTable[index]->writeTileIndex(address, value);

	// Update Sprite #0 hit:
	//updateSpr0Hit();
	checkSprite0(scanline + 1 - vblankAdd - 21);
}

// Updates the internal pattern
// table buffers with this new attribute
// table byte.
void PPU::attribTableWrite(int index, int address, uint16_t value) {
	nameTable[index]->writeAttrib(address, value);
}

// Updates the internally buffered sprite
// data with this new byte of info.
void PPU::spriteRamWriteUpdate(int address, uint16_t value) {
	int tIndex = address / 4;

	if(tIndex == 0) {
		//updateSpr0Hit();
		checkSprite0(scanline + 1 - vblankAdd - 21);
	}

	if(address % 4 == 0) {

		// Y coordinate
		sprY[tIndex] = value;

	} else if(address % 4 == 1) {

		// Tile index
		sprTile[tIndex] = value;

	} else if(address % 4 == 2) {

		// Attributes
		vertFlip[tIndex] = ((value & 0x80) != 0);
		horiFlip[tIndex] = ((value & 0x40) != 0);
		bgPriority[tIndex] = ((value & 0x20) != 0);
		sprCol[tIndex] = (value & 3) << 2;

	} else if(address % 4 == 3) {

		// X coordinate
		sprX[tIndex] = value;

	}
}

void PPU::doNMI() {
	// Set VBlank flag:
	setStatusFlag(STATUS_VBLANK, true);
	//nes->getCpu().doNonMaskableInterrupt();
	nes->getCpu()->requestIrq(CPU::IRQ_NMI);
}

int PPU::statusRegsToInt() {
	int ret = 0;
	ret = (f_nmiOnVblank) |
			(f_spriteSize << 1) |
			(f_bgPatternTable << 2) |
			(f_spPatternTable << 3) |
			(f_addrInc << 4) |
			(f_nTblAddress << 5) |
			(f_color << 6) |
			(f_spVisibility << 7) |
			(f_bgVisibility << 8) |
			(f_spClipping << 9) |
			(f_bgClipping << 10) |
			(f_dispType << 11);

	return ret;
}

void PPU::statusRegsFromInt(int n) {
	f_nmiOnVblank = (n) & 0x1;
	f_spriteSize = (n >> 1) & 0x1;
	f_bgPatternTable = (n >> 2) & 0x1;
	f_spPatternTable = (n >> 3) & 0x1;
	f_addrInc = (n >> 4) & 0x1;
	f_nTblAddress = (n >> 5) & 0x1;

	f_color = (n >> 6) & 0x1;
	f_spVisibility = (n >> 7) & 0x1;
	f_bgVisibility = (n >> 8) & 0x1;
	f_spClipping = (n >> 9) & 0x1;
	f_bgClipping = (n >> 10) & 0x1;
	f_dispType = (n >> 11) & 0x1;
}

void PPU::stateLoad(ByteBuffer* buf) {
	// Check version:
	if(buf->readByte() == 1) {

		// Counters:
		cntFV = buf->readInt();
		cntV = buf->readInt();
		cntH = buf->readInt();
		cntVT = buf->readInt();
		cntHT = buf->readInt();


		// Registers:
		regFV = buf->readInt();
		regV = buf->readInt();
		regH = buf->readInt();
		regVT = buf->readInt();
		regHT = buf->readInt();
		regFH = buf->readInt();
		regS = buf->readInt();


		// VRAM address:
		vramAddress = buf->readInt();
		vramTmpAddress = buf->readInt();


		// Control/Status registers:
		statusRegsFromInt(buf->readInt());


		// VRAM I/O:
		vramBufferedReadValue = static_cast<uint16_t>(buf->readInt());
		firstWrite = buf->readBoolean();
		//System.out.println("firstWrite: "+firstWrite);


		// Mirroring:
		//currentMirroring = -1;
		//setMirroring(buf->readInt());
		for(size_t i = 0; i < vramMirrorTable.size(); ++i) {
			vramMirrorTable[i] = buf->readInt();
		}


		// SPR-RAM I/O:
		sramAddress = static_cast<uint16_t>(buf->readInt());

		// Rendering progression:
		curX = buf->readInt();
		scanline = buf->readInt();
		lastRenderedScanline = buf->readInt();


		// Misc:
		requestEndFrame = buf->readBoolean();
		nmiOk = buf->readBoolean();
		dummyCycleToggle = buf->readBoolean();
		nmiCounter = buf->readInt();
		tmp = static_cast<uint16_t>(buf->readInt());


		// Stuff used during rendering:
		for(size_t i = 0; i < bgbuffer.size(); ++i) {
			bgbuffer[i] = buf->readByte();
		}
		for(size_t i = 0; i < pixrendered.size(); ++i) {
			pixrendered[i] = buf->readByte();
		}

		// Name tables:
		for(size_t i = 0; i < 4; ++i) {
			ntable1[i] = buf->readByte();
			nameTable[i]->stateLoad(buf);
		}

		// Pattern data:
		for(size_t i = 0; i < ptTile.size(); ++i) {
			ptTile[i]->stateLoad(buf);
		}

		// Update internally stored stuff from VRAM memory:
		/*vector<uint16_t>* mem = ppuMem.mem;

		// Palettes:
		for(int i=0x3f00;i<0x3f20;++i) {
		writeMem(i,mem[i]);
		}
		*/
		// Sprite data:
		vector<uint16_t>* sprmem = &(nes->getSprMemory()->mem);
		for(size_t i = 0; i < sprmem->size(); ++i) {
			spriteRamWriteUpdate(i, (*sprmem)[i]);
		}
	}
}

void PPU::stateSave(ByteBuffer* buf) {
	// Version:
	buf->putByte(static_cast<uint16_t>(1));


	// Counters:
	buf->putInt(cntFV);
	buf->putInt(cntV);
	buf->putInt(cntH);
	buf->putInt(cntVT);
	buf->putInt(cntHT);


	// Registers:
	buf->putInt(regFV);
	buf->putInt(regV);
	buf->putInt(regH);
	buf->putInt(regVT);
	buf->putInt(regHT);
	buf->putInt(regFH);
	buf->putInt(regS);


	// VRAM address:
	buf->putInt(vramAddress);
	buf->putInt(vramTmpAddress);


	// Control/Status registers:
	buf->putInt(statusRegsToInt());


	// VRAM I/O:
	buf->putInt(vramBufferedReadValue);
	//System.out.println("firstWrite: "+firstWrite);
	buf->putBoolean(firstWrite);

	// Mirroring:
	//buf->putInt(currentMirroring);
	for(size_t i = 0; i < vramMirrorTable.size(); ++i) {
		buf->putInt(vramMirrorTable[i]);
	}


	// SPR-RAM I/O:
	buf->putInt(sramAddress);


	// Rendering progression:
	buf->putInt(curX);
	buf->putInt(scanline);
	buf->putInt(lastRenderedScanline);


	// Misc:
	buf->putBoolean(requestEndFrame);
	buf->putBoolean(nmiOk);
	buf->putBoolean(dummyCycleToggle);
	buf->putInt(nmiCounter);
	buf->putInt(tmp);


	// Stuff used during rendering:
	for(size_t i = 0; i < bgbuffer.size(); ++i) {
		buf->putByte(static_cast<uint16_t>(bgbuffer[i]));
	}
	for(size_t i = 0; i < pixrendered.size(); ++i) {
		buf->putByte(static_cast<uint16_t>(pixrendered[i]));
	}

	// Name tables:
	for(size_t i = 0; i < 4; ++i) {
		buf->putByte(static_cast<uint16_t>(ntable1[i]));
		nameTable[i]->stateSave(buf);
	}

	// Pattern data:
	for(size_t i = 0; i < ptTile.size(); ++i) {
		ptTile[i]->stateSave(buf);
	}

}

// Reset PPU:
void PPU::reset() {
	ppuMem->reset();
	sprMem->reset();

	vramBufferedReadValue = 0;
	sramAddress = 0;
	curX = 0;
	scanline = 0;
	lastRenderedScanline = 0;
	spr0HitX = 0;
	spr0HitY = 0;
	mapperIrqCounter = 0;

	currentMirroring = -1;

	firstWrite = true;
	requestEndFrame = false;
	nmiOk = false;
	hitSpr0 = false;
	dummyCycleToggle = false;
	validTileData = false;
	nmiCounter = 0;
	tmp = 0;
	att = 0;
	i = 0;

	// Control Flags Register 1:
	f_nmiOnVblank = 0;	// NMI on VBlank. 0=disable, 1=enable
	f_spriteSize = 0;	// Sprite size. 0=8x8, 1=8x16
	f_bgPatternTable = 0; // Background Pattern Table address. 0=0x0000,1=0x1000
	f_spPatternTable = 0; // Sprite Pattern Table address. 0=0x0000,1=0x1000
	f_addrInc = 0;		// PPU Address Increment. 0=1,1=32
	f_nTblAddress = 0;	// Name Table Address. 0=0x2000,1=0x2400,2=0x2800,3=0x2C00

	// Control Flags Register 2:
	f_color = 0;	  	 // Background color. 0=black, 1=blue, 2=green, 4=red
	f_spVisibility = 0;   // Sprite visibility. 0=not displayed,1=displayed
	f_bgVisibility = 0;   // Background visibility. 0=Not Displayed,1=displayed
	f_spClipping = 0;	// Sprite clipping. 0=Sprites invisible in left 8-pixel column,1=No clipping
	f_bgClipping = 0;	// Background clipping. 0=BG invisible in left 8-pixel column, 1=No clipping
	f_dispType = 0;	  // Display type. 0=color, 1=monochrome


	// Counters:
	cntFV = 0;
	cntV = 0;
	cntH = 0;
	cntVT = 0;
	cntHT = 0;

	// Registers:
	regFV = 0;
	regV = 0;
	regH = 0;
	regVT = 0;
	regHT = 0;
	regFH = 0;
	regS = 0;

	// Initialize stuff:
	init();
}

#ifdef NACL
bool PPU::is_safe_to_paint() {
	return nes->_salty_nes->width() * nes->_salty_nes->height() == 
	(_zoom * 256) * (_zoom * 240);
}
#endif


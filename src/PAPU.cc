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
void fill_audio_sdl_cb(void* udata, uint8_t* stream, int len) {
	PAPU* papu = reinterpret_cast<PAPU*>(udata);
	
	if(!papu->ready_for_buffer_write)
		return;

	uint32_t mix_len = len > papu->bufferIndex ? papu->bufferIndex : len;
	SDL_MixAudio(stream, papu->sampleBuffer.data(), mix_len, SDL_MIX_MAXVOLUME);
	papu->bufferIndex = 0;
	//std::fill(papu->sampleBuffer.begin(), papu->sampleBuffer.end(), 0);
	papu->ready_for_buffer_write = false;
}
#endif

#ifdef NACL
void fill_audio_nacl_cb(void* samples, uint32_t buffer_size, void* data) {
	SaltyNES* salty_nes = reinterpret_cast<SaltyNES*>(data);
	PAPU* papu = salty_nes->vnes->nes->papu;
	uint8_t* buff = reinterpret_cast<uint8_t*>(samples);

	// If there is no sound, just play zero
	if(salty_nes->vnes->nes->_is_paused || papu == nullptr || !papu->ready_for_buffer_write) {
		for(size_t i=0; i<buffer_size; ++i) {
			buff[i] = 0;
		}
		return;
	}

	//const uint32_t channels = papu->stereo ? 2 : 1;

	// Make sure we can't write outside the buffer.
	//assert(buffer_size >= (sizeof(*buff) * channels * papu->sample_frame_count_));

	size_t mix_len = buffer_size > static_cast<size_t>(papu->bufferIndex) ? static_cast<size_t>(papu->bufferIndex) : buffer_size;
	for(size_t i=0; i<mix_len; ++i) {
		buff[i] = papu->sampleBuffer[i];
	}

	papu->bufferIndex = 0;
	papu->ready_for_buffer_write = false;
}
#endif

const uint8_t PAPU::panning[] = {
	80,
	170,
	100,
	150,
	128
};

const uint8_t PAPU::lengthLookup[] = {
	0x0A, 0xFE,
	0x14, 0x02,
	0x28, 0x04,
	0x50, 0x06,
	0xA0, 0x08,
	0x3C, 0x0A,
	0x0E, 0x0C,
	0x1A, 0x0E,
	0x0C, 0x10,
	0x18, 0x12,
	0x30, 0x14,
	0x60, 0x16,
	0xC0, 0x18,
	0x48, 0x1A,
	0x10, 0x1C,
	0x20, 0x1E
};

const uint16_t PAPU::dmcFreqLookup[] = {
	0xD60,
	0xBE0,
	0xAA0,
	0xA00,
	0x8F0,
	0x7F0,
	0x710,
	0x6B0,
	0x5F0,
	0x500,
	0x470,
	0x400,
	0x350,
	0x2A0,
	0x240,
	0x1B0
};

const uint16_t PAPU::noiseWavelengthLookup[] = {
	0x004,
	0x008,
	0x010,
	0x020,
	0x040,
	0x060,
	0x080,
	0x0A0,
	0x0CA,
	0x0FE,
	0x17C,
	0x1FC,
	0x2FA,
	0x3F8,
	0x7F2,
	0xFE4
};

void PAPU::lock_mutex() {
	pthread_mutex_lock(&_mutex);
}

void PAPU::unlock_mutex() {
	pthread_mutex_unlock(&_mutex);
}

PAPU::PAPU(NES* nes) {
	pthread_mutex_init(&_mutex, nullptr);
	_is_running = false;

	channelEnableValue = 0;
	b1 = 0;
	b2 = 0;
	b3 = 0;
	b4 = 0;
	frameClockNow = false;
	masterFrameCounter = 0;
	derivedFrameCounter = 0;
	countSequence = 0;
	sampleTimer = 0;
	frameTime = 0;
	sampleTimerMax = 0;
	sampleCount = 0;
	sampleValueL = 0;
	sampleValueR = 0;
	smpSquare1 = 0;
	smpSquare2 = 0;
	smpTriangle = 0;
	smpNoise = 0;
	smpDmc = 0;
	accCount = 0;
	sq_index = 0;
	tnd_index = 0;

	// Stereo positioning:
	stereoPosLSquare1 = 0;
	stereoPosLSquare2 = 0;
	stereoPosLTriangle = 0;
	stereoPosLNoise = 0;
	stereoPosLDMC = 0;
	stereoPosRSquare1 = 0;
	stereoPosRSquare2 = 0;
	stereoPosRTriangle = 0;
	stereoPosRNoise = 0;
	stereoPosRDMC = 0;
	extraCycles = 0;
	maxCycles = 0;

	this->bufferSize = 2048;
	this->sampleRate = 44100;
	this->startedPlaying = false;
	this->recordOutput = false;
	this->stereo = true;
	this->initingHardware = false;
	this->userEnableSquare1 = true;
	this->userEnableSquare2 = true;
	this->userEnableTriangle = true;
	this->userEnableNoise = true;
	this->userEnableDmc = true;
	this->triValue = 0;
	this->prevSampleL = 0;
	this->prevSampleR = 0;
	this->smpAccumL = 0;
	this->smpAccumR = 0;
	this->smpDiffL = 0;
	this->smpDiffR = 0;
	this->dacRange = 0;
	this->dcValue = 0;

	this->nes = nes;
	cpuMem = nes->getCpuMemory();
	square_table = vector<int>(32 * 16, 0);
	tnd_table = vector<int>(204 * 16, 0);
	ready_for_buffer_write = false;

	lock_mutex();
	synchronized_setSampleRate(sampleRate, false);
	unlock_mutex();
	
	sampleBuffer = vector<uint8_t>(bufferSize * (stereo ? 4 : 2), 0);
	ismpbuffer = vector<int>(bufferSize * (stereo ? 2 : 1), 0);
	bufferIndex = 0;
	frameIrqEnabled = false;
	initCounter = 2048;

	square1 = new ChannelSquare(this, true);
	square2 = new ChannelSquare(this, false);
	triangle = new ChannelTriangle(this);
	noise = new ChannelNoise(this);
	dmc = new ChannelDM(this);

	masterVolume = 256;
	updateStereoPos();

	// Initialize lookup tables:
	initDACtables();

	frameIrqCounter = 0;
	frameIrqCounterMax = 4;
	
#ifdef SDL
	// Setup SDL for the format we want
	SDL_Init(SDL_INIT_AUDIO);
	SDL_AudioSpec desiredSpec;
	desiredSpec.freq = 44100;
	desiredSpec.format = AUDIO_S16SYS;
	desiredSpec.channels = 2;
	desiredSpec.samples = 4096;
	desiredSpec.callback = fill_audio_sdl_cb;
	desiredSpec.userdata = this;

	SDL_AudioSpec obtainedSpec;
	if(SDL_OpenAudio(&desiredSpec, &obtainedSpec) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(1);
	}
	/*cout << "freq: " << obtainedSpec.freq << endl;
	cout << "format: " << obtainedSpec.format << endl;
	cout << "channels: " << (s32) obtainedSpec.channels << endl;
	cout << "samples: " << obtainedSpec.samples << endl;
	cout << "callback: " << obtainedSpec.callback << endl;*/
#endif

#ifdef NACL
	frequency_ = 440;

	// Ask the device for an appropriate sample count size.
	const uint32_t desired_sample_frame_count = 2048; // Same as PAPU::bufferSize
	sample_frame_count_ = pp::AudioConfig::RecommendSampleFrameCount(
		this->nes->_salty_nes,
		PP_AUDIOSAMPLERATE_44100,
		desired_sample_frame_count
	);
	
	pp::AudioConfig config(
		this->nes->_salty_nes,
		PP_AUDIOSAMPLERATE_44100,
		sample_frame_count_
	);
	
	audio_ = pp::Audio(
		this->nes->_salty_nes,
		config,
		fill_audio_nacl_cb,
		this->nes->_salty_nes
	);
#endif
}

PAPU::~PAPU() {
	delete_n_null(square1);
	delete_n_null(square2);
	delete_n_null(triangle);
	delete_n_null(noise);
	delete_n_null(dmc);

	nes = nullptr;
	cpuMem = nullptr;

	pthread_mutex_destroy(&_mutex);
}

void PAPU::stateLoad(ByteBuffer* buf) {
	assert(buf);
	// not yet.
}

void PAPU::stateSave(ByteBuffer* buf) {
	assert(buf);
	// not yet.
}

void PAPU::synchronized_start() {
	_is_running = true;
	bufferIndex = 0;
	
//		Mixer.Info[] mixerInfo = AudioSystem.getMixerInfo();

//		if(mixerInfo == nullptr || mixerInfo.length == 0) {
//			//System.out.println("No audio mixer available, sound disabled.");
//			Globals::enableSound = false;
//			return;
//		}

//		mixer = AudioSystem.getMixer(mixerInfo[1]);

//		AudioFormat audioFormat = new AudioFormat(sampleRate, 16, (stereo ? 2 : 1), true, false);
//		DataLine.Info info = new DataLine.Info(SourceDataLine.class, audioFormat, sampleRate);

	try {

//			line = reinterperet_cast<SourceDataLine*>(AudioSystem.getLine(info));
//			line->open(audioFormat);
//			line->start();
		// Start running the stream
		#ifdef SDL
		SDL_PauseAudio(0);
		#endif
		
		#ifdef NACL
		audio_.StartPlayback();
		#endif

	} catch (exception& e) {
		//System.out.println("Couldn't get sound lines->");
	}

}

NES* PAPU::getNes() {
	return nes;
}

uint16_t PAPU::readReg() {
	// Read 0x4015:
	int tmp = 0;
	tmp |= (square1->getLengthStatus());
	tmp |= (square2->getLengthStatus() << 1);
	tmp |= (triangle->getLengthStatus() << 2);
	tmp |= (noise->getLengthStatus() << 3);
	tmp |= (dmc->getLengthStatus() << 4);
	tmp |= (((frameIrqActive && frameIrqEnabled) ? 1 : 0) << 6);
	tmp |= (dmc->getIrqStatus() << 7);

	frameIrqActive = false;
	dmc->irqGenerated = false;

	////System.out.println("$4015 read. Value = "+Misc.bin8(tmp)+" countseq = "+countSequence);
	return static_cast<uint16_t>(tmp);
}

void PAPU::writeReg(int address, uint16_t value) {
	if(address >= 0x4000 && address < 0x4004) {

		// Square Wave 1 Control
		square1->writeReg(address, value);
	////System.out.println("Square Write");

	} else if(address >= 0x4004 && address < 0x4008) {

		// Square 2 Control
		square2->writeReg(address, value);

	} else if(address >= 0x4008 && address < 0x400C) {

		// Triangle Control
		triangle->writeReg(address, value);

	} else if(address >= 0x400C && address <= 0x400F) {

		// Noise Control
		noise->writeReg(address, value);

	} else if(address == 0x4010) {

		// DMC Play mode & DMA frequency
		dmc->writeReg(address, value);

	} else if(address == 0x4011) {

		// DMC Delta Counter
		dmc->writeReg(address, value);

	} else if(address == 0x4012) {

		// DMC Play code starting address
		dmc->writeReg(address, value);

	} else if(address == 0x4013) {

		// DMC Play code length
		dmc->writeReg(address, value);

	} else if(address == 0x4015) {

		// Channel enable
		updateChannelEnable(value);

		if(value != 0 && initCounter > 0) {

			// Start hardware initialization
			initingHardware = true;

		}

		// DMC/IRQ Status
		dmc->writeReg(address, value);

	} else if(address == 0x4017) {


		// Frame counter control
		countSequence = (value >> 7) & 1;
		masterFrameCounter = 0;
		frameIrqActive = false;

		if(((value >> 6) & 0x1) == 0) {
			frameIrqEnabled = true;
		} else {
			frameIrqEnabled = false;
		}

		if(countSequence == 0) {

			// NTSC:
			frameIrqCounterMax = 4;
			derivedFrameCounter = 4;

		} else {

			// PAL:
			frameIrqCounterMax = 5;
			derivedFrameCounter = 0;
			frameCounterTick();

		}

	}
}

void PAPU::resetCounter() {
	if(countSequence == 0) {
		derivedFrameCounter = 4;
	} else {
		derivedFrameCounter = 0;
	}
}


// Updates channel enable status.
// This is done on writes to the
// channel enable register (0x4015),
// and when the user enables/disables channels
// in the GUI.
void PAPU::updateChannelEnable(int value) {
	channelEnableValue = static_cast<uint16_t>(value);
	square1->setEnabled(userEnableSquare1 && (value & 1) != 0);
	square2->setEnabled(userEnableSquare2 && (value & 2) != 0);
	triangle->setEnabled(userEnableTriangle && (value & 4) != 0);
	noise->setEnabled(userEnableNoise && (value & 8) != 0);
	dmc->setEnabled(userEnableDmc && (value & 16) != 0);
}

// Clocks the frame counter. It should be clocked at
// twice the cpu speed, so the cycles will be
// divided by 2 for those counters that are
// clocked at cpu speed.
void PAPU::clockFrameCounter(int nCycles) {
	if(initCounter > 0) {
		if(initingHardware) {
			initCounter -= nCycles;
			if(initCounter <= 0) {
				initingHardware = false;
			}
			return;
		}
	}

	// Don't process ticks beyond next sampling:
	nCycles += extraCycles;
	maxCycles = sampleTimerMax - sampleTimer;
	if((nCycles << 10) > maxCycles) {

		extraCycles = ((nCycles << 10) - maxCycles) >> 10;
		nCycles -= extraCycles;

	} else {

		extraCycles = 0;

	}

	// Clock DMC:
	if(dmc->isEnabled()) {

		dmc->shiftCounter -= (nCycles << 3);
		while(dmc->shiftCounter <= 0 && dmc->dmaFrequency > 0) {
			dmc->shiftCounter += dmc->dmaFrequency;
			dmc->clockDmc();
		}

	}

	// Clock Triangle channel Prog timer:
	if(triangle->progTimerMax > 0) {

		triangle->progTimerCount -= nCycles;
		while(triangle->progTimerCount <= 0) {

			triangle->progTimerCount += triangle->progTimerMax + 1;
			if(triangle->linearCounter > 0 && triangle->lengthCounter > 0) {

				++triangle->triangleCounter;
				triangle->triangleCounter &= 0x1F;

				if(triangle->isEnabled()) {
					if(triangle->triangleCounter >= 0x10) {
						// Normal value.
						triangle->sampleValue = (triangle->triangleCounter & 0xF);
					} else {
						// Inverted value.
						triangle->sampleValue = (0xF - (triangle->triangleCounter & 0xF));
					}
					triangle->sampleValue <<= 4;
				}

			}
		}

	}

	// Clock Square channel 1 Prog timer:
	square1->progTimerCount -= nCycles;
	if(square1->progTimerCount <= 0) {

		square1->progTimerCount += (square1->progTimerMax + 1) << 1;

		++square1->squareCounter;
		square1->squareCounter &= 0x7;
		square1->updateSampleValue();

	}

	// Clock Square channel 2 Prog timer:
	square2->progTimerCount -= nCycles;
	if(square2->progTimerCount <= 0) {

		square2->progTimerCount += (square2->progTimerMax + 1) << 1;

		++square2->squareCounter;
		square2->squareCounter &= 0x7;
		square2->updateSampleValue();

	}

	// Clock noise channel Prog timer:
	int acc_c = nCycles;
	if(noise->progTimerCount - acc_c > 0) {

		// Do all cycles at once:
		noise->progTimerCount -= acc_c;
		noise->accCount += acc_c;
		noise->accValue += acc_c * noise->sampleValue;

	} else {

		// Slow-step:
		while((acc_c--) > 0) {

			if(--noise->progTimerCount <= 0 && noise->progTimerMax > 0) {

				// Update noise shift register:
				noise->shiftReg <<= 1;
				noise->tmp = (((noise->shiftReg << (noise->randomMode == 0 ? 1 : 6)) ^ noise->shiftReg) & 0x8000);
				if(noise->tmp != 0) {

					// Sample value must be 0.
					noise->shiftReg |= 0x01;
					noise->randomBit = 0;
					noise->sampleValue = 0;

				} else {

					// Find sample value:
					noise->randomBit = 1;
					if(noise->isEnabled() && noise->lengthCounter > 0) {
						noise->sampleValue = noise->masterVolume;
					} else {
						noise->sampleValue = 0;
					}

				}

				noise->progTimerCount += noise->progTimerMax;

			}

			noise->accValue += noise->sampleValue;
			++noise->accCount;

		}
	}


	// Frame IRQ handling:
	if(frameIrqEnabled && frameIrqActive) {
		nes->cpu->requestIrq(CPU::IRQ_NORMAL);
	}

	// Clock frame counter at double CPU speed:
	masterFrameCounter += (nCycles << 1);
	if(masterFrameCounter >= frameTime) {

		// 240Hz tick:
		masterFrameCounter -= frameTime;
		frameCounterTick();


	}


	// Accumulate sample value:
	accSample(nCycles);


	// Clock sample timer:
	sampleTimer += nCycles << 10;
	if(sampleTimer >= sampleTimerMax) {

		// Sample channels:
		sample();
		sampleTimer -= sampleTimerMax;

	}
}

void PAPU::accSample(int cycles) {
	// Special treatment for triangle channel - need to interpolate.
	if(triangle->sampleCondition) {

		triValue = (triangle->progTimerCount << 4) / (triangle->progTimerMax + 1);
		if(triValue > 16) {
			triValue = 16;
		}
		if(triangle->triangleCounter >= 16) {
			triValue = 16 - triValue;
		}

		// Add non-interpolated sample value:
		triValue += triangle->sampleValue;

	}


	// Now sample normally:
	if(cycles == 2) {

		smpTriangle += triValue << 1;
		smpDmc += dmc->sample << 1;
		smpSquare1 += square1->sampleValue << 1;
		smpSquare2 += square2->sampleValue << 1;
		accCount += 2;

	} else if(cycles == 4) {

		smpTriangle += triValue << 2;
		smpDmc += dmc->sample << 2;
		smpSquare1 += square1->sampleValue << 2;
		smpSquare2 += square2->sampleValue << 2;
		accCount += 4;

	} else {

		smpTriangle += cycles * triValue;
		smpDmc += cycles * dmc->sample;
		smpSquare1 += cycles * square1->sampleValue;
		smpSquare2 += cycles * square2->sampleValue;
		accCount += cycles;
	}
}

void PAPU::frameCounterTick() {
	++derivedFrameCounter;
	if(derivedFrameCounter >= frameIrqCounterMax) {
		derivedFrameCounter = 0;
	}

	if(derivedFrameCounter == 1 || derivedFrameCounter == 3) {
		// Clock length & sweep:
		triangle->clockLengthCounter();
		square1->clockLengthCounter();
		square2->clockLengthCounter();
		noise->clockLengthCounter();
		square1->clockSweep();
		square2->clockSweep();
	}

	if(derivedFrameCounter >= 0 && derivedFrameCounter < 4) {
		// Clock linear & decay:
		square1->clockEnvDecay();
		square2->clockEnvDecay();
		noise->clockEnvDecay();
		triangle->clockLinearCounter();
	}

	if(derivedFrameCounter == 3 && countSequence == 0) {
		// Enable IRQ:
		frameIrqActive = true;
	}
// End of 240Hz tick
}


// Samples the channels, mixes the output together,
// writes to buffer and (if enabled) file.
void PAPU::sample() {
	if(accCount > 0) {
		smpSquare1 <<= 4;
		smpSquare1 /= accCount;

		smpSquare2 <<= 4;
		smpSquare2 /= accCount;

		smpTriangle /= accCount;

		smpDmc <<= 4;
		smpDmc /= accCount;

		accCount = 0;

	} else {

		smpSquare1 = square1->sampleValue << 4;
		smpSquare2 = square2->sampleValue << 4;
		smpTriangle = triangle->sampleValue;
		smpDmc = dmc->sample << 4;

	}

	smpNoise = static_cast<int>((noise->accValue << 4) / noise->accCount);
	noise->accValue = smpNoise >> 4;
	noise->accCount = 1;

	if(stereo) {

		// Stereo sound.

		// Left channel:
		sq_index = (smpSquare1 * stereoPosLSquare1 + smpSquare2 * stereoPosLSquare2) >> 8;
		tnd_index = (3 * smpTriangle * stereoPosLTriangle + (smpNoise << 1) * stereoPosLNoise + smpDmc * stereoPosLDMC) >> 8;
		if(sq_index >= static_cast<int>(square_table.size())) {
			sq_index = square_table.size() - 1;
		}
		if(tnd_index >= static_cast<int>(tnd_table.size())) {
			tnd_index = tnd_table.size() - 1;
		}
		sampleValueL = square_table[sq_index] + tnd_table[tnd_index] - dcValue;

		// Right channel:
		sq_index = (smpSquare1 * stereoPosRSquare1 + smpSquare2 * stereoPosRSquare2) >> 8;
		tnd_index = (3 * smpTriangle * stereoPosRTriangle + (smpNoise << 1) * stereoPosRNoise + smpDmc * stereoPosRDMC) >> 8;
		if(sq_index >= static_cast<int>(square_table.size())) {
			sq_index = square_table.size() - 1;
		}
		if(tnd_index >= static_cast<int>(tnd_table.size())) {
			tnd_index = tnd_table.size() - 1;
		}
		sampleValueR = square_table[sq_index] + tnd_table[tnd_index] - dcValue;

	} else {

		// Mono sound:
		sq_index = smpSquare1 + smpSquare2;
		tnd_index = 3 * smpTriangle + 2 * smpNoise + smpDmc;
		if(sq_index >= static_cast<int>(square_table.size())) {
			sq_index = square_table.size() - 1;
		}
		if(tnd_index >= static_cast<int>(tnd_table.size())) {
			tnd_index = tnd_table.size() - 1;
		}
		sampleValueL = 3 * (square_table[sq_index] + tnd_table[tnd_index] - dcValue);
		sampleValueL >>= 2;

	}

	// Remove DC from left channel:
	smpDiffL = sampleValueL - prevSampleL;
	prevSampleL += smpDiffL;
	smpAccumL += smpDiffL - (smpAccumL >> 10);
	sampleValueL = smpAccumL;

	if(stereo) {

		// Remove DC from right channel:
		smpDiffR = sampleValueR - prevSampleR;
		prevSampleR += smpDiffR;
		smpAccumR += smpDiffR - (smpAccumR >> 10);
		sampleValueR = smpAccumR;

		// Write:
		if(bufferIndex + 4 < static_cast<int>(sampleBuffer.size())) {
			sampleBuffer[bufferIndex++] = static_cast<uint8_t>((sampleValueL) & 0xFF);
			sampleBuffer[bufferIndex++] = static_cast<uint8_t>((sampleValueL >> 8) & 0xFF);
			sampleBuffer[bufferIndex++] = static_cast<uint8_t>((sampleValueR) & 0xFF);
			sampleBuffer[bufferIndex++] = static_cast<uint8_t>((sampleValueR >> 8) & 0xFF);

		}


	} else {

		// Write:
		if(bufferIndex + 2 < static_cast<int>(sampleBuffer.size())) {

			sampleBuffer[bufferIndex++] = static_cast<uint8_t>((sampleValueL) & 0xFF);
			sampleBuffer[bufferIndex++] = static_cast<uint8_t>((sampleValueL >> 8) & 0xFF);

		}

	}
	// Reset sampled values:
	smpSquare1 = 0;
	smpSquare2 = 0;
	smpTriangle = 0;
	smpDmc = 0;
}

// Writes the sound buffer to the output line:
void PAPU::writeBuffer() {
	bufferIndex -= (bufferIndex % (stereo ? 4 : 2));
	ready_for_buffer_write = true;
}

void PAPU::stop() {
#ifdef SDL
	SDL_PauseAudio(1);
#endif
#ifdef NACL
	audio_.StopPlayback();
#endif
	_is_running = false;
}

int PAPU::getSampleRate() {
	return sampleRate;
}

void PAPU::reset() {
	lock_mutex();
	synchronized_setSampleRate(sampleRate, false);
	unlock_mutex();
	
	updateChannelEnable(0);
	masterFrameCounter = 0;
	derivedFrameCounter = 0;
	countSequence = 0;
	sampleCount = 0;
	initCounter = 2048;
	frameIrqEnabled = false;
	initingHardware = false;

	resetCounter();

	square1->reset();
	square2->reset();
	triangle->reset();
	noise->reset();
	dmc->reset();

	bufferIndex = 0;
	accCount = 0;
	smpSquare1 = 0;
	smpSquare2 = 0;
	smpTriangle = 0;
	smpNoise = 0;
	smpDmc = 0;

	frameIrqEnabled = false;
	frameIrqCounterMax = 4;

	channelEnableValue = 0xFF;
	b1 = 0;
	b2 = 0;
	startedPlaying = false;
	sampleValueL = 0;
	sampleValueR = 0;
	prevSampleL = 0;
	prevSampleR = 0;
	smpAccumL = 0;
	smpAccumR = 0;
	smpDiffL = 0;
	smpDiffR = 0;

}

int PAPU::getLengthMax(int value) {
	return lengthLookup[value >> 3];
}

int PAPU::getDmcFrequency(int value) {
	if(value >= 0 && value < 0x10) {
		return dmcFreqLookup[value];
	}
	return 0;
}

int PAPU::getNoiseWaveLength(int value) {
	if(value >= 0 && value < 0x10) {
		return noiseWavelengthLookup[value];
	}
	return 0;
}

void PAPU::synchronized_setSampleRate(int rate, bool restart) {
	bool cpuRunning = nes->isRunning();
	if(cpuRunning) {
		nes->stopEmulation();
	}

	sampleRate = rate;
	sampleTimerMax = static_cast<int>((1024.0 * Globals::CPU_FREQ_NTSC * Globals::preferredFrameRate) /
			(sampleRate * 60.0));

	frameTime = static_cast<int>((14915.0 * static_cast<double>(Globals::preferredFrameRate)) / 60.0);

	sampleTimer = 0;
	bufferIndex = 0;

	if(restart) {
		stop();
		
		lock_mutex();
		synchronized_start();
		unlock_mutex();
	}

	if(cpuRunning) {
//			nes->startEmulation();
	}
}

void PAPU::synchronized_setStereo(bool s, bool restart) {
	if(stereo == s) {
		return;
	}

	bool running = nes->isRunning();
	nes->stopEmulation();

	stereo = s;
	if(stereo) {
		sampleBuffer.resize(bufferSize * 4);
	} else {
		sampleBuffer.resize(bufferSize * 2);
	}
	std::fill(sampleBuffer.begin(), sampleBuffer.end(), 0);

	if(restart) {
		stop();

		lock_mutex();
		synchronized_start();
		unlock_mutex();
	}

	if(running) {
//			nes->startEmulation();
	}
}

size_t PAPU::getPapuBufferSize() {
	return sampleBuffer.size();
}

void PAPU::setChannelEnabled(int channel, bool value) {
	if(channel == 0) {
		userEnableSquare1 = value;
	} else if(channel == 1) {
		userEnableSquare2 = value;
	} else if(channel == 2) {
		userEnableTriangle = value;
	} else if(channel == 3) {
		userEnableNoise = value;
	} else {
		userEnableDmc = value;
	}
	updateChannelEnable(channelEnableValue);
}

void PAPU::setMasterVolume(int value) {
	if(value < 0) {
		value = 0;
	}
	if(value > 256) {
		value = 256;
	}
	masterVolume = value;
	updateStereoPos();
}

void PAPU::updateStereoPos() {
	stereoPosLSquare1 = (panning[0] * masterVolume) >> 8;
	stereoPosLSquare2 = (panning[1] * masterVolume) >> 8;
	stereoPosLTriangle = (panning[2] * masterVolume) >> 8;
	stereoPosLNoise = (panning[3] * masterVolume) >> 8;
	stereoPosLDMC = (panning[4] * masterVolume) >> 8;

	stereoPosRSquare1 = masterVolume - stereoPosLSquare1;
	stereoPosRSquare2 = masterVolume - stereoPosLSquare2;
	stereoPosRTriangle = masterVolume - stereoPosLTriangle;
	stereoPosRNoise = masterVolume - stereoPosLNoise;
	stereoPosRDMC = masterVolume - stereoPosLDMC;
}

bool PAPU::isRunning() {
	return _is_running;
}

int PAPU::getMillisToAvailableAbove(int target_avail) {
	double time;
	int cur_avail;
	if((cur_avail = 0) >= target_avail) {
		return 0;
	}

	time = ((target_avail - cur_avail) * 1000) / sampleRate;
	time /= (stereo ? 4 : 2);

	return static_cast<int>(time);
}

int PAPU::getBufferPos() {
	return bufferIndex;
}

void PAPU::initDACtables() {
	double value;

	int ival;
	int max_sqr = 0;
	int max_tnd = 0;

	for(int i = 0; i < 32 * 16; ++i) {


		value = 95.52 / (8128.0 / (static_cast<double>(i) / 16.0) + 100.0);
		value *= 0.98411;
		value *= 50000.0;
		ival = static_cast<int>(value);

		square_table[i] = ival;
		if(ival > max_sqr) {
			max_sqr = ival;
		}

	}

	for(int i = 0; i < 204 * 16; ++i) {

		value = 163.67 / (24329.0 / (static_cast<double>(i) / 16.0) + 100.0);
		value *= 0.98411;
		value *= 50000.0;
		ival = static_cast<int>(value);

		tnd_table[i] = ival;
		if(ival > max_tnd) {
			max_tnd = ival;
		}

	}

	this->dacRange = max_sqr + max_tnd;
	this->dcValue = dacRange / 2;
}


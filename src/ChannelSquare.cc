/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

const int ChannelSquare::dutyLookup[32] = {
				0, 1, 0, 0, 0, 0, 0, 0,
				0, 1, 1, 0, 0, 0, 0, 0,
				0, 1, 1, 1, 1, 0, 0, 0,
				1, 0, 0, 1, 1, 1, 1, 1};

const int ChannelSquare::impLookup[32] = {
				1, -1, 0, 0, 0, 0, 0, 0,
				1, 0, -1, 0, 0, 0, 0, 0,
				1, 0, 0, 0, -1, 0, 0, 0,
				-1, 0, 1, 0, 0, 0, 0, 0};

ChannelSquare::ChannelSquare() {
}

ChannelSquare::ChannelSquare(shared_ptr<PAPU> papu, bool square1) {
	this->papu = papu;
	sqr1 = square1;
	_isEnabled = false;
	lengthCounterEnable = false;
	sweepActive = false;
	envDecayDisable = false;
	envDecayLoopEnable = false;
	envReset = false;
	sweepCarry = false;
	updateSweepPeriod = false;
	progTimerCount = 0;
	progTimerMax = 0;
	lengthCounter = 0;
	squareCounter = 0;
	sweepCounter = 0;
	sweepCounterMax = 0;
	sweepMode = 0;
	sweepShiftAmount = 0;
	envDecayRate = 0;
	envDecayCounter = 0;
	envVolume = 0;
	masterVolume = 0;
	dutyMode = 0;
	sweepResult = 0;
	sampleValue = 0;
	vol = 0;
}

ChannelSquare::~ChannelSquare() {
	papu = nullptr;
}

void ChannelSquare::clockLengthCounter() {
	if(lengthCounterEnable && lengthCounter > 0) {
		--lengthCounter;
		if(lengthCounter == 0) {
			updateSampleValue();
		}
	}

}

void ChannelSquare::clockEnvDecay() {
	if(envReset) {
		// Reset envelope:
		envReset = false;
		envDecayCounter = envDecayRate + 1;
		envVolume = 0xF;
	} else if((--envDecayCounter) <= 0) {
		// Normal handling:
		envDecayCounter = envDecayRate + 1;
		if(envVolume > 0) {
			--envVolume;
		} else {
			envVolume = envDecayLoopEnable ? 0xF : 0;
		}
	}

	masterVolume = envDecayDisable ? envDecayRate : envVolume;
	updateSampleValue();
}

void ChannelSquare::clockSweep() {
	if(--sweepCounter <= 0) {

		sweepCounter = sweepCounterMax + 1;
		if(sweepActive && sweepShiftAmount > 0 && progTimerMax > 7) {

			// Calculate result from shifter:
			sweepCarry = false;
			if(sweepMode == 0) {
				progTimerMax += (progTimerMax >> sweepShiftAmount);
				if(progTimerMax > 4095) {
					progTimerMax = 4095;
					sweepCarry = true;
				}
			} else {
				progTimerMax = progTimerMax - ((progTimerMax >> sweepShiftAmount) - (sqr1 ? 1 : 0));
			}

		}

	}

	if(updateSweepPeriod) {
		updateSweepPeriod = false;
		sweepCounter = sweepCounterMax + 1;
	}

}

void ChannelSquare::updateSampleValue() {

	if(_isEnabled && lengthCounter > 0 && progTimerMax > 7) {

		if(sweepMode == 0 && (progTimerMax + (progTimerMax >> sweepShiftAmount)) > 4095) {
			//if(sweepCarry) {

			sampleValue = 0;

		} else {

			sampleValue = masterVolume * dutyLookup[(dutyMode << 3) + squareCounter];

		}

	} else {

		sampleValue = 0;

	}

}

void ChannelSquare::writeReg(int address, int value) {

	int addrAdd = (sqr1 ? 0 : 4);
	if(address == 0x4000 + addrAdd) {

		// Volume/Envelope decay:
		envDecayDisable = ((value & 0x10) != 0);
		envDecayRate = value & 0xF;
		envDecayLoopEnable = ((value & 0x20) != 0);
		dutyMode = (value >> 6) & 0x3;
		lengthCounterEnable = ((value & 0x20) == 0);
		masterVolume = envDecayDisable ? envDecayRate : envVolume;
		updateSampleValue();

	} else if(address == 0x4001 + addrAdd) {

		// Sweep:
		sweepActive = ((value & 0x80) != 0);
		sweepCounterMax = ((value >> 4) & 7);
		sweepMode = (value >> 3) & 1;
		sweepShiftAmount = value & 7;
		updateSweepPeriod = true;

	} else if(address == 0x4002 + addrAdd) {

		// Programmable timer:
		progTimerMax &= 0x700;
		progTimerMax |= value;

	} else if(address == 0x4003 + addrAdd) {

		// Programmable timer, length counter
		progTimerMax &= 0xFF;
		progTimerMax |= ((value & 0x7) << 8);

		if(_isEnabled) {
			lengthCounter = papu->getLengthMax(value & 0xF8);
		}

		envReset = true;

	}

}

void ChannelSquare::setEnabled(bool value) {
	_isEnabled = value;
	if(!value) {
		lengthCounter = 0;
	}
	updateSampleValue();
}

bool ChannelSquare::isEnabled() {
	return _isEnabled;
}

int ChannelSquare::getLengthStatus() {
	return ((lengthCounter == 0 || !_isEnabled) ? 0 : 1);
}

void ChannelSquare::reset() {
	progTimerCount = 0;
	progTimerMax = 0;
	lengthCounter = 0;
	squareCounter = 0;
	sweepCounter = 0;
	sweepCounterMax = 0;
	sweepMode = 0;
	sweepShiftAmount = 0;
	envDecayRate = 0;
	envDecayCounter = 0;
	envVolume = 0;
	masterVolume = 0;
	dutyMode = 0;
	vol = 0;

	_isEnabled = false;
	lengthCounterEnable = false;
	sweepActive = false;
	sweepCarry = false;
	envDecayDisable = false;
	envDecayLoopEnable = false;
}

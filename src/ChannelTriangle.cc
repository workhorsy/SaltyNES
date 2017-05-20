/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

ChannelTriangle::ChannelTriangle() {
}

ChannelTriangle::ChannelTriangle(shared_ptr<PAPU> papu) {
	this->papu = papu;
	this->_isEnabled = false;
	this->sampleCondition = false;
	this->lengthCounterEnable = false;
	this->lcHalt = false;
	this->lcControl = false;
	this->progTimerCount = 0;
	this->progTimerMax = 0;
	this->triangleCounter = 0;
	this->lengthCounter = 0;
	this->linearCounter = 0;
	this->lcLoadValue = 0;
	this->sampleValue = 0;
	this->tmp = 0;
}

ChannelTriangle::~ChannelTriangle() {
	papu = nullptr;
}

void ChannelTriangle::clockLengthCounter() {
	if(lengthCounterEnable && lengthCounter > 0) {
		--lengthCounter;
		if(lengthCounter == 0) {
			updateSampleCondition();
		}
	}
}

void ChannelTriangle::clockLinearCounter() {
	if(lcHalt) {

		// Load:
		linearCounter = lcLoadValue;
		updateSampleCondition();

	} else if(linearCounter > 0) {

		// Decrement:
		--linearCounter;
		updateSampleCondition();

	}

	if(!lcControl) {

		// Clear halt flag:
		lcHalt = false;

	}

}

int ChannelTriangle::getLengthStatus() {
	return ((lengthCounter == 0 || !_isEnabled) ? 0 : 1);
}
/*
 int ChannelTriangle::readReg(int address) {
	return 0;
}
*/
void ChannelTriangle::writeReg(int address, int value) {
	if(address == 0x4008) {

		// New values for linear counter:
		lcControl = (value & 0x80) != 0;
		lcLoadValue = value & 0x7F;

		// Length counter enable:
		lengthCounterEnable = !lcControl;

	} else if(address == 0x400A) {

		// Programmable timer:
		progTimerMax &= 0x700;
		progTimerMax |= value;

	} else if(address == 0x400B) {

		// Programmable timer, length counter
		progTimerMax &= 0xFF;
		progTimerMax |= ((value & 0x07) << 8);
		lengthCounter = papu->getLengthMax(value & 0xF8);
		lcHalt = true;

	}

	updateSampleCondition();

}

void ChannelTriangle::clockProgrammableTimer(int nCycles) {
	if(progTimerMax > 0) {
		progTimerCount += nCycles;
		while(progTimerMax > 0 && progTimerCount >= progTimerMax) {
			progTimerCount -= progTimerMax;
			if(_isEnabled && lengthCounter > 0 && linearCounter > 0) {
				clockTriangleGenerator();
			}
		}
	}

}

void ChannelTriangle::clockTriangleGenerator() {
	++triangleCounter;
	triangleCounter &= 0x1F;
}

void ChannelTriangle::setEnabled(bool value) {
	_isEnabled = value;
	if(!value) {
		lengthCounter = 0;
	}
	updateSampleCondition();
}

bool ChannelTriangle::isEnabled() {
	return _isEnabled;
}

void ChannelTriangle::updateSampleCondition() {
	sampleCondition =
			_isEnabled &&
			progTimerMax > 7 &&
			linearCounter > 0 &&
			lengthCounter > 0;
}

void ChannelTriangle::reset() {
	progTimerCount = 0;
	progTimerMax = 0;
	triangleCounter = 0;
	_isEnabled = false;
	sampleCondition = false;
	lengthCounter = 0;
	lengthCounterEnable = false;
	linearCounter = 0;
	lcLoadValue = 0;
	lcHalt = true;
	lcControl = false;
	tmp = 0;
	sampleValue = 0xF;
}

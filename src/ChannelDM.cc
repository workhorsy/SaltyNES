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


ChannelDM::ChannelDM(PAPU* papu) {
	this->papu = papu;

	this->_isEnabled = false;
	this->hasSample = false;
	this->irqGenerated = false;
	this->playMode = 0;
	this->dmaFrequency = 0;
	this->dmaCounter = 0;
	this->deltaCounter = 0;
	this->playStartAddress = 0;
	this->playAddress = 0;
	this->playLength = 0;
	this->playLengthCounter = 0;
	this->shiftCounter = 0;
	this->reg4012 = 0;
	this->reg4013 = 0;
	this->status = 0;
	this->sample = 0;
	this->dacLsb = 0;
	this->data = 0;
}

ChannelDM::~ChannelDM() {
	papu = nullptr;
}

void ChannelDM::clockDmc() {

	// Only alter DAC value if the sample buffer has data:
	if(hasSample) {

		if((data & 1) == 0) {

			// Decrement delta:
			if(deltaCounter > 0) {
				deltaCounter--;
			}

		} else {

			// Increment delta:
			if(deltaCounter < 63) {
				++deltaCounter;
			}

		}

		// Update sample value:
		sample = _isEnabled ? (deltaCounter << 1) + dacLsb : 0;

		// Update shift register:
		data >>= 1;

	}

	dmaCounter--;
	if(dmaCounter <= 0) {

		// No more sample bits.
		hasSample = false;
		endOfSample();
		dmaCounter = 8;

	}

	if(irqGenerated) {
		papu->nes->cpu->requestIrq(CPU::IRQ_NORMAL);
	}

}

void ChannelDM::endOfSample() {
	if(playLengthCounter == 0 && playMode == MODE_LOOP) {

		// Start from beginning of sample:
		playAddress = playStartAddress;
		playLengthCounter = playLength;

	}

	if(playLengthCounter > 0) {

		// Fetch next sample:
		nextSample();

		if(playLengthCounter == 0) {

			// Last byte of sample fetched, generate IRQ:
			if(playMode == MODE_IRQ) {

				// Generate IRQ:
				irqGenerated = true;

			}

		}

	}

}

void ChannelDM::nextSample() {
	// Fetch byte:
	data = papu->getNes()->getMemoryMapper()->load(playAddress);
	papu->getNes()->cpu->haltCycles(4);

	--playLengthCounter;
	++playAddress;
	if(playAddress > 0xFFFF) {
		playAddress = 0x8000;
	}

	hasSample = true;
}

void ChannelDM::writeReg(int address, int value) {
	if(address == 0x4010) {
		// Play mode, DMA Frequency
		if((value >> 6) == 0) {
			playMode = MODE_NORMAL;
		} else if(((value >> 6) & 1) == 1) {
			playMode = MODE_LOOP;
		} else if((value >> 6) == 2) {
			playMode = MODE_IRQ;
		}

		if((value & 0x80) == 0) {
			irqGenerated = false;
		}

		dmaFrequency = papu->getDmcFrequency(value & 0xF);

	} else if(address == 0x4011) {
		// Delta counter load register:
		deltaCounter = (value >> 1) & 63;
		dacLsb = value & 1;
		if(papu->userEnableDmc) {
			sample = ((deltaCounter << 1) + dacLsb); // update sample value
		}

	} else if(address == 0x4012) {
		// DMA address load register
		playStartAddress = (value << 6) | 0x0C000;
		playAddress = playStartAddress;
		reg4012 = value;

	} else if(address == 0x4013) {
		// Length of play code
		playLength = (value << 4) + 1;
		playLengthCounter = playLength;
		reg4013 = value;

	} else if(address == 0x4015) {
		// DMC/IRQ Status
		if(((value >> 4) & 1) == 0) {
			// Disable:
			playLengthCounter = 0;
		} else {
			// Restart:
			playAddress = playStartAddress;
			playLengthCounter = playLength;
		}
		irqGenerated = false;
	}
}

void ChannelDM::setEnabled(bool value) {
	if((!_isEnabled) && value) {
		playLengthCounter = playLength;
	}
	_isEnabled = value;

}

bool ChannelDM::isEnabled() {
	return _isEnabled;
}

int ChannelDM::getLengthStatus() {
	return ((playLengthCounter == 0 || !_isEnabled) ? 0 : 1);
}

int ChannelDM::getIrqStatus() {
	return (irqGenerated ? 1 : 0);
}

void ChannelDM::reset() {
	_isEnabled = false;
	irqGenerated = false;
	playMode = MODE_NORMAL;
	dmaFrequency = 0;
	dmaCounter = 0;
	deltaCounter = 0;
	playStartAddress = 0;
	playAddress = 0;
	playLength = 0;
	playLengthCounter = 0;
	status = 0;
	sample = 0;
	dacLsb = 0;
	shiftCounter = 0;
	reg4012 = 0;
	reg4013 = 0;
	data = 0;
}

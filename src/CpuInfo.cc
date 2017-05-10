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

// Opdata array:
bool CpuInfo::isOp = false;
vector<int> CpuInfo::opdata;

// Instruction names:
const vector<string> CpuInfo::instname = {{
	"ADC",
	"AND",
	"ASL",
	"BCC",
	"BCS",
	"BEQ",
	"BIT",
	"BMI",
	"BNE",
	"BPL",
	"BRK",
	"BVC",
	"BVS",
	"CLC",
	"CLD",
	"CLI",
	"CLV",
	"CMP",
	"CPX",
	"CPY",
	"DEC",
	"DEX",
	"DEY",
	"EOR",
	"INC",
	"INX",
	"INY",
	"JMP",
	"JSR",
	"LDA",
	"LDX",
	"LDY",
	"LSR",
	"NOP",
	"ORA",
	"PHA",
	"PHP",
	"PLA",
	"PLP",
	"ROL",
	"ROR",
	"RTI",
	"RTS",
	"SBC",
	"SEC",
	"SED",
	"SEI",
	"STA",
	"STX",
	"STY",
	"TAX",
	"TAY",
	"TSX",
	"TXA",
	"TXS",
	"TYA"
}};

// Address mode descriptions:
const vector<string> CpuInfo::addrDesc = {{
	"Zero Page           ",
	"Relative            ",
	"Implied             ",
	"Absolute            ",
	"Accumulator         ",
	"Immediate           ",
	"Zero Page,X         ",
	"Zero Page,Y         ",
	"Absolute,X          ",
	"Absolute,Y          ",
	"Preindexed Indirect ",
	"Postindexed Indirect",
	"Indirect Absolute   "
}};

const vector<int> CpuInfo::cycTable = {
	/*0x00*/7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
	/*0x10*/ 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	/*0x20*/ 6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
	/*0x30*/ 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	/*0x40*/ 6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
	/*0x50*/ 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	/*0x60*/ 6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
	/*0x70*/ 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	/*0x80*/ 2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
	/*0x90*/ 2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
	/*0xA0*/ 2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
	/*0xB0*/ 2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
	/*0xC0*/ 2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
	/*0xD0*/ 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	/*0xE0*/ 2, 6, 3, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
	/*0xF0*/ 2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
};

// Instruction types:
// -------------------------------- //
const int CpuInfo::INS_ADC = 0;
const int CpuInfo::INS_AND = 1;
const int CpuInfo::INS_ASL = 2;
const int CpuInfo::INS_BCC = 3;
const int CpuInfo::INS_BCS = 4;
const int CpuInfo::INS_BEQ = 5;
const int CpuInfo::INS_BIT = 6;
const int CpuInfo::INS_BMI = 7;
const int CpuInfo::INS_BNE = 8;
const int CpuInfo::INS_BPL = 9;
const int CpuInfo::INS_BRK = 10;
const int CpuInfo::INS_BVC = 11;
const int CpuInfo::INS_BVS = 12;
const int CpuInfo::INS_CLC = 13;
const int CpuInfo::INS_CLD = 14;
const int CpuInfo::INS_CLI = 15;
const int CpuInfo::INS_CLV = 16;
const int CpuInfo::INS_CMP = 17;
const int CpuInfo::INS_CPX = 18;
const int CpuInfo::INS_CPY = 19;
const int CpuInfo::INS_DEC = 20;
const int CpuInfo::INS_DEX = 21;
const int CpuInfo::INS_DEY = 22;
const int CpuInfo::INS_EOR = 23;
const int CpuInfo::INS_INC = 24;
const int CpuInfo::INS_INX = 25;
const int CpuInfo::INS_INY = 26;
const int CpuInfo::INS_JMP = 27;
const int CpuInfo::INS_JSR = 28;
const int CpuInfo::INS_LDA = 29;
const int CpuInfo::INS_LDX = 30;
const int CpuInfo::INS_LDY = 31;
const int CpuInfo::INS_LSR = 32;
const int CpuInfo::INS_NOP = 33;
const int CpuInfo::INS_ORA = 34;
const int CpuInfo::INS_PHA = 35;
const int CpuInfo::INS_PHP = 36;
const int CpuInfo::INS_PLA = 37;
const int CpuInfo::INS_PLP = 38;
const int CpuInfo::INS_ROL = 39;
const int CpuInfo::INS_ROR = 40;
const int CpuInfo::INS_RTI = 41;
const int CpuInfo::INS_RTS = 42;
const int CpuInfo::INS_SBC = 43;
const int CpuInfo::INS_SEC = 44;
const int CpuInfo::INS_SED = 45;
const int CpuInfo::INS_SEI = 46;
const int CpuInfo::INS_STA = 47;
const int CpuInfo::INS_STX = 48;
const int CpuInfo::INS_STY = 49;
const int CpuInfo::INS_TAX = 50;
const int CpuInfo::INS_TAY = 51;
const int CpuInfo::INS_TSX = 52;
const int CpuInfo::INS_TXA = 53;
const int CpuInfo::INS_TXS = 54;
const int CpuInfo::INS_TYA = 55;
const int CpuInfo::INS_DUMMY = 56; // dummy instruction used for 'halting' the processor some cycles
// -------------------------------- //
// Addressing modes:
const int CpuInfo::ADDR_ZP = 0;
const int CpuInfo::ADDR_REL = 1;
const int CpuInfo::ADDR_IMP = 2;
const int CpuInfo::ADDR_ABS = 3;
const int CpuInfo::ADDR_ACC = 4;
const int CpuInfo::ADDR_IMM = 5;
const int CpuInfo::ADDR_ZPX = 6;
const int CpuInfo::ADDR_ZPY = 7;
const int CpuInfo::ADDR_ABSX = 8;
const int CpuInfo::ADDR_ABSY = 9;
const int CpuInfo::ADDR_PREIDXIND = 10;
const int CpuInfo::ADDR_POSTIDXIND = 11;
const int CpuInfo::ADDR_INDABS = 12;

vector<string> CpuInfo::getInstNames() {
	return instname;
}

string CpuInfo::getInstName(size_t inst) {
	if(inst < instname.size()) {
		return instname[inst];
	} else {
		return "???";
	}
}

vector<string> CpuInfo::getAddressModeNames() {
	return addrDesc;
}

string CpuInfo::getAddressModeName(int addrMode) {
	if(addrMode >= 0 && addrMode < static_cast<int>(addrDesc.size())) {
		return addrDesc[addrMode];
	}
	return "???";
}

void CpuInfo::initOpData() {
	if(isOp)
		return;

	isOp = true;

	// Set all to invalid instruction (to detect crashes):
	opdata = vector<int>(256, 0xFF);

	// Now fill in all valid opcodes:

	// ADC:
	setOp(INS_ADC, 0x69, ADDR_IMM, 2, 2);
	setOp(INS_ADC, 0x65, ADDR_ZP, 2, 3);
	setOp(INS_ADC, 0x75, ADDR_ZPX, 2, 4);
	setOp(INS_ADC, 0x6D, ADDR_ABS, 3, 4);
	setOp(INS_ADC, 0x7D, ADDR_ABSX, 3, 4);
	setOp(INS_ADC, 0x79, ADDR_ABSY, 3, 4);
	setOp(INS_ADC, 0x61, ADDR_PREIDXIND, 2, 6);
	setOp(INS_ADC, 0x71, ADDR_POSTIDXIND, 2, 5);

	// AND:
	setOp(INS_AND, 0x29, ADDR_IMM, 2, 2);
	setOp(INS_AND, 0x25, ADDR_ZP, 2, 3);
	setOp(INS_AND, 0x35, ADDR_ZPX, 2, 4);
	setOp(INS_AND, 0x2D, ADDR_ABS, 3, 4);
	setOp(INS_AND, 0x3D, ADDR_ABSX, 3, 4);
	setOp(INS_AND, 0x39, ADDR_ABSY, 3, 4);
	setOp(INS_AND, 0x21, ADDR_PREIDXIND, 2, 6);
	setOp(INS_AND, 0x31, ADDR_POSTIDXIND, 2, 5);

	// ASL:
	setOp(INS_ASL, 0x0A, ADDR_ACC, 1, 2);
	setOp(INS_ASL, 0x06, ADDR_ZP, 2, 5);
	setOp(INS_ASL, 0x16, ADDR_ZPX, 2, 6);
	setOp(INS_ASL, 0x0E, ADDR_ABS, 3, 6);
	setOp(INS_ASL, 0x1E, ADDR_ABSX, 3, 7);

	// BCC:
	setOp(INS_BCC, 0x90, ADDR_REL, 2, 2);

	// BCS:
	setOp(INS_BCS, 0xB0, ADDR_REL, 2, 2);

	// BEQ:
	setOp(INS_BEQ, 0xF0, ADDR_REL, 2, 2);

	// BIT:
	setOp(INS_BIT, 0x24, ADDR_ZP, 2, 3);
	setOp(INS_BIT, 0x2C, ADDR_ABS, 3, 4);

	// BMI:
	setOp(INS_BMI, 0x30, ADDR_REL, 2, 2);

	// BNE:
	setOp(INS_BNE, 0xD0, ADDR_REL, 2, 2);

	// BPL:
	setOp(INS_BPL, 0x10, ADDR_REL, 2, 2);

	// BRK:
	setOp(INS_BRK, 0x00, ADDR_IMP, 1, 7);

	// BVC:
	setOp(INS_BVC, 0x50, ADDR_REL, 2, 2);

	// BVS:
	setOp(INS_BVS, 0x70, ADDR_REL, 2, 2);

	// CLC:
	setOp(INS_CLC, 0x18, ADDR_IMP, 1, 2);

	// CLD:
	setOp(INS_CLD, 0xD8, ADDR_IMP, 1, 2);

	// CLI:
	setOp(INS_CLI, 0x58, ADDR_IMP, 1, 2);

	// CLV:
	setOp(INS_CLV, 0xB8, ADDR_IMP, 1, 2);

	// CMP:
	setOp(INS_CMP, 0xC9, ADDR_IMM, 2, 2);
	setOp(INS_CMP, 0xC5, ADDR_ZP, 2, 3);
	setOp(INS_CMP, 0xD5, ADDR_ZPX, 2, 4);
	setOp(INS_CMP, 0xCD, ADDR_ABS, 3, 4);
	setOp(INS_CMP, 0xDD, ADDR_ABSX, 3, 4);
	setOp(INS_CMP, 0xD9, ADDR_ABSY, 3, 4);
	setOp(INS_CMP, 0xC1, ADDR_PREIDXIND, 2, 6);
	setOp(INS_CMP, 0xD1, ADDR_POSTIDXIND, 2, 5);

	// CPX:
	setOp(INS_CPX, 0xE0, ADDR_IMM, 2, 2);
	setOp(INS_CPX, 0xE4, ADDR_ZP, 2, 3);
	setOp(INS_CPX, 0xEC, ADDR_ABS, 3, 4);

	// CPY:
	setOp(INS_CPY, 0xC0, ADDR_IMM, 2, 2);
	setOp(INS_CPY, 0xC4, ADDR_ZP, 2, 3);
	setOp(INS_CPY, 0xCC, ADDR_ABS, 3, 4);

	// DEC:
	setOp(INS_DEC, 0xC6, ADDR_ZP, 2, 5);
	setOp(INS_DEC, 0xD6, ADDR_ZPX, 2, 6);
	setOp(INS_DEC, 0xCE, ADDR_ABS, 3, 6);
	setOp(INS_DEC, 0xDE, ADDR_ABSX, 3, 7);

	// DEX:
	setOp(INS_DEX, 0xCA, ADDR_IMP, 1, 2);

	// DEY:
	setOp(INS_DEY, 0x88, ADDR_IMP, 1, 2);

	// EOR:
	setOp(INS_EOR, 0x49, ADDR_IMM, 2, 2);
	setOp(INS_EOR, 0x45, ADDR_ZP, 2, 3);
	setOp(INS_EOR, 0x55, ADDR_ZPX, 2, 4);
	setOp(INS_EOR, 0x4D, ADDR_ABS, 3, 4);
	setOp(INS_EOR, 0x5D, ADDR_ABSX, 3, 4);
	setOp(INS_EOR, 0x59, ADDR_ABSY, 3, 4);
	setOp(INS_EOR, 0x41, ADDR_PREIDXIND, 2, 6);
	setOp(INS_EOR, 0x51, ADDR_POSTIDXIND, 2, 5);

	// INC:
	setOp(INS_INC, 0xE6, ADDR_ZP, 2, 5);
	setOp(INS_INC, 0xF6, ADDR_ZPX, 2, 6);
	setOp(INS_INC, 0xEE, ADDR_ABS, 3, 6);
	setOp(INS_INC, 0xFE, ADDR_ABSX, 3, 7);

	// INX:
	setOp(INS_INX, 0xE8, ADDR_IMP, 1, 2);

	// INY:
	setOp(INS_INY, 0xC8, ADDR_IMP, 1, 2);

	// JMP:
	setOp(INS_JMP, 0x4C, ADDR_ABS, 3, 3);
	setOp(INS_JMP, 0x6C, ADDR_INDABS, 3, 5);

	// JSR:
	setOp(INS_JSR, 0x20, ADDR_ABS, 3, 6);

	// LDA:
	setOp(INS_LDA, 0xA9, ADDR_IMM, 2, 2);
	setOp(INS_LDA, 0xA5, ADDR_ZP, 2, 3);
	setOp(INS_LDA, 0xB5, ADDR_ZPX, 2, 4);
	setOp(INS_LDA, 0xAD, ADDR_ABS, 3, 4);
	setOp(INS_LDA, 0xBD, ADDR_ABSX, 3, 4);
	setOp(INS_LDA, 0xB9, ADDR_ABSY, 3, 4);
	setOp(INS_LDA, 0xA1, ADDR_PREIDXIND, 2, 6);
	setOp(INS_LDA, 0xB1, ADDR_POSTIDXIND, 2, 5);


	// LDX:
	setOp(INS_LDX, 0xA2, ADDR_IMM, 2, 2);
	setOp(INS_LDX, 0xA6, ADDR_ZP, 2, 3);
	setOp(INS_LDX, 0xB6, ADDR_ZPY, 2, 4);
	setOp(INS_LDX, 0xAE, ADDR_ABS, 3, 4);
	setOp(INS_LDX, 0xBE, ADDR_ABSY, 3, 4);

	// LDY:
	setOp(INS_LDY, 0xA0, ADDR_IMM, 2, 2);
	setOp(INS_LDY, 0xA4, ADDR_ZP, 2, 3);
	setOp(INS_LDY, 0xB4, ADDR_ZPX, 2, 4);
	setOp(INS_LDY, 0xAC, ADDR_ABS, 3, 4);
	setOp(INS_LDY, 0xBC, ADDR_ABSX, 3, 4);

	// LSR:
	setOp(INS_LSR, 0x4A, ADDR_ACC, 1, 2);
	setOp(INS_LSR, 0x46, ADDR_ZP, 2, 5);
	setOp(INS_LSR, 0x56, ADDR_ZPX, 2, 6);
	setOp(INS_LSR, 0x4E, ADDR_ABS, 3, 6);
	setOp(INS_LSR, 0x5E, ADDR_ABSX, 3, 7);

	// NOP:
	setOp(INS_NOP, 0xEA, ADDR_IMP, 1, 2);

	// ORA:
	setOp(INS_ORA, 0x09, ADDR_IMM, 2, 2);
	setOp(INS_ORA, 0x05, ADDR_ZP, 2, 3);
	setOp(INS_ORA, 0x15, ADDR_ZPX, 2, 4);
	setOp(INS_ORA, 0x0D, ADDR_ABS, 3, 4);
	setOp(INS_ORA, 0x1D, ADDR_ABSX, 3, 4);
	setOp(INS_ORA, 0x19, ADDR_ABSY, 3, 4);
	setOp(INS_ORA, 0x01, ADDR_PREIDXIND, 2, 6);
	setOp(INS_ORA, 0x11, ADDR_POSTIDXIND, 2, 5);

	// PHA:
	setOp(INS_PHA, 0x48, ADDR_IMP, 1, 3);

	// PHP:
	setOp(INS_PHP, 0x08, ADDR_IMP, 1, 3);

	// PLA:
	setOp(INS_PLA, 0x68, ADDR_IMP, 1, 4);

	// PLP:
	setOp(INS_PLP, 0x28, ADDR_IMP, 1, 4);

	// ROL:
	setOp(INS_ROL, 0x2A, ADDR_ACC, 1, 2);
	setOp(INS_ROL, 0x26, ADDR_ZP, 2, 5);
	setOp(INS_ROL, 0x36, ADDR_ZPX, 2, 6);
	setOp(INS_ROL, 0x2E, ADDR_ABS, 3, 6);
	setOp(INS_ROL, 0x3E, ADDR_ABSX, 3, 7);

	// ROR:
	setOp(INS_ROR, 0x6A, ADDR_ACC, 1, 2);
	setOp(INS_ROR, 0x66, ADDR_ZP, 2, 5);
	setOp(INS_ROR, 0x76, ADDR_ZPX, 2, 6);
	setOp(INS_ROR, 0x6E, ADDR_ABS, 3, 6);
	setOp(INS_ROR, 0x7E, ADDR_ABSX, 3, 7);

	// RTI:
	setOp(INS_RTI, 0x40, ADDR_IMP, 1, 6);

	// RTS:
	setOp(INS_RTS, 0x60, ADDR_IMP, 1, 6);

	// SBC:
	setOp(INS_SBC, 0xE9, ADDR_IMM, 2, 2);
	setOp(INS_SBC, 0xE5, ADDR_ZP, 2, 3);
	setOp(INS_SBC, 0xF5, ADDR_ZPX, 2, 4);
	setOp(INS_SBC, 0xED, ADDR_ABS, 3, 4);
	setOp(INS_SBC, 0xFD, ADDR_ABSX, 3, 4);
	setOp(INS_SBC, 0xF9, ADDR_ABSY, 3, 4);
	setOp(INS_SBC, 0xE1, ADDR_PREIDXIND, 2, 6);
	setOp(INS_SBC, 0xF1, ADDR_POSTIDXIND, 2, 5);

	// SEC:
	setOp(INS_SEC, 0x38, ADDR_IMP, 1, 2);

	// SED:
	setOp(INS_SED, 0xF8, ADDR_IMP, 1, 2);

	// SEI:
	setOp(INS_SEI, 0x78, ADDR_IMP, 1, 2);

	// STA:
	setOp(INS_STA, 0x85, ADDR_ZP, 2, 3);
	setOp(INS_STA, 0x95, ADDR_ZPX, 2, 4);
	setOp(INS_STA, 0x8D, ADDR_ABS, 3, 4);
	setOp(INS_STA, 0x9D, ADDR_ABSX, 3, 5);
	setOp(INS_STA, 0x99, ADDR_ABSY, 3, 5);
	setOp(INS_STA, 0x81, ADDR_PREIDXIND, 2, 6);
	setOp(INS_STA, 0x91, ADDR_POSTIDXIND, 2, 6);

	// STX:
	setOp(INS_STX, 0x86, ADDR_ZP, 2, 3);
	setOp(INS_STX, 0x96, ADDR_ZPY, 2, 4);
	setOp(INS_STX, 0x8E, ADDR_ABS, 3, 4);

	// STY:
	setOp(INS_STY, 0x84, ADDR_ZP, 2, 3);
	setOp(INS_STY, 0x94, ADDR_ZPX, 2, 4);
	setOp(INS_STY, 0x8C, ADDR_ABS, 3, 4);

	// TAX:
	setOp(INS_TAX, 0xAA, ADDR_IMP, 1, 2);

	// TAY:
	setOp(INS_TAY, 0xA8, ADDR_IMP, 1, 2);

	// TSX:
	setOp(INS_TSX, 0xBA, ADDR_IMP, 1, 2);

	// TXA:
	setOp(INS_TXA, 0x8A, ADDR_IMP, 1, 2);

	// TXS:
	setOp(INS_TXS, 0x9A, ADDR_IMP, 1, 2);

	// TYA:
	setOp(INS_TYA, 0x98, ADDR_IMP, 1, 2);
}

void CpuInfo::setOp(int inst, int op, int addr, int size, int cycles) {
	opdata[op] =
			((inst & 0xFF)) |
			((addr & 0xFF) << 8) |
			((size & 0xFF) << 16) |
			((cycles & 0xFF) << 24);
}



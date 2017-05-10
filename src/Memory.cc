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


Memory::Memory(NES* nes, size_t byteCount) {
	this->nes = nes;
	this->mem = vector<uint16_t>(byteCount, 0);
}

Memory::~Memory() {
	nes = nullptr;
}

void Memory::stateLoad(ByteBuffer* buf) {
	buf->readByteArray(&mem);
}

void Memory::stateSave(ByteBuffer* buf) {
	buf->putByteArray(&mem);
}

void Memory::reset() {
	std::fill(mem.begin(), mem.end(), 0);
}

size_t Memory::getMemSize() {
	return mem.size();
}

void Memory::write(size_t address, uint16_t value) {
	mem[address] = value;
}

uint16_t Memory::load(size_t address) {
	return mem[address];
}

void Memory::dump(string file) {
	dump(file, 0, mem.size());
}

void Memory::dump(string file, size_t offset, size_t length) {
	char* ch = new char[length];
	for(size_t i=0; i<length; ++i) {
		ch[i] = static_cast<char>(mem[offset + i]);
	}
	
	try{
        ofstream writer(file.c_str(), ios::out|ios::binary);
		writer.write(ch, length);
		writer.close();
		printf("Memory dumped to file \"%s\".\n", file.c_str());
		
	}catch(exception& ioe) {
		printf("%s\n", "Memory dump to file: IO Error!");
	}

	delete[] ch;
}

void Memory::write(size_t address, vector<uint16_t>* array, size_t length) {
	if(address+length > mem.size())
		return;
	arraycopy_short(array, 0, &mem, address, length);
}

void Memory::write(size_t address, vector<uint16_t>* array, size_t arrayoffset, size_t length) {
	if(address+length > mem.size())
		return;
	arraycopy_short(array, arrayoffset, &mem, address, length);
}



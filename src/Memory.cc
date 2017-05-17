/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"


Memory::Memory(shared_ptr<NES> nes, size_t byteCount) {
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

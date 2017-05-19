/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

vector<float>* Misc::_rnd = nullptr;
int Misc::nextRnd = 0;
float Misc::rndret = 0;

vector<float>* Misc::rnd() {
	if(_rnd == nullptr) {
		_rnd = new vector<float>(100000);
		for(size_t i = 0; i < _rnd->size(); ++i) {
			(*_rnd)[i] = rand_float();

		}
	}
	return _rnd;
}

string Misc::hex8(int i) {
	string s = intToHexString(i);
	while(s.length() < 2) {
		s = "0" + s;
	}
	return toUpperCase(s);
}

string Misc::hex16(int i) {
	string s = intToHexString(i);
	while(s.length() < 4) {
		s = "0" + s;
	}
	return toUpperCase(s);
}

string Misc::binN(int num, int N) {
	char* c = new char[N];
	for(int i = 0; i < N; ++i) {
		c[N - i - 1] = (num & 0x1) == 1 ? '1' : '0';
		num >>= 1;
	}
	return string(c);
}

string Misc::bin8(int num) {
	return binN(num, 8);
}

string Misc::bin16(int num) {
	return binN(num, 16);
}

string Misc::binStr(uint32_t value, int bitcount) {
	string ret = "";
	for(int i = 0; i < bitcount; ++i) {
		ret = ((value & (1 << i)) != 0 ? "1" : "0") + ret;
	}
	return ret;
}

string Misc::pad(string str, string padStr, int length) {
	while(static_cast<int>(str.length()) < length) {
		str += padStr;
	}
	return str;
}

float Misc::random() {
	rndret = (*rnd())[nextRnd];
	++nextRnd;
	if(nextRnd >= static_cast<int>(rnd()->size())) {
		nextRnd = static_cast<int>(rand_float() * (rnd()->size() - 1));
	}
	return rndret;
}

string Misc::from_vector_to_hex_string(array<uint16_t, 0x2000>* data) {
	const size_t BYTE_LEN = 4;
	stringstream out;
	for(size_t i=0; i<data->size(); ++i) {
		out << std::hex << setfill('0') << std::setw(BYTE_LEN) << static_cast<uint16_t>((*data)[i]);
	}
	return out.str();
}

vector<uint16_t>* Misc::from_hex_string_to_vector(string data) {
	const size_t BYTE_LEN = 4;
	const size_t VECTOR_SIZE = data.length() / BYTE_LEN;
	vector<uint16_t>* retval = new vector<uint16_t>(VECTOR_SIZE, 0);

	uint16_t value = 0;
	stringstream in;
	size_t j = 0;
	for(size_t i=0; i<VECTOR_SIZE; ++i) {
		in.clear();
		in << std::hex << data.substr(j, BYTE_LEN);
		in >> value;
		(*retval)[i] = value;
		j += BYTE_LEN;
	}

	return retval;
}

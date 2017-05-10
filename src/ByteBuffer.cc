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



ByteBuffer::ByteBuffer(size_t size, const int byteOrdering) {
	if(size < 1) {
		size = 1;
	}
	this->buf = vector<uint16_t>(size, 0);
	this->byteOrder = byteOrdering;
	curPos = 0;
	hasBeenErrors = false;
}

ByteBuffer::ByteBuffer(vector<uint8_t>* content, const int byteOrdering) {
	try {
		this->buf = vector<uint16_t>(content->size(), 0);
		for(size_t i = 0; i < content->size(); ++i) {
			buf[i] = static_cast<uint16_t>((*content)[i] & 255);
		}
		this->byteOrder = byteOrdering;
		curPos = 0;
		hasBeenErrors = false;
	} catch (exception& e) {
		//System.out.println("ByteBuffer: Couldn't create buffer from empty array.");
	}
}

void ByteBuffer::setExpandable(bool exp) {
	expandable = exp;
}

void ByteBuffer::setExpandBy(size_t expBy) {

	if(expBy > 1024) {
		this->expandBy = expBy;
	}

}

void ByteBuffer::setByteOrder(int byteOrder) {

	if(byteOrder >= 0 && byteOrder < 2) {
		this->byteOrder = byteOrder;
	}

}

uint8_t* ByteBuffer::getBytes() {
	return reinterpret_cast<uint8_t*>(this->buf.data());
}

size_t ByteBuffer::getSize() {
	return this->buf.size();
}

size_t ByteBuffer::getPos() {
	return curPos;
}

void ByteBuffer::error() {
	hasBeenErrors = true;
	printf("Not in range!\n");
}

bool ByteBuffer::hasHadErrors() {
	return hasBeenErrors;
}

void ByteBuffer::clear() {
	curPos = 0;
	std::fill(buf.begin(), buf.end(), 0);
}

void ByteBuffer::fill(uint8_t value) {
	std::fill(buf.begin(), buf.end(), value);
}

bool ByteBuffer::fillRange(size_t start, size_t length, uint8_t value) {
	if(inRange(start, length)) {
		for(size_t i = start; i < (start + length); ++i) {
			buf[i] = value;
		}
		return true;
	} else {
		error();
		return false;
	}
}

void ByteBuffer::resize(size_t length) {
	buf.resize(length);
}

void ByteBuffer::resizeToCurrentPos() {
	resize(curPos);
}

void ByteBuffer::expand() {
	expand(expandBy);
}

void ByteBuffer::expand(size_t byHowMuch) {
	resize(buf.size() + byHowMuch);
}

void ByteBuffer::goTo(size_t position) {
	if(inRange(position)) {
		curPos = position;
	} else {
		error();
	}
}

void ByteBuffer::move(size_t howFar) {
	curPos += howFar;
	if(!inRange(curPos)) {
		curPos = buf.size() - 1;
	}
}

bool ByteBuffer::inRange(size_t pos) {
	if(pos < buf.size()) {
		return true;
	} else {
		if(expandable) {
			expand(max(pos + 1 - buf.size(), expandBy));
			return true;
		} else {
			return false;
		}
	}
}

bool ByteBuffer::inRange(size_t pos, size_t length) {
	if(pos + (length - 1) < buf.size()) {
		return true;
	} else {
		if(expandable) {
			expand(max(pos + length - buf.size(), expandBy));
			return true;
		} else {
			return false;
		}
	}
}

bool ByteBuffer::putBoolean(bool b) {
	bool ret = putBoolean(b, curPos);
	move(1);
	return ret;
}

bool ByteBuffer::putBoolean(bool b, size_t pos) {
	if(b) {
		return putByte(static_cast<uint16_t>(1), pos);
	} else {
		return putByte(static_cast<uint16_t>(0), pos);
	}
}

bool ByteBuffer::putByte(uint16_t var) {
	if(inRange(curPos, 1)) {
		buf[curPos] = var;
		move(1);
		return true;
	} else {
		error();
		return false;
	}
}

bool ByteBuffer::putByte(uint16_t var, size_t pos) {
	if(inRange(pos, 1)) {
		buf[pos] = var;
		return true;
	} else {
		error();
		return false;
	}
}

bool ByteBuffer::putShort(uint16_t var) {
	bool ret = putShort(var, curPos);
	if(ret) {
		move(2);
	}
	return ret;
}

bool ByteBuffer::putShort(uint16_t var, size_t pos) {
	if(inRange(pos, 2)) {
		if(this->byteOrder == BO_BIG_ENDIAN) {
			buf[pos + 0] = static_cast<uint16_t>((var >> 8) & 255);
			buf[pos + 1] = static_cast<uint16_t>((var) & 255);
		} else {
			buf[pos + 1] = static_cast<uint16_t>((var >> 8) & 255);
			buf[pos + 0] = static_cast<uint16_t>((var) & 255);
		}
		return true;
	} else {
		error();
		return false;
	}
}

bool ByteBuffer::putInt(int var) {
	bool ret = putInt(var, curPos);
	if(ret) {
		move(4);
	}
	return ret;
}

bool ByteBuffer::putInt(int var, size_t pos) {
	if(inRange(pos, 4)) {
		if(this->byteOrder == BO_BIG_ENDIAN) {
			buf[pos + 0] = static_cast<uint16_t>((var >> 24) & 255);
			buf[pos + 1] = static_cast<uint16_t>((var >> 16) & 255);
			buf[pos + 2] = static_cast<uint16_t>((var >> 8) & 255);
			buf[pos + 3] = static_cast<uint16_t>(var & 255);
		} else {
			buf[pos + 3] = static_cast<uint16_t>((var >> 24) & 255);
			buf[pos + 2] = static_cast<uint16_t>((var >> 16) & 255);
			buf[pos + 1] = static_cast<uint16_t>((var >> 8) & 255);
			buf[pos + 0] = static_cast<uint16_t>(var & 255);
		}
		return true;
	} else {
		error();
		return false;
	}
}

bool ByteBuffer::putString(string var) {
	bool ret = putString(var, curPos);
	if(ret) {
		move(2 * var.length());
	}
	return ret;
}

bool ByteBuffer::putString(string var, size_t pos) {
	const char* charArr = reinterpret_cast<const char*>(var.c_str());
	uint16_t theChar;
	if(inRange(pos, var.length() * 2)) {
		for(size_t i = 0; i < var.length(); ++i) {
			theChar = static_cast<uint16_t>(charArr[i]);
			buf[pos + 0] = static_cast<uint16_t>((theChar >> 8) & 255);
			buf[pos + 1] = static_cast<uint16_t>(theChar & 255);
			pos += 2;
		}
		return true;
	} else {
		error();
		return false;
	}
}

bool ByteBuffer::putChar(char var) {
	bool ret = putChar(var, curPos);
	if(ret) {
		move(2);
	}
	return ret;
}

bool ByteBuffer::putChar(char var, size_t pos) {
	int tmp = var;
	if(inRange(pos, 2)) {
		if(byteOrder == BO_BIG_ENDIAN) {
			buf[pos + 0] = static_cast<uint16_t>((tmp >> 8) & 255);
			buf[pos + 1] = static_cast<uint16_t>(tmp & 255);
		} else {
			buf[pos + 1] = static_cast<uint16_t>((tmp >> 8) & 255);
			buf[pos + 0] = static_cast<uint16_t>(tmp & 255);
		}
		return true;
	} else {
		error();
		return false;
	}
}

bool ByteBuffer::putCharAscii(char var) {
	bool ret = putCharAscii(var, curPos);
	if(ret) {
		move(1);
	}
	return ret;
}

bool ByteBuffer::putCharAscii(char var, size_t pos) {
	if(inRange(pos)) {
		buf[pos] = static_cast<uint16_t>(var);
		return true;
	} else {
		error();
		return false;
	}
}

bool ByteBuffer::putStringAscii(string var) {
	bool ret = putStringAscii(var, curPos);
	if(ret) {
		move(var.length());
	}
	return ret;
}

bool ByteBuffer::putStringAscii(string var, size_t pos) {
	const char* charArr = reinterpret_cast<const char*>(var.c_str());
	if(inRange(pos, var.length())) {
		for(size_t i = 0; i < var.length(); ++i) {
			buf[pos] = static_cast<uint16_t>(charArr[i]);
			++pos;
		}
		return true;
	} else {
		error();
		return false;
	}
}

bool ByteBuffer::putByteArray(vector<uint16_t>* arr) {
	if(arr == nullptr) {
		return false;
	}
	if(buf.size() - curPos < arr->size()) {
		resize(curPos + arr->size());
	}
	for(size_t i = 0; i < arr->size(); ++i) {
		buf[curPos + i] = static_cast<uint8_t>((*arr)[i]);
	}
	curPos += arr->size();
	return true;
}

bool ByteBuffer::readByteArray(vector<uint16_t>* arr) {
	if(arr == nullptr) {
		return false;
	}
	if(buf.size() - curPos < arr->size()) {
		return false;
	}
	for(size_t i = 0; i < arr->size(); ++i) {
		(*arr)[i] = static_cast<uint16_t>(buf[curPos + i] & 0xFF);
	}
	curPos += arr->size();
	return true;
}

bool ByteBuffer::putShortArray(vector<uint16_t>* arr) {
	if(arr == nullptr) {
		return false;
	}
	if(buf.size() - curPos < arr->size() * 2) {
		resize(curPos + arr->size() * 2);
	}
	if(byteOrder == BO_BIG_ENDIAN) {
		for(size_t i = 0; i < arr->size(); ++i) {
			buf[curPos + 0] = static_cast<uint16_t>(((*arr)[i] >> 8) & 255);
			buf[curPos + 1] = static_cast<uint16_t>(((*arr)[i]) & 255);
			curPos += 2;
		}
	} else {
		for(size_t i = 0; i < arr->size(); ++i) {
			buf[curPos + 1] = static_cast<uint16_t>(((*arr)[i] >> 8) & 255);
			buf[curPos + 0] = static_cast<uint16_t>(((*arr)[i]) & 255);
			curPos += 2;
		}
	}
	return true;
}

string ByteBuffer::toString() {
	char* strBuf = new char(buf.size()-1);
	uint16_t tmp;
	for(size_t i = 0; i < (buf.size() - 1); i += 2) {
		tmp = static_cast<uint16_t>((buf[i] << 8) | (buf[i + 1]));
		strBuf[i] = static_cast<char>(tmp);
	}
	return string(strBuf);
}

string ByteBuffer::toStringAscii() {
	char* strBuf = new char(buf.size()-1);
	for(size_t i = 0; i < buf.size(); ++i) {
		strBuf[i] = static_cast<char>(buf[i]);
	}
	return string(strBuf);
}

bool ByteBuffer::readBoolean() {
	bool ret = readBoolean(curPos);
	move(1);
	return ret;
}

bool ByteBuffer::readBoolean(size_t pos) {
	return readByte(pos) == 1;
}

uint16_t ByteBuffer::readByte() {
	uint16_t ret = readByte(curPos);
	move(1);
	return ret;
}

uint16_t ByteBuffer::readByte(size_t pos) {
	if(inRange(pos)) {
		return buf[pos];
	} else {
		error();
		throw "ArrayIndexOutOfBoundsException";
	}
}

uint16_t ByteBuffer::readShort() {
	uint16_t ret = readShort(curPos);
	move(2);
	return ret;
}

uint16_t ByteBuffer::readShort(size_t pos) {
	if(inRange(pos, 2)) {
		if(this->byteOrder == BO_BIG_ENDIAN) {
			return static_cast<uint16_t>((buf[pos] << 8) | (buf[pos + 1]));
		} else {
			return static_cast<uint16_t>((buf[pos + 1] << 8) | (buf[pos]));
		}
	} else {
		error();
		throw "ArrayIndexOutOfBoundsException";
	}
}

int ByteBuffer::readInt() {
	int ret = readInt(curPos);
	move(4);
	return ret;
}

int ByteBuffer::readInt(size_t pos) {
	int ret = 0;
	if(inRange(pos, 4)) {
		if(this->byteOrder == BO_BIG_ENDIAN) {
			ret |= (buf[pos + 0] << 24);
			ret |= (buf[pos + 1] << 16);
			ret |= (buf[pos + 2] << 8);
			ret |= (buf[pos + 3]);
		} else {
			ret |= (buf[pos + 3] << 24);
			ret |= (buf[pos + 2] << 16);
			ret |= (buf[pos + 1] << 8);
			ret |= (buf[pos + 0]);
		}
		return ret;
	} else {
		error();
		throw "ArrayIndexOutOfBoundsException";
	}
}

char ByteBuffer::readChar() {
	char ret = readChar(curPos);
	move(2);
	return ret;
}

char ByteBuffer::readChar(size_t pos) {
	if(inRange(pos, 2)) {
		return static_cast<char>(readShort(pos));
	} else {
		error();
		throw "ArrayIndexOutOfBoundsException";
	}
}

char ByteBuffer::readCharAscii() {
	char ret = readCharAscii(curPos);
	move(1);
	return ret;
}

char ByteBuffer::readCharAscii(size_t pos) {
	if(inRange(pos, 1)) {
		return static_cast<char>(readByte(pos) & 255);
	} else {
		error();
		throw "ArrayIndexOutOfBoundsException";
	}
}

string ByteBuffer::readString(size_t length) {
	if(length > 0) {
		string ret = readString(curPos, length);
		move(ret.length() * 2);
		return ret;
	} else {
		return string("");
	}
}

string ByteBuffer::readString(size_t pos, size_t length) {
	char* tmp;
	if(inRange(pos, length * 2)) {
		tmp = new char[length];
		for(size_t i = 0; i < length; ++i) {
			tmp[i] = readChar(pos + i * 2);
		}
		return string(tmp);
	} else {
		throw "ArrayIndexOutOfBoundsException";
	}
}

string ByteBuffer::readStringWithShortLength() {
	string ret = readStringWithShortLength(curPos);
	move(ret.length() * 2 + 2);
	return ret;
}

string ByteBuffer::readStringWithShortLength(size_t pos) {
	uint16_t len;
	if(inRange(pos, 2)) {
		len = readShort(pos);
		if(len > 0) {
			return readString(pos + 2, len);
		} else {
			return string("");
		}
	} else {
		throw "ArrayIndexOutOfBoundsException";
	}
}

string ByteBuffer::readStringAscii(size_t length) {
	string ret = readStringAscii(curPos, length);
	move(ret.length());
	return ret;
}

string ByteBuffer::readStringAscii(size_t pos, size_t length) {
	char* tmp;
	if(inRange(pos, length) && length > 0) {
		tmp = new char[length];
		for(size_t i = 0; i < length; ++i) {
			tmp[i] = readCharAscii(pos + i);
		}
		return string(tmp);
	} else {
		throw "ArrayIndexOutOfBoundsException";
	}
}

string ByteBuffer::readStringAsciiWithShortLength() {
	string ret = readStringAsciiWithShortLength(curPos);
	move(ret.length() + 2);
	return ret;
}

string ByteBuffer::readStringAsciiWithShortLength(size_t pos) {
	uint16_t len;
	if(inRange(pos, 2)) {
		len = readShort(pos);
		if(len > 0) {
			return readStringAscii(pos + 2, len);
		} else {
			return string("");
		}
	} else {
		throw "ArrayIndexOutOfBoundsException";
	}
}
/*
vector<uint16_t>* ByteBuffer::expandShortArray(vector<uint16_t>* array, size_t size) {
	vector<uint16_t>* newArr = new vector<uint16_t>(array->size() + size, 0);
	for(size_t i=0; i<array->size(); ++i)
		(*newArr)[i] = (*array)[i];
}

void ByteBuffer::crop() {
	if(curPos > 0) {
		if(curPos < buf.size()) {
			buf.resize(curPos);
		}
	} else {
		//System.out.println("Could not crop buffer, as the current position is 0. The buffer may not be empty.");
	}
}

ByteBuffer* ByteBuffer::asciiEncode(ByteBuffer* buf) {

	vector<uint16_t>* data = &buf->buf;
	vector<uint8_t>* enc = new vector<uint8_t>(buf->getSize() * 2, 0);

	size_t encpos = 0;
	int tmp;
	for(size_t i = 0; i < data->size(); ++i) {

		tmp = (*data)[i];
		(*enc)[encpos] = static_cast<uint8_t>((65 + tmp) & 0xF);
		(*enc)[encpos + 1] = static_cast<uint8_t>((65 + (tmp >> 4)) & 0xF);
		encpos += 2;

	}
	return new ByteBuffer(enc, ByteBuffer::BO_BIG_ENDIAN);

}

ByteBuffer* ByteBuffer::asciiDecode(ByteBuffer* buf) {
	return nullptr;
}

void ByteBuffer::saveToZipFile(File f, ByteBuffer* buf) {

	try {

		FileOutputStream fOut = new FileOutputStream(f);
		ZipOutputStream zipOut = new ZipOutputStream(fOut);
		zipOut.putNextEntry(new ZipEntry("contents"));
		zipOut.write(buf->getBytes());
		zipOut.closeEntry();
		zipOut.close();
		fOut.close();
	//System.out.println("Buffer was successfully saved to "+f.getPath());

	} catch (Exception e) {

		//System.out.println("Unable to save buffer to file "+f.getPath());
		e.printStackTrace();

	}

}

static ByteBuffer* ByteBuffer::readFromZipFile(File f) {

	try {

		FileInputStream in = new FileInputStream(f);
		ZipInputStream zipIn = new ZipInputStream(in);
		int len, curlen, read;

		ZipFile zip = new ZipFile(f);
		ZipEntry entry = zip.getEntry("contents");
		len = (int) entry.getSize();
		//System.out.println("Len = "+len);

		curlen = 0;
		uint8_t* buf = new uint8_t[len];
		zipIn.getNextEntry();
		while(curlen < len) {
			read = zipIn.read(buf, curlen, len - curlen);
			if(read >= 0) {
				curlen += read;
			} else {
				// end of file.
				break;
			}
		}
		zipIn.closeEntry();
		zipIn.close();
		in.close();
		zip.close();
		return new ByteBuffer(buf, ByteBuffer.BO_BIG_ENDIAN);

	} catch (Exception e) {
		//System.out.println("Unable to load buffer from file "+f.getPath());
		e.printStackTrace();
	}

	// fail:
	return nullptr;

}
*/


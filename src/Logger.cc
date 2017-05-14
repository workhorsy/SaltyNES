/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#include "SaltyNES.h"

#include <fstream>

using namespace std;


string Logger::_file_name;
ofstream* Logger::_log = nullptr;
size_t Logger::_length = 0;
size_t Logger::_log_number = 0;
bool Logger::_is_on = false;

void Logger::init(string file_name) {
	if(!Logger::_is_on) return;

	// Create the first log file
	_file_name = file_name;
	_log_number = 0;
	create_log_file();
}

void Logger::create_log_file() {
	if(!Logger::_is_on) return;

	// Create the next numbered log name
	stringstream out;
	out << _file_name << _log_number;

	// Create the file
	if(_log)
		_log->close();
	_log = new ofstream();
	_log->open(out.str().c_str());
}

void Logger::write(string message) {
	if(!Logger::_is_on) return;

	// Write the message to the log
	(*_log) << message;
	_length += message.length();
}

void Logger::flush() {
	if(!Logger::_is_on) return;

	_log->flush();

	// If the log file is too big, make a new one
	if(_length >= 10000000) {
		_length = 0;
		++_log_number;
		create_log_file();
	}
}

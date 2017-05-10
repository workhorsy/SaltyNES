/*
SaltyNES Copyright (c) 2012-2014 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>

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

#include <iostream>
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


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

#ifdef NACL

#include <ppapi/cpp/module.h>

#include "SaltyNES.h"

// The Module class.  The browser calls the CreateInstance() method to create
// an instance of your NaCl module on the web page.  The browser creates a new
// instance for each <embed> tag with type="application/x-nacl".
class SaltyNESModule : public pp::Module {
public:
	SaltyNESModule() : pp::Module() {}
	virtual ~SaltyNESModule() {}

	// Create and return a SaltyNESInstance object.
	virtual pp::Instance* CreateInstance(PP_Instance instance) {
		return new SaltyNES(instance);
	}
};

// Factory function called by the browser when the module is first loaded.
// The browser keeps a singleton of this module.  It calls the
// CreateInstance() method on the object you return to make instances.  There
// is one instance per <embed> tag on the page.  This is the main binding
// point for your NaCl module with the browser.
namespace pp {
	Module* CreateModule() {
		return new SaltyNESModule();
	}
}

#endif


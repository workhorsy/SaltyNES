
#include <sstream>
#include <string>

string get_build_date() {
	stringstream ss;
	ss << "Build date: " <<  __DATE__ << ", " << __TIME__;
	return ss.str();
}

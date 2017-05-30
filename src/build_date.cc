
#include <sstream>
#include <string>

std::string get_build_date() {
	std::stringstream ss;
	ss << "Build date: " <<  __DATE__ << ", " << __TIME__;
	return ss.str();
}

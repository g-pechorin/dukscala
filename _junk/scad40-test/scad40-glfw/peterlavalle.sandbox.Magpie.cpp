
#include <functional>
#include "D40.hpp"

peterlavalle::magpie::System::System(void)
{
	// nothing to do here
}

void peterlavalle::magpie::System::err(scad40::duk_string& message)
{
	std::cerr << message.c_str() << std::endl;
}

void peterlavalle::magpie::System::out(scad40::duk_string& message)
{
	std::cout << message.c_str() << std::endl;
}

bool peterlavalle::magpie::System::require(scad40::duk_string& path)
{
	std::string filePath = "var/";
#ifdef _DEBUG
	filePath = CMAKE_SOURCE_DIR "/var/";
#else
	filePath = "var/";
#endif
	filePath = filePath + path.c_str() + ".js";

	if (duk_peval_file(Host(), filePath.c_str()))
	{
		std::cout << "Error: `" << duk_safe_to_string(Host(), -1) << "`" << std::endl;
		return false;
	}
	duk_pop(Host());  /* ignore result */
	return true;
}

peterlavalle::magpie::System::~System(void)
{
	// nothing to do here
}
#include <iostream>

#include "palbl.h"

int main(int argc, char* argv[])
{

	std::cout << "Hello World" << std::endl;

	const char* source = "[tag][/tag]";

	auto begin = [](void* global, void*parent, const palbl_string_t tag) -> void*
	{
		std::cout << static_cast<std::string>(tag) << ">" << std::endl;
		return nullptr;
	};
	auto close = [](void* global, void*parent, const palbl_string_t tag, void* self) -> void*
	{
		std::cout << static_cast<std::string>(tag) << "<" << std::endl;
		return nullptr;
	};

	palbl_load(palbl_string_t(source), 0, nullptr, nullptr, begin, nullptr, close);

	return EXIT_SUCCESS;
}

#define PALBL_C
#include "palbl.h"
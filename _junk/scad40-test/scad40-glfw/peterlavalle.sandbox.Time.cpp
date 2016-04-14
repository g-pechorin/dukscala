
#include <functional>
#include "D40.hpp"

#include <GLFW/glfw3.h>

peterlavalle::magpie::Time::Time(void)
{
	_now = (float)glfwGetTime();
	_delta = 0.0f;
}

peterlavalle::magpie::Time::~Time(void)
{
	// nothing to do here
}
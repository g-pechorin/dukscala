#pragma once


#include <GLFW/glfw3.h>
#include <queue>

class glfw_queue
{
	std::queue<GLFWwindow*> _windows;
	void(*_tick)(GLFWwindow*);
public:
	glfw_queue(void(*tick)(GLFWwindow*)) :
		_tick(tick)
	{
	}

	bool step(void)
	{
		if (_windows.empty())
		{
			return false;
		}

		GLFWwindow* window = _windows.front();
		_windows.pop();

		glfwMakeContextCurrent(window);
		if (glfwWindowShouldClose(window))
		{
			glfwDestroyWindow(window);
		}
		else
		{
			_tick(window);

			glfwSwapBuffers(window);
			glfwPollEvents();
			_windows.push(window);
		}

		return true;
	}



	GLFWwindow* create(size_t width, size_t height, const char* name, void* userdata)
	{
		GLFWwindow* window = glfwCreateWindow(width, height, name, nullptr, nullptr);

		if (window)
		{
			_windows.push(window);
			glfwSetWindowUserPointer(window, userdata);
			glfwMakeContextCurrent(window);
			glfwSwapInterval(1);
		}

		return window;
	}
};

#include <iostream>

#include "glfw_queue.hpp"

#include <GLFW/glfw3.h>
#include <queue>

#include <functional>
#include "D40.hpp"

void main(int argc, char* argv[])
{
	glfwSetErrorCallback([](int error, const char* message)
	{
		std::cerr << "GLFW[" << std::hex << error << "] = `" << message << "`";
		exit(EXIT_FAILURE);
	});

	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}
	atexit(glfwTerminate);

	glfw_queue queue([](GLFWwindow* window)
	{
		auto ctx = (duk_context*)glfwGetWindowUserPointer(window);

		auto& time = peterlavalle::magpie::Time::get(ctx);
		{
			const float next = glfwGetTime();
			time._delta = next - time._now;
			time._now = next;
		}


		// peterlavalle::magpie::

		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef((float)glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
		glBegin(GL_TRIANGLES);
		{
			glColor3f(1.f, 0.f, 0.f);
			glVertex3f(-0.6f, -0.4f, 0.f);
			glColor3f(0.f, 1.f, 0.f);
			glVertex3f(0.6f, -0.4f, 0.f);
			glColor3f(0.f, 0.f, 1.f);
			glVertex3f(0.f, 0.6f, 0.f);
		}
		glEnd();
	});

	// create a context
	auto ctx = duk_create_heap_default();

	{

		// wipeout the world
		duk_push_object(ctx);
		duk_set_global_object(ctx);

		// setup the loadering thing
		peterlavalle::magpie::install(ctx);

		if (!peterlavalle::magpie::System::get(ctx).require("demo"))
		{
			std::cerr << "Failed to load demo script" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	{
		//
		GLFWwindow* window = queue.create(640, 480, "Simple example", ctx);
		if (!window)
		{
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
				glfwSetWindowShouldClose(window, GL_TRUE);
		});
	}

	auto hamster = peterlavalle::magpie::Listener::New(ctx, "Hamster");

	hamster->onStart();
	while (queue.step())
	{
		hamster->onUpdate();
	}
	hamster->onClose();

	duk_destroy_heap(ctx);

	exit(EXIT_SUCCESS);
}
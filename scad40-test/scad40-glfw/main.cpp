#include <iostream>

#include "glfw_queue.hpp"

#include <GLFW/glfw3.h>
#include <queue>

void main(int argc, char* argv[])
{
	std::cout << "Hello World" << std::endl;

	glfwSetErrorCallback([](int error, const char* message)
	{
		std::cerr << "GLFW[" << std::hex << error << "] = `" << message << "`";
		exit(EXIT_FAILURE);
	});

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfw_queue queue([](GLFWwindow* window)
	{
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

	for (int i = 0; i < 3; ++i)
	{
		GLFWwindow* window = queue.create(640, 480, "Simple example", nullptr);
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




	while (queue.step())
	{
		;
	}


	glfwTerminate();
	exit(EXIT_SUCCESS);
}
#include "glSetup.h"
#include <iostream>

void	render(GLFWwindow* window);
void	keyboard(GLFWwindow* window, int key, int code, int action, int mods);
	
// Colors
GLfloat bgColor[4] = { 1, 1, 1, 1 };

int
main(int argc, char* argv[])
{
	// Orthographics viewing
	perspectiveView = false;

	// Initialize the OpenGL system
	GLFWwindow* window = initializeOpenGL(argc, argv, bgColor);
	if (window == NULL) return -1;

	// Callbacks
	glfwSetKeyCallback(window, keyboard);

	// Viewport and perspective setting
	reshape(window, windowW, windowH);

	// Usage
	std::cout << std::endl;
	std::cout << "Keyboard Input: q for quit" << std::endl;

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		render(window);				//Draw one frame
		glfwSwapBuffers(window);	//Swap buffers 
		glfwPollEvents();			//Events
	}

	// Terminate the glfw system
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void
render(GLFWwindow* window)
{
	// Background color
	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLineWidth(5 * dpiScaling);

	// Draw a rectangle using GL_LINE_LOOP
	glBegin(GL_LINES);
	{
/*
		glColor3f(1, 0, 0);	glVertex2f(-0.5, -0.5);
		glColor3f(0, 1, 0);	glVertex2f(0.5, -0.5);
		glColor3f(0, 0, 1);	glVertex2f(0.5, 0.5);
		glColor3f(0, 0, 0); glVertex2f(-0.5, 0.5);
*/		

		glColor3f(1, 0, 0);	glVertex2f(-0.5, -0.5);
		glColor3f(0, 1, 0);	glVertex2f(0.5, -0.5);

		glColor3f(0, 1, 0);	glVertex2f(0.5, -0.5);
		glColor3f(0, 0, 1);	glVertex2f(0.5, 0.5);

		glColor3f(0, 0, 1);	glVertex2f(0.5, 0.5);
		glColor3f(0, 0, 0); glVertex2f(-0.5, 0.5);

		glColor3f(0, 0, 0); glVertex2f(-0.5, 0.5);
		glColor3f(1, 0, 0);	glVertex2f(-0.5, -0.5);

	}
	glEnd();
}

void
keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)

{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch (key)
		{
			// Quit
		case GLFW_KEY_Q:
		case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
		}
	}
}
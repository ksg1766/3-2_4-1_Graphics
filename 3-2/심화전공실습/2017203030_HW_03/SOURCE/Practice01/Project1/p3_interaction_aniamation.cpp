#include "glSetup.h"
#include <iostream>
using namespace::std;

#ifdef _WIN32
#define _USE_MATH_DEFINES	// To include the definition of M_PI in math. h
#endif

#include <math.h>

void	render(GLFWwindow* window);
void	keyboard(GLFWwindow* window, int key, int code, int action, int mods);
void	mouseButton(GLFWwindow* window, int button, int action, int mods);
void	mouseMove(GLFWwindow* window, double x, double y);

// Global coordinate frame
float AXIS_LENGTH = 0.5;
float AXIS_LINE_WIDTH = 2.0;

// Colors
GLfloat bgColor[4] = { 1, 1, 1, 1 };
// Endpoints of the line segment
float point[3][2] = { {0, 0}, {0, 0}, {0, 0} };
int pressCount = 0;

// Mouse input mode
enum class InputMode
{
	NONE = 0,
	DRAGGING = 1,
	COMPLETE = 2,
};

InputMode	inputMode = InputMode::NONE;

// Paly configuration
bool	pause = true;

float timeStep = 1.0f / 120;	// 120fps
float period = 8.0;

void
rotate(float p[2], float delta_theta)
{
	float x = p[0];
	float y = p[1];

	float	cos_delta_theta = cos(delta_theta);
	float	sin_delta_theta = sin(delta_theta);

	p[0] = cos_delta_theta * x - sin_delta_theta * y;
	p[1] = sin_delta_theta * x + cos_delta_theta * y;
}

int
main(int argc, char* argv[])
{
	// Orthographic projection
	perspectiveView = false;

	// FPS control: vsync should be e for precise time stepping
	vsync = 0; // 0 for immediate mode (Tearing possible)

	// Initialize the OpenGL system
	GLFWwindow* window = initializeOpenGL(argc, argv, bgColor);
	if (window == NULL) return -1;

	// Callbacks
	glfwSetKeyCallback(window, keyboard);
	glfwSetMouseButtonCallback(window, mouseButton);
	glfwSetCursorPosCallback(window, mouseMove);

	// Viewport and perspective setting
	reshape(window, windowW, windowH);

	// Usage
	cout << endl;
	cout << "Mouse button down:	start point of the line segment" << endl;
	cout << "Mouse dragging:	changes the line segment" << endl;
	cout << "Mouse button up:	end point of the line segment" << endl;
	cout << "Keyboard input:	space for play/pause" << endl;
	cout << "Keyboard input:	up for increasing period" << endl;
	cout << "Keyboard input:	down for decreasing period" << endl;

	// Main loop
	float	previous = (float)glfwGetTime();
	float	elapsed = 0;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();	// Events

		// Time passed during a single loop
		float now = (float)glfwGetTime();
		float delta_t = now - previous;
		previous = now;

		// Time passed after the previous frame
		elapsed += delta_t;
		//cout << "elapsed = " << elapsed * leee << ms " << endl;

			// Deal with the current frame
		if (elapsed > timeStep)
		{
			// Rotate the line segment if not paused
			if (!pause && inputMode == InputMode::COMPLETE)
			{
				// One rotation per period
				float delta_theta = float(2.0 * M_PI) / period * elapsed;

				rotate(point[0], delta_theta);
				rotate(point[1], delta_theta);
			}

			//cout << "\telapsed = " << elapsed * 1000 << " ms" << endl;
			//cout << "\ttimeStep =	<< timeStep * 1000 << " ms" << endl;
			elapsed = 0;	// Reset the elapsed time
		//cout << endl;
		}

		render(window);	// Draw one frame
		glfwSwapBuffers(window);	// Swap buffers
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

	// Axes
	glDisable(GL_LINE_STIPPLE);
	drawAxes(AXIS_LENGTH, AXIS_LINE_WIDTH * dpiScaling);
	// Attribute of the line segment
	glColor3f(0, 0, 1);
	glLineWidth(5 * dpiScaling);

	// Desired position of the end-effector
	if (inputMode == InputMode::DRAGGING)
	{
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(int(3 * dpiScaling), 0xcccc);
	}
	else glDisable(GL_LINE_STIPPLE);

	if (inputMode > InputMode::NONE)
	{
		glBegin(GL_LINE_LOOP);
		glVertex2f(point[0][0], point[0][1]);
		glVertex2f(point[1][0], point[1][1]);
		glVertex2f(point[2][0], point[2][1]);
		glEnd();
	}
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

			// Play on/off
		case GLFW_KEY_SPACE: pause = !pause; break;

			// Period control
		case GLFW_KEY_DOWN: period = (float)max(period - 0.1, 0.1);  break;
		case GLFW_KEY_UP: period += 0.1f; break;
		}
	}
}
// How about using gluUnproject(xs, ys, 0.0, ...) 
void
screen2world(float xs, float ys, float& xw, float& yw)
{
	// In the world space. See reshape() in glSetup. cpp
	float	aspect = (float)screenW / screenH;
	xw = 2.0f * (xs / (screenW - 1) - 0.5f) * aspect;
	yw = -2.0f * (ys / (screenH - 1) - 0.5f);
}

void
mouseButton(GLFWwindow* window, int button, int action, int mods)
{
	// Mouse cursor position in the screen coordinate system
	double xs, ys;
	glfwGetCursorPos(window, &xs, &ys);

	// Screen coordinate 2 world coordinate conversion
	float xw, yw;
	screen2world((float)xs, (float)ys, xw, yw);

	// The left button of the mouse is pressed.
	
	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT)
	{
		inputMode = InputMode::DRAGGING;	// Dragging starts
		if (pressCount == 0) {
			point[0][0] = xw;	point[0][1] = yw;	// Start point
			point[1][0] = xw;	point[1][1] = yw;	// End point
			pressCount++;
		}
		else{
			point[1][0] = xw;	point[1][1] = yw;	// Start point
			point[2][0] = xw;	point[2][1] = yw;	// End point
		}
	}

	//The left button of the mouse is released.
	if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT)
	{
		inputMode = InputMode::COMPLETE;	// Dragging ends

		point[1][0] = xw;	point[1][1] = yw;	// End point
	}
}

void
mouseMove(GLFWwindow* window, double x, double y)
{
	// Update the end point while dragging
	if (inputMode == InputMode::DRAGGING)
	{
		// Screen coordinate 2 world coordinate conversion
		screen2world((float)x, (float)y, point[1][0], point[1][1]);
	}
}
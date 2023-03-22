#include "glSetup.h"

#ifdef	_WIN32
#endif

#include <glm/glm.hpp>		// OpenGL Mathematics
#include <glm/gtc/type_ptr.hpp> // glm: : value_ptr()
using namespace glm;

#include <iostream>
using namespace std;

void	init();
void	quit();
void	render(GLFWwindow* window);
void	keyboard(GLFWwindow* window, int key, int code, int action, int mods);

// Camera configuation
vec3	eye(3.5, 3, 3.5);
vec3	center(0, 0, 0);
vec3	up(0, 1, 0);

// Global coordinate frame
float AXIS_LENGTH = 3;
float AXIS_LINE_WIDTH = 2;

// Colors
GLfloat bgColor[4] = { 1, 1, 1, 1 };

// Selected example
int selection = 1;

bool	depthTest = true;
bool	polygonFill = true;

int		sweepAngle = 36;
glm::vec3	p[37][19];

void quit()
{}

int
main(int argc, char* argv[])
{
	// vsync should be 0 for precise time stepping.
	vsync = 0;

	// Initialize the OpenGL system
	GLFWwindow* window = initializeOpenGL(argc, argv, bgColor);
	if (window == NULL) return -1;

	// Callbacks
	glfwSetKeyCallback(window, keyboard);

	// Depth test
	glEnable(GL_DEPTH_TEST);

	// Normal vectors are normalized after transformation.
	glEnable(GL_NORMALIZE);

	// Viewport and perspective setting
	reshape(window, windowW, windowH);

	// Initialization - Main loop - Finalization

	init();

	// Main loop
	float	previous = (float)glfwGetTime();
	float	elapsed = 0;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();	// Events

		render(window);	// Draw one frame
		glfwSwapBuffers(window);	// Swap buffers
	}
	// Finalization
	quit();

	// Terminate the glfw system
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void
init()
{
	// Keyboard
	cout << endl;
	cout << "Keyboard	input :	space for play/pause" << endl;
	cout << endl;
	cout << "Keyboard	input :	1  	 	  question1" << endl;
	cout << "Keyboard	input :	2  	 	  question2" << endl;
}

void initTorusVertices2() {
	double radMain = 1;
	double radCircle = 0.5;

	for (int i = 0; i < sweepAngle; i++) {
		double mainAngle = i * 3.141592 / 18;
		for (int j = 0; j < 18; j++){
			double angle = j * 3.141592 / 9;
			p[i][j].x = cos(mainAngle) * (radMain + radCircle * cos(angle));
			p[i][j].z = -1 * sin(mainAngle) * (p[0][j].x);
			p[i][j].y = radMain + radCircle * sin(angle);
			p[36][j] = p[0][j];
		}
		p[i][18] = p[i][0];
	}
}

void initTorusVertices() {
	float	theta1 = 20;
	float	theta2 = 10;

	glm::vec3	axis1(0, 0, 1);
	glm::vec3	axis2(0, 1, 0);
	glm::vec3	pivot(1, 1, 0);
	glm::vec4	origin(0.5, 0, 0, 1);

	for (int j = 0; j < 18; j++) {
		glm::mat4	M(1.0);
		glm::vec4	result;
		M = glm::translate(M, pivot);
		M = glm::rotate(M, glm::radians(j * theta1), axis1);
		result = M * origin;
		p[0][j].x = result.x;
		p[0][j].y = result.y;
		p[0][j].z = result.z;
	}
	p[0][18] = p[0][0];

	for (int j = 0; j < 19; j++) {
		for (int i = 0; i < sweepAngle + 1; i++) {
			glm::mat4	M(1.0);
			glm::vec4	temp;
			glm::vec4	result;
			temp[0] = p[0][j].x;
			temp[1] = p[0][j].y;
			temp[2] = p[0][j].z;
			temp[3] = 1;
			M = glm::rotate(M, glm::radians(i * theta2), axis2);
			result = M * temp;
			p[i][j].x = result.x;
			p[i][j].y = result.y;
			p[i][j].z = result.z;
		}
	}
}

void drawTorusWireframe() {
	initTorusVertices();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	for (int i = 0; i < sweepAngle; i++) {
		for (int j = 0; j < 18; j++) {
			if (i == 35 && j != 17) {
				glColor3f(0, 0, 0);

				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][j + 1].x, p[i][j + 1].y, p[i][j + 1].z);
				glVertex3f(p[0][j + 1].x, p[0][j + 1].y, p[0][j + 1].z);
				glVertex3f(p[0][j].x, p[0][j].y, p[0][j].z);
			}

			else if (i != 35 && j == 17) {
				glColor3f(0, 0, 0);

				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][0].x, p[i][0].y, p[i][0].z);
				glVertex3f(p[i + 1][0].x, p[i + 1][0].y, p[i + 1][0].z);
				glVertex3f(p[i + 1][j].x, p[i + 1][j].y, p[i + 1][j].z);
			}

			else if (i == 35 && j == 17) {
				glColor3f(0, 0, 0);

				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][0].x, p[i][0].y, p[i][0].z);
				glVertex3f(p[0][0].x, p[0][0].y, p[0][0].z);
				glVertex3f(p[0][j].x, p[0][j].y, p[0][j].z);
			}
			else {
				glColor3f(0, 0, 0);

				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][j + 1].x, p[i][j + 1].y, p[i][j + 1].z);
				glVertex3f(p[i + 1][j + 1].x, p[i + 1][j + 1].y, p[i + 1][j + 1].z);
				glVertex3f(p[i + 1][j].x, p[i + 1][j].y, p[i + 1][j].z);
			}
		}
	}
	glEnd();
}

void drawTorusQuads() {
	initTorusVertices();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_QUADS);
	for (int i = 0; i < sweepAngle; i++) {
		for (int j = 0; j < 18; j++) {
			glm::vec3	v1 = p[i + 1][j] - p[i][j];
			glm::vec3	v2 = p[i][j + 1] - p[i][j];
			glm::vec3	v = normalize(cross(v1, v2));

			if (i == 35 && j != 17) {
				if (dot(v, eye - p[i][j]) < 0)
					glColor3f(1, 0, 0);
				else
					glColor3f(0, 0, 1);
				glNormal3fv(value_ptr(v));

				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][j + 1].x, p[i][j + 1].y, p[i][j + 1].z);
				glVertex3f(p[0][j + 1].x, p[0][j + 1].y, p[0][j + 1].z);
				glVertex3f(p[0][j].x, p[0][j].y, p[0][j].z);
			}

			else if (i != 35 && j == 17) {
				if (dot(v, eye - p[i][j]) < 0)
					glColor3f(1, 0, 0);
				else
					glColor3f(0, 0, 1);
				glNormal3fv(value_ptr(v));


				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][0].x, p[i][0].y, p[i][0].z);
				glVertex3f(p[i + 1][0].x, p[i + 1][0].y, p[i + 1][0].z);
				glVertex3f(p[i + 1][j].x, p[i + 1][j].y, p[i + 1][j].z);
			}

			else if (i == 35 && j == 17) {
				if (dot(v, eye - p[i][j]) < 0)
					glColor3f(1, 0, 0);
				else
					glColor3f(0, 0, 1);
				glNormal3fv(value_ptr(v));


				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][0].x, p[i][0].y, p[i][0].z);
				glVertex3f(p[0][0].x, p[0][0].y, p[0][0].z);
				glVertex3f(p[0][j].x, p[0][j].y, p[0][j].z);
			}
			else {
				if (dot(v, eye - p[i][j]) < 0)
					glColor3f(1, 0, 0);
				else
					glColor3f(0, 0, 1);
				glNormal3fv(value_ptr(v));


				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][j + 1].x, p[i][j + 1].y, p[i][j + 1].z);
				glVertex3f(p[i + 1][j + 1].x, p[i + 1][j + 1].y, p[i + 1][j + 1].z);
				glVertex3f(p[i + 1][j].x, p[i + 1][j].y, p[i + 1][j].z);
			}
		}
	}
	glEnd();
}

void
render(GLFWwindow* window)
{
	if (depthTest)	glEnable(GL_DEPTH_TEST);
	else			glDisable(GL_DEPTH_TEST);

	// Background color
	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);

	// Axes
	drawAxes(AXIS_LENGTH, AXIS_LINE_WIDTH * dpiScaling);

	// Draw
	switch (selection)
	{
	case 1:	drawTorusWireframe();	break;
	case 2: glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
		drawTorusWireframe();
		drawTorusQuads();
		break;
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

		case GLFW_KEY_D:	depthTest = !depthTest;	break;
			// Example selection
		case GLFW_KEY_1:	selection = 1;	break;
		case GLFW_KEY_2:	selection = 2;	break;

		case GLFW_KEY_LEFT:	if (sweepAngle)	sweepAngle--;	break;
		case GLFW_KEY_RIGHT:if (sweepAngle != 36)	sweepAngle++;	break;
		}
	}
}
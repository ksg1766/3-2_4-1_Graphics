#include "glSetup.h"

#include <glm/glm.hpp>	// OpenGL Mathematics
#include <glm/gtc/type_ptr.hpp>	// value_ptr()
using namespace glm;

#include <iostream>
#include <fstream>
using namespace std;

void init();
void quit();
void render(GLFWwindow* window);
void	keyboard(GLFWwindow* window, int key, int code, int action, int mods);
void	mouseButton(GLFWwindow* window, int button, int action, int mods);

// Camera configuation
vec3	eyeTopView(0, 10, 0);		//Top view
vec3	upTopView(0, 0, -1);		//Top view		  

vec3	eyeFrontView(0, 0, 10);  	  //Front view
vec3	upFrontView(0, 1, 0);	 	  //Front view

vec3	eyeBirdView(0, -10, 4);   //Bird-eye view
vec3	upBirdView(0, 1, 0);	  //Bird-eye view
vec3	center(0, 0, 0);

// Light configuration			
vec4	light(0.0, 0.0, 0.8, 1);	// Initial light position
// Global coordinate frame
bool	axes = true;

float	AXIS_LENGTH = 1.25;
float	AXIS_LINE_WIDTH = 2;

// Colors
GLfloat bgColor[4] = { 1, 1, 1, 1 };

// Width and height of picking region in window coordinates
double delX = 2;
double delY = 2 ;

// control variable
int	view = 1;		//Top, front, bird  eye view
// Picking
int	picked = 1;	//Selected example

int
main(int argc, char* argv[])
{
	// Field of view of 85mm lens in degree
	fovy = 16.1f;

	// Initialize the OpenGL system
	GLFWwindow* window = initializeOpenGL(argc, argv, bgColor);
	if (window == NULL) return -1;

	// Callbacks
	glfwSetKeyCallback(window, keyboard);
	glfwSetMouseButtonCallback(window, mouseButton);

	// Depth test
	glEnable(GL_DEPTH_TEST);

	// Normal vectors are normalized after transformation.
	glEnable(GL_NORMALIZE);

	// Viewport and perspective setting
	reshape(window, windowW, windowH);

	// Initialization - Main loop - Finalization

	init();

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();	// Events

		render(window);		// Draw one frame
		glfwSwapBuffers(window);	// Swap buffers
	}

	// Finalization
	quit();

	// Terminate the glfw system
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

// Sphere, cylinder
GLUquadricObj* sphere = NULL;

void
init()
{
	// Prepare quadric shapes
	sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluQuadricNormals(sphere, GLU_SMOOTH);
	gluQuadricOrientation(sphere, GLU_OUTSIDE);
	gluQuadricTexture(sphere, GL_FALSE);

	// Usage
	cout << endl;
	cout << "Keyboard input :	x for axes on/off" << endl;
	cout << "Keyboard input :	v the top, front, bird-eye view" << endl;
	cout << "Mouse button down: Select an object" << endl;
}

void
quit()
{
	// Delete quadric	shapes		
	gluDeleteQuadric(sphere);
}

// Material
void
setupColoredMaterial(const vec3& color)
{
	// Material
	GLfloat mat_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat mat_diffuse[4] = { color[0], color[1], color[2], 1.0f };
	GLfloat mat_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat mat_shininess = 100.0f;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
}

// Draw a sphere using a GLU
void
drawSphere(float radius, int slices, int stacks)
{
	gluSphere(sphere, radius, slices, stacks);
}

// Light
void
setupLight(const vec4& p)
{
	GLfloat ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, value_ptr(p));
}

void
render(GLFWwindow* window)
{
	// Picking
	glInitNames();	// Initialize the name stack
	glPushName(-1);	// Push at least one name in the stack

	// Background color
	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vec3	eye(0), up(0);
	switch (view)
	{
	case	0:	eye = eyeTopView;	up = upTopView;		break;
	case	1:	eye = eyeFrontView;	up = upFrontView;	break;
	case	2:	eye = eyeBirdView;	up = upBirdView;	break;
	}

	gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);

	// Axes
	if (axes)
	{
		glDisable(GL_LIGHTING);
		drawAxes(AXIS_LENGTH, AXIS_LINE_WIDTH * dpiScaling);
	}

	// Lighting
	//
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	setupLight(light);

	// Draw objects
	//
	// 3x3 spheres or bunnies
	glScalef(0.4f, 0.4f, 0.4f);

	vec3	u[3];	// y-axis
	vec3	v[3];	// x-axis
	u[0] = vec3(0, 1, 0) * 2.0f; u[1] = vec3(0, 0, 0);	u[2] = -u[0];
	v[0] = vec3(1, 0, 0) * 4.0f; v[1] = vec3(0, 0, 0);	v[2] = -v[0];

	for (int i = 0; i < 3; i++)
	{
		glPushMatrix();
		glTranslatef(u[i].x, u[i].y, u[i].z);

		for (int j = 0; j < 3; j++)
		{
			glPushMatrix();
			glTranslatef(v[j].x, v[j].y, v[j].z);

			//Material
			if (picked == (3 * i + j))	setupColoredMaterial(vec3(0, 0, 1));
			else						setupColoredMaterial(vec3(1, 1, 1));

			//Picking
			glLoadName(3 * i + j);

			//Draw a shere
			drawSphere(0.7f, 64, 64);

			glPopMatrix();
		}
		glPopMatrix();
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

			// Axes on/off
		case GLFW_KEY_X:	axes = !axes;	break;
			// Top, front, bird-eye view
		case GLFW_KEY_V: view = (view + 1) % 3; break;
		}
	}
}

int
findNearestHits(int hits, GLuint selectBuffer[64])
{
	bool	diagnosis = true;

	if (diagnosis) cout << "hits = " << hits << endl;

	int	name = -1;
	float	nearest = 2.0; // zl, z2 in [0, 1]

	int	index = 0;
	for (int i = 0; i < hits; i++)
	{
		int	n = selectBuffer[index + 0];// # of names
		float	z1 = (float)selectBuffer[index + 1] / 0xffffffff;	// min depth value
		float	z2 = (float)selectBuffer[index + 2] / 0xffffffff;	// max depth value

		//Choose the nearest one only
		if (z1 < nearest) { nearest = z1; name = selectBuffer[index + 3]; }

		if (diagnosis)
		{
			cout << "\t# of names = " << n << endl;
			cout << "\tz1 =	" << z1 << endl;
			cout << "\tz2 = " << z2 << endl;
			cout << "\tnames: ";
			for (int j = 0; j < n; j++)
				cout << selectBuffer[index + 3 + j] << " ";
			cout << endl;
		}
		// To the next available one
		index += (3 + n);
	}
	if (diagnosis) cout << "picked = " << name << endl;

	return name;
}

int
selectObjects(GLFWwindow* window, double x, double y)
{
	// Width and height of picking region in window coordinates
	//double delX = 2;
	//double delY = 2;

	// Maximum 64 selections
	GLuint selectBuffer[64];
	glSelectBuffer(64, selectBuffer);

	// Current viewport
	GLint	viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Backup the porjection matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	glLoadIdentity();

	// Creates a projection matrix that can be used to restrict drawing to
	// a small region of the viewport
	gluPickMatrix(x, viewport[3] - y, delX, delY, viewport); // y screen to viewport

	// Exploit the projection matrix for normal rendering
	setupProjectionMatrix();

	// Enter selection mode
	glRenderMode(GL_SELECT);

	// Render the objects for selection
	render(window);

	// Return to normal rendering mode and getting the picked object
	GLint	hits = glRenderMode(GL_RENDER);
	int name = findNearestHits(hits, selectBuffer);

	// Restore the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	return name;
}

void
mouseButton(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT)
	{
		// Mouse cursor position in the screen coordinate
		double xs, ys;
		glfwGetCursorPos(window, &xs, &ys);

		// Mouse cursor position in the framebuffer coordinate
		double x = xs * dpiScaling; // for HiDPI
		double y = ys * dpiScaling; // for HiDPI

		// Retrieve the selected object
		picked = selectObjects(window, x, y);
	}
}
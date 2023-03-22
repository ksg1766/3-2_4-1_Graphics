#include "glSetup.h"

#ifdef	_WIN32
#define _USE_MATH_DEFINES	// To include the definition of M_PI in math. h
#endif

#include <glm/glm.hpp>	// OpenGL Mathematics
#include <glm/gtc/type_ptr.hpp>	// value_ptr()
using namespace glm;

#include <iostream>
#include <fstream>
#include <list>
using namespace std;

void init();
void quit();
void render(GLFWwindow* window);
void keyboard(GLFWwindow* window, int key, int code, int action, int mods);

bool readMesh(const char* filename);
void deleteMesh();

// Camera configuation
vec3 eye(3, 3, 3);
vec3 center(0, 0, 0);
vec3 up(0, 0, 1);

// Light configuration 	  
vec4	light(5.0, 5.0, 0.0, 1);	// Light position		

// Global coordinate frame
float AXIS_LENGTH = 1.75;
float AXIS_LINE_WIDTH = 2;

// Colors
GLfloat bgColor[4] = { 1, 1, 1, 1 };

// Selected example
int selection = 1;

// Sphere, cylinder
GLUquadricObj* sphere = NULL;
GLUquadricObj* cylinder = NULL;
GLUquadricObj* disk = NULL;

// Paly configuration
bool	pause = true;

float	timeStep = 1.0f / 120;	// 120fps
float	period = 8.0;
float	theta = 0;

// Drawing parameters	
bool	depthTest = true;	// Depth test
bool	polygonFill = true;	// Polygon mode

int
main(int argc, char* argv[])
{
	// FPS control: vsync should be e for precise time stepping
	vsync = 0; // e for immediate mode (Tearing possible)

	// Initialize the OpenGL system
	GLFWwindow* window = initializeOpenGL(argc, argv, bgColor);
	if (window == NULL) return -1;

	// Callbacks
	glfwSetKeyCallback(window, keyboard);

	// Normal vectors are normalized after transformation.
	glEnable(GL_NORMALIZE);

	// Turn off back face culling for cylinder
	glDisable(GL_CULL_FACE);

	// Viewport and perspective setting
	reshape(window, windowW, windowH);

	// Initialization - Main loop - Finalization
	//
	init();

	// Main loop
	float	previous = (float)glfwGetTime();
	float	elapsed = 0;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();	// Events

		// Time passed during a single loop
		float now = (float)glfwGetTime();
		float delta = now - previous;
		previous = now;

		// Time passed after the previous frame
		elapsed += delta;

		// Deal with the current frame
		if (elapsed > timeStep)
		{
			// Rotate the line segment if not paused
			if (!pause)
			{
				// One rotation per period
				theta += float(2.0 * M_PI) / period * elapsed;
			}

			elapsed = 0;	// Reset the elapsed time
		}

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
	// Prepare quadric shapes
	sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluQuadricNormals(sphere, GLU_SMOOTH);
	gluQuadricOrientation(sphere, GLU_OUTSIDE);
	gluQuadricTexture(sphere, GL_FALSE);

	cylinder = gluNewQuadric();
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	gluQuadricNormals(cylinder, GLU_SMOOTH);
	gluQuadricOrientation(cylinder, GLU_OUTSIDE);
	gluQuadricTexture(cylinder, GL_FALSE);

	disk = gluNewQuadric();
	gluQuadricDrawStyle(disk, GLU_FILL);
	gluQuadricNormals(disk, GLU_SMOOTH);
	gluQuadricOrientation(disk, GLU_OUTSIDE);
	gluQuadricTexture(disk, GL_FALSE);

	// Prepare mesh
	readMesh("m01_bunny.off");

	// Keyboard			
	cout << endl;
	cout << "Keyboard Input: d for depth test on/off" << endl;
	cout << "Keyboard Input: f for polygon fill on/ off" << endl;
	cout << endl;
	cout << "Keyboard Input: 1 for sphere" << endl;
	cout << "Keyboard Input: 2 for cylinder" << endl;
	cout << "Keyboard Input: 3 for disk" << endl;
	cout << "Keyboard Input: 4 for bunny" << endl;
}

void
quit()
{
	// Delete quadric	shapes		
	gluDeleteQuadric(sphere);
	gluDeleteQuadric(cylinder);
	gluDeleteQuadric(disk);

	// Delete mesh
	deleteMesh();
}

// Light
void
setupLight()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	GLfloat ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, value_ptr(light));

	// For disk
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

// Material
void
setupMaterial()
{
	// Material
	GLfloat mat_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat mat_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat mat_shininess = 128;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
}

void
setDiffuseColor(const vec3& color)
{
	GLfloat mat_diffuse[4] = { color[0], color[1], color[2], 1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_diffuse);
}

// Draw a sphere after setting up its material
void
drawSphere(float radius, const vec3& color)
{
	// Material
	setDiffuseColor(color);

	// Sphere using GLU quadrics
	gluSphere(sphere, radius, 36, 36);
}

// Draw a cylinder after setting up its material
void
drawCylinder(float radius, float height, const vec3& color)
{
	// Material
	setDiffuseColor(color);

	// Cylinder using GLU quadrics
	gluCylinder(cylinder, radius, radius, height, 72, 5);
}

// Draw a disk after setting up its material
void
drawDisk(float inner_radius, float outer_radius, const vec3& color)
{
	// Material
	setDiffuseColor(color);

	// Disk using GLU quadrics
	gluDisk(disk, inner_radius, outer_radius, 72, 5);
}

// Read a mesh from a given OFF file
int	nVertices = 0, nFaces = 0, nEdges = 0;
vec3* vertex = NULL;
int* face[3] = { NULL, NULL, NULL };
list<int>* head;

bool
readMesh(const char* filename)
{
	int countEdges = 0;
	ifstream	is(filename);
	if (is.fail()) return false;

	char	magicNumber[256];
	is >> magicNumber;

	// # vertices, # faces, # edges	
	is >> nVertices >> nFaces >> nEdges;
	cout << "# vertices = " << nVertices << endl;
	cout << "# faces = " << nFaces << endl;
	cout << "# edge = " << nEdges << endl;

	// Vertices
	vertex = new vec3[nVertices];
	for (int i = 0; i < nVertices; i++)
		is >> vertex[i].x >> vertex[i].y >> vertex[i].z;

	// Faces
	face[0] = new int[nFaces];	// Only support triangles
	face[1] = new int[nFaces];
	face[2] = new int[nFaces];

	head = new list<int>[nVertices];

	int n;
	for (int i = 0; i < nFaces; i++) {
		is >> n >> face[0][i] >> face[1][i] >> face[2][i];

		for (int j = 0; j < 2; j++) {
			for (int k = j + 1; k < 3; k++) {
				if (face[j][i] > face[k][i]) {
					int temp;
					temp = face[k][i];
					face[k][i] = face[j][i];
					face[j][i] = temp;
				}
			}
		}

		if (find(head[face[0][i]].begin(), head[face[0][i]].end(), face[1][i]) == head[face[0][i]].end()) {
			head[face[0][i]].push_back(face[1][i]);
			countEdges++;
		}

		if (find(head[face[0][i]].begin(), head[face[0][i]].end(), face[2][i]) == head[face[0][i]].end()) {
			head[face[0][i]].push_back(face[2][i]);
			countEdges++;
		}

		if (find(head[face[1][i]].begin(), head[face[1][i]].end(), face[2][i]) == head[face[1][i]].end()) {
			head[face[1][i]].push_back(face[2][i]);
			countEdges++;
		}


		if (n != 3) cout << "# vertices of the " << i << "-th faces = " << n << endl;
	}
	cout << "# countEdges = " << countEdges << endl;
	return true;
}

// Delete the vertices and faces of the mesh
void
deleteMesh()
{
	if (vertex) { delete[] vertex; vertex = NULL; }
	if (face[0]) { delete[]	face[0]; face[0] = NULL; }
	if (face[1]) { delete[] face[1]; face[1] = NULL; }
	if (face[2]) { delete[] face[2]; face[2] = NULL; }
}

// Draw the vertices of the mesh only
void
drawMesh()
{
	// Color of the points
	setDiffuseColor(vec3(0, 0, 0));

	// Place the bunny model
	glRotatef(90, 0, 0, 1);
	glRotatef(90, 1, 0, 0);

	// Draw all the vertices of the bunny model
	glPointSize(3 * dpiScaling);

	glBegin(GL_LINES);

	for (int i = 0; i < nVertices; i++) {
		for (auto iter = begin(head[i]); iter != end(head[i]); iter++) {
			glVertex3f(vertex[i].x, vertex[i].y, vertex[i].z);
			glVertex3f(vertex[*iter].x, vertex[*iter].y, vertex[*iter].z);
		}
	}
	glEnd();
}

void
render(GLFWwindow* window)
{
	// Depth test
	if (depthTest)	glEnable(GL_DEPTH_TEST);
	else			glDisable(GL_DEPTH_TEST);

	// Polygon mode
	if (polygonFill)	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Background color
	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);

	// Axes
	glDisable(GL_LIGHTING);
	drawAxes(AXIS_LENGTH, AXIS_LINE_WIDTH * dpiScaling);

	// Lighting
	setupLight();

	// Material
	setupMaterial();

	// Rotation about the z-axis
	glRotatef(degrees(theta), 0, 1, 0); // Radian to degree

	// Draw sphere using gluSphere() after setting up the material
	switch (selection)

	{
	case 1:	drawSphere(1.0, vec3(0.95, 0.95, 0.95));		break;
	case 2:	drawCylinder(1.0, 1.0, vec3(0.95, 0.95, 0.95));	break;
	case 3:	drawDisk(0.5, 1.0, vec3(0.95, 0.95, 0.95));		break;
	case 4:	drawMesh();										break;
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

			// Example selection	
		case GLFW_KEY_1: selection = 1;	break;
		case GLFW_KEY_2: selection = 2;	break;
		case GLFW_KEY_3: selection = 3;	break;
		case GLFW_KEY_4: selection = 4;	break;

			// Play on/off	
		case GLFW_KEY_SPACE: pause = !pause; break;

			// Depth test on/off
		case GLFW_KEY_D: depthTest = !depthTest;	break;

			// Polygon fill on/off
		case GLFW_KEY_F: polygonFill = !polygonFill;	break;
		}
	}
}
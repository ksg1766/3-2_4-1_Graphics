#include "glSetup.h"

#include <glm/glm.hpp>	// OpenGL Mathematics
#include <glm/gtc/type_ptr.hpp>	// glm: : value_ptr()
using namespace glm;

#include <iostream>
#include <fstream>
using namespace std;

void init();
void quit();
void render(GLFWwindow* window);
void keyboard(GLFWwindow* window, int key, int code, int action, int mods);

bool readMesh(const char* filename);
void deleteMesh();

// Camera configuation
vec3	eye(3, 3, 3);
vec3	center(0, 0, 0);
vec3	up(0, 1, 0);

// Light configuration
vec4	light(5.0, 5.0, 0.0, 1);	// Light position

// Global coordinate frame
bool	axes = true;

float	AXIS_LENGTH = 3;
float	AXIS_LINE_WIDTH = 2;

// Colors
GLfloat bgColor[4] = { 1, 1, 1, 1 };

// control variable
bool	smooth = true;

// Selected example
int selection = 1;

// Sphere
GLUquadricObj* sphere = NULL;

// Paly configuration
bool	pause = true;

float	timeStep = 1.0f / 120;	// 120fps
float	period = 4.0;

// Drawing parameters
bool	polygonFill = true; // Polygon mode

// Current frame
int frame = 0;

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
			// Animate 1 frame
			if (!pause) frame += 1;

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

	// Prepare mesh
	readMesh("m01_bunny.off");

	// Keyboard
	cout << endl;
	cout << "Keyboard input :	space for play/pause" << endl;
	cout << "Keyboard Input :	s for turn on/of smooth shading" << endl;
	cout << "Keyboard Input :	f polygon fill on/off" << endl;
	cout << endl;

	cout << "Keyboard	Input :	1 sphere with  16 slices and  16 stacks " << endl;
	cout << "Keyboard	Input :	2 sphere with  32 slices and  32 stacks " << endl;
	cout << "Keyboard	Input :	3 sphere with  64 slices and  64 stacks " << endl;
	cout << "Keyboard	Input :	4 sphere with 128 slices and 128 stacks " << endl;
	cout << "Keyboard	Input :	5 for flat approximation of a bunny" << endl;
	cout << "Keyboard	Input :	6 for smooth approximation of a bunny" << endl;
	cout << "Keyboard	Input :	7 for vertex normal vector of a bunny" << endl;
	cout << "Keyboard	Input :	8 for face normal vector of a bunny" << endl;
}

void
quit()
{
	// Delete quadric shapes
	gluDeleteQuadric(sphere);

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
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
}

// Draw a sphere after setting up its material
void
drawSphere(float radius, int slices, int stacks, const vec3& color)
{
	// Material
	setDiffuseColor(color);

	// Sphere using GLU quadrics
	gluSphere(sphere, radius, slices, stacks);
}

// Read a mesh from a given OFF file
int nVertices = 0, nFaces = 0, nEdges = 0;
glm::vec3* vertex = NULL;
glm::vec3* fv = NULL;
glm::vec3* vnormal = NULL;
glm::vec3* fnormal = NULL;
int* face[3] = { NULL, NULL, NULL };

bool
readMesh(const char* filename)
{
	ifstream	is(filename);
	if (is.fail()) return false;

	char	magicNumber[256];
	is >> magicNumber;

	// # vertices, # faces, # edges
	is >> nVertices >> nFaces >> nEdges;
	cout << "# vertices = " << nVertices << endl;
	cout << "# faces = " << nFaces << endl;

	// Vertices
	vertex = new glm::vec3[nVertices];
	for (int i = 0; i < nVertices; i++)
		is >> vertex[i].x >> vertex[i].y >> vertex[i].z;

	// Vertex normals
	vnormal = new glm::vec3[nVertices];
	for (int i = 0; i < nVertices; i++)
		vnormal[i] = glm::vec3(0, 0, 0);

	// Face normals
	fnormal = new glm::vec3[nFaces];

	// Faces
	face[0] = new int[nFaces]; // Only support triangles int[nFaces]; int[nFaces];
	face[1] = new int[nFaces];
	face[2] = new int[nFaces];
	
	int n;
	for (int i = 0; i < nFaces; i++)
	{
		is >> n >> face[0][i] >> face[1][i] >> face[2][i];
		
		if (n != 3) cout << "# vertices of the " << i << "-th faces = " << n << endl;

		// Normal vector of the face
		glm::vec3	v1 = vertex[face[1][i]] - vertex[face[0][i]];
		glm::vec3	v2 = vertex[face[2][i]] - vertex[face[0][i]];
		glm::vec3	v = normalize(cross(v1, v2));

		fnormal[i] = v;

		// Add it to the normal vector of each vertex
		vnormal[face[0][i]] += v;
		vnormal[face[1][i]] += v;
		vnormal[face[2][i]] += v;	
	}

	fv = new glm::vec3[nFaces];
	for (int i = 0; i < nFaces; i++) {
		fv[i].x = (vertex[face[0][i]].x + vertex[face[1][i]].x + vertex[face[2][i]].x) / 3.0f;
		fv[i].y = (vertex[face[0][i]].y + vertex[face[1][i]].y + vertex[face[2][i]].y) / 3.0f;
		fv[i].z = (vertex[face[0][i]].z + vertex[face[1][i]].z + vertex[face[2][i]].z) / 3.0f;
	}

	// Normalization of the normal vectors
	for (int i = 0; i < nVertices; i++)
		vnormal[i] = normalize(vnormal[i]);

	return true;
}

// Delete the vertices and faces of the mesh
void
deleteMesh()
{
	if (vertex) { delete[] vertex; vertex = NULL; }
	if (fv) { delete[] fv; fv = NULL; }
	if (vnormal) { delete[] vnormal; vnormal = NULL; }
	if (fnormal) { delete[] fnormal; fnormal = NULL; }
	if (face[0]) { delete[] face[0]; face[0] = NULL; }
	if (face[1]) { delete[] face[1]; face[1] = NULL; }
	if (face[2]) { delete[] face[2]; face[2] = NULL; }
}

// Draw a flat mesh by specifying its face normal vectors
void
drawFlatMesh()
{
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < nFaces; i++)
	{
		glNormal3fv(value_ptr(fnormal[i]));
		for (int j = 0; j < 3; j++)
			glVertex3fv(value_ptr(vertex[face[j][i]]));
	}
	glEnd();
}

// Draw a smooth mesh by specifying its vertex normal vectors
void
drawSmoothMesh()
{
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < nFaces; i++)
		for (int j = 0; j < 3; j++)
		{
			glNormal3fv(value_ptr(vnormal[face[j][i]]));
			glVertex3fv(value_ptr(vertex[face[j][i]]));
		}
	glEnd();
}

void
drawFaceNormal()
{
	glBegin(GL_LINES);
	for (int i = 0; i < nFaces; i++)
	{
		glVertex3f(0.1 * fnormal[i].x + fv[i].x, 0.1 * fnormal[i].y + fv[i].y, 0.1 * fnormal[i].z + fv[i].z);
		glVertex3f(fv[i].x, fv[i].y, fv[i].z);
	}
	glEnd();
}

void
drawVertexNormal()
{
	glBegin(GL_LINES);
	for (int i = 0; i < nVertices; i++)
	{
		glVertex3f(0.1 * vnormal[i].x + vertex[i].x, 0.1 * vnormal[i].y + vertex[i].y, 0.1 * vnormal[i].z + vertex[i].z);
		glVertex3f(vertex[i].x, vertex[i].y, vertex[i].z);
	}
	glEnd();
}

void
render(GLFWwindow* window)
{
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
	if (axes)
	{
		glDisable(GL_LIGHTING);
		drawAxes(AXIS_LENGTH, AXIS_LINE_WIDTH * dpiScaling);
	}

	// Lighting
	setupLight();

	// Smooth shading
	if (smooth)	glShadeModel(GL_SMOOTH);
	else		glShadeModel(GL_FLAT);	// Use the normal of the last vertex of a polygon

	// Material
	setupMaterial();

	// Rotation
	float	theta = frame * 2 / period;
	glm::vec3	axis(0, 1, 0);
	glRotatef(theta, axis.x, axis.y, axis.z);

	// Draw sphere using gluSphere() after setting up the material	
	switch (selection)
	{
	case 1:	drawSphere(1.0, 16, 16, vec3(0.95, 0.95, 0.95));	break;
	case 2:	drawSphere(1.0, 32, 32, vec3(0.95, 0.95, 0.95));	break;
	case 3:	drawSphere(1.0, 64, 64, vec3(0.95, 0.95, 0.95));	break;
	case 4:	drawSphere(1.0, 128, 128, vec3(0.95, 0.95, 0.95));	break;
	case 5:	drawFlatMesh();										break;
	case 6:	drawSmoothMesh();									break;
	case 7:	drawVertexNormal();									break;
	case 8:	drawFaceNormal();									break;
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

			// Axes
		case GLFW_KEY_X: axes = !axes; break;

			// Play on/off
		case GLFW_KEY_SPACE: pause = !pause; break;

			// Control for changing smooth/flat shading
		case GLFW_KEY_S: smooth = !smooth;	break;

			// Polygon fill on/off
		case GLFW_KEY_F: polygonFill = !polygonFill;	break;

			// Example selection
		case GLFW_KEY_1:	selection = 1;	break;
		case GLFW_KEY_2:	selection = 2;	break;
		case GLFW_KEY_3:	selection = 3;	break;
		case GLFW_KEY_4:	selection = 4;	break;
		case GLFW_KEY_5:	selection = 5;	break;
		case GLFW_KEY_6:	selection = 6;	break;
		case GLFW_KEY_7:	selection = 7;	break;
		case GLFW_KEY_8:	selection = 8;	break;
		}
	}
}
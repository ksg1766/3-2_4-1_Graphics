#include "glSetup.h"

#ifdef	_WIN32
#define _USE_MATH_DEFINES	// To include the definition of M_PI in math. h
#endif

#include <glm/glm.hpp>		// OpenGL Mathematics
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr()
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

// Light configuration			
vec4	lightInitialP(0.0, 2.0, 2.5, 1);	// Initial light position

// Global coordinate frame
float AXIS_LENGTH = 3;
float AXIS_LINE_WIDTH = 2;

// Colors
GLfloat bgColor[4] = { 1, 1, 1, 1 };

// Selected example
int selection = 1;

bool	rotationLight = false;	//Rotate the lights

//Rotation angle around the y - axis
float	thetaLight;

bool	lightOn; //Point = 0, distant = 1, spot = 2 lights

bool	depthTest = true;

int		sweepAngle = 36;
glm::vec3	p[37][19];
int pv[37][19] = { 0 };

bool	exponent = false;
float	exponentInitial = 0.0;	 	 	//[0, 128]
float	exponentValue = exponentInitial;
float	exponentNorm = exponentValue / 128.0f;

bool	cutoff = false;
float	cutoffMax = 60; //[0, 90] degree
float	cutoffInitial = 30.0;	//[0, cutoffMax] degree
float	cutoffValue = cutoffInitial;
float	cutoffNorm = cutoffValue / cutoffMax;

// Paly configuration
bool	pause = false;

float	timeStep = 1.0f / 120;	// 120fps
float	period = 4.0f;

// Current frame
int frame = 0;

GLfloat mat_red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
GLfloat mat_blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

void quit()
{}

GLfloat mat_shininess = 128.0;

void
setupColoredMaterial(const vec3& color)
{
	// Material
	GLfloat mat_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat mat_diffuse[4] = { color[0], color[1], color[2], 1.0f };
	GLfloat mat_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
//	GLfloat mat_shininess = 100;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
}

void
animate()
{
	frame += 1;

	//Rotation angle of the light
	if (rotationLight)
	{
			if (lightOn) thetaLight += 2 / period;	// degree
	}

		//Periodically change the exponent and /or cutoff value of the spot light
	if (lightOn && exponent)
	{
		exponentNorm += float(radians(4.0 / period) / M_PI);
		exponentValue = float(128.0f * (acos(cos(exponentNorm * M_PI)) / M_PI));
	}
	if (lightOn && cutoff)
	{
		cutoffNorm += float(radians(4.0 / period) / M_PI);
		cutoffValue = float(cutoffMax * (acos(cos(cutoffNorm * M_PI)) / M_PI));
	}
}

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
			if (!pause) animate();
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

GLUquadricObj* sphere = NULL;
GLUquadricObj* cone = NULL;

void
init()
{
	lightOn = true;

	thetaLight = 0;

	exponentValue = exponentInitial;
	exponentNorm = exponentValue / 128.0f;
	cutoffValue = cutoffInitial;
	cutoffNorm = cutoffValue / cutoffMax;

	// Prepare quadric shapes
	sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluQuadricNormals(sphere, GLU_SMOOTH);
	gluQuadricOrientation(sphere, GLU_OUTSIDE);
	gluQuadricTexture(sphere, GL_FALSE);

	cone = gluNewQuadric();
	gluQuadricDrawStyle(cone, GLU_FILL);
	gluQuadricNormals(cone, GLU_SMOOTH);
	gluQuadricOrientation(cone, GLU_OUTSIDE);
	gluQuadricTexture(cone, GL_FALSE);

	// Keyboard
	cout << endl;
	cout << "Keyboard	input :	space			play/pause (default = play)" << endl;
	cout << endl;
	cout << "Keyboard	input :	1  	 			wireframe torus" << endl;
	cout << "Keyboard	input :	2  	 			torus with face" << endl;
	cout << "Keyboard	input :	3  				torus with vertex normal vector" << endl;
	cout << "Keyboard	input :	d 				depthtest on/off" << endl;
	cout << "Keyboard	input :	q/esc  			quit the program" << endl;
	cout << "Keyboard	input :	l  	 			rotate light position" << endl;
	cout << "Keyboard	input :	e  	 			time dependent exponent of a spot light" << endl;
	cout << "Keyboard	input :	c  				time dependent cutoff of a spot light" << endl;
	cout << "Keyboard	input :	s  	 			spot light on/off" << endl;
	cout << "Keyboard	input :	up/down			control sweepangle" << endl;
	cout << "Keyboard	input :	left/right		control shininess coefficient" << endl;
}

void initTorusVertices2() {
	double radMain = 1;
	double radCircle = 0.5;

	for (int i = 0; i < sweepAngle; i++) {
		double mainAngle = i * 3.141592 / 18;
		for (int j = 0; j < 18; j++) {
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
	float	theta1 = -20;
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

	setupColoredMaterial(vec3(0, 0, 0));
	for (int i = 0; i < sweepAngle; i++) {
		for (int j = 0; j < 18; j++) {
			if (i == 35 && j != 17) {
				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][j + 1].x, p[i][j + 1].y, p[i][j + 1].z);
				glVertex3f(p[0][j + 1].x, p[0][j + 1].y, p[0][j + 1].z);
				glVertex3f(p[0][j].x, p[0][j].y, p[0][j].z);
			}

			else if (i != 35 && j == 17) {
				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][0].x, p[i][0].y, p[i][0].z);
				glVertex3f(p[i + 1][0].x, p[i + 1][0].y, p[i + 1][0].z);
				glVertex3f(p[i + 1][j].x, p[i + 1][j].y, p[i + 1][j].z);
			}

			else if (i == 35 && j == 17) {
				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][0].x, p[i][0].y, p[i][0].z);
				glVertex3f(p[0][0].x, p[0][0].y, p[0][0].z);
				glVertex3f(p[0][j].x, p[0][j].y, p[0][j].z);
			}
			else {
				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][j + 1].x, p[i][j + 1].y, p[i][j + 1].z);
				glVertex3f(p[i + 1][j + 1].x, p[i + 1][j + 1].y, p[i + 1][j + 1].z);
				glVertex3f(p[i + 1][j].x, p[i + 1][j].y, p[i + 1][j].z);
			}
		}
	}
	glEnd();
}

glm::vec3* vnormal = NULL;

void drawVertexNormal()
{
	glm::vec3	v1;
	glm::vec3	v2;
	glm::vec3	v;

	for (int i = 0; i < 36; i++)
	{
		for (int j = 0; j < 19; j++) {
			pv[i][j] = i * 18 + j;
		}
	}

	for (int i = 0; i < 36; i++)
		pv[i][18] = pv[i][0];

	for (int j = 0; j < 18; j++)
		pv[36][j] = pv[0][j];

	vnormal = new glm::vec3[37 * 18];
	for (int i = 0; i < 36 * 18; i++)
		vnormal[i] = glm::vec3(0, 0, 0);

	for (int i = 0; i < 36; i++) {
		for (int j = 0; j < 18; j++) {
			v1 = p[i + 1][j] - p[i][j];
			v2 = p[i][j + 1] - p[i][j];
			v = normalize(cross(v2, v1));

			vnormal[pv[i][j]] += v;
			vnormal[pv[i][j + 1]] += v;
			vnormal[pv[i + 1][j + 1]] += v;
			vnormal[pv[i + 1][j]] += v;
		}
	}

	for (int i = 0; i < 36; i++)
		vnormal[pv[i][0]] += vnormal[pv[i][18]];

	for (int j = 0; j < 18; j++)
		vnormal[pv[0][j]] += vnormal[pv[36][j]];

	for (int i = 0; i < 36 * 18; i++)
		vnormal[i] = normalize(vnormal[i]);

	glBegin(GL_LINES);
	for (int i = 0; i < sweepAngle; i++)
		for (int j = 0; j < 18; j++)
		{
			setupColoredMaterial(vec3(0, 0, 0));
			glVertex3f(0.15 * vnormal[pv[i][j]].x + p[i][j].x, 0.15 * vnormal[pv[i][j]].y + p[i][j].y, 0.15 * vnormal[pv[i][j]].z + p[i][j].z);
			glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
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
			glm::vec3	v = normalize(cross(v2, v1));
			
			if (i == 35 && j != 17) {
				if (dot(v, eye - p[i][j]) < 0)
					glMaterialfv(GL_BACK, GL_DIFFUSE, mat_red);

				else
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_blue);

				glNormal3fv(value_ptr(v));

				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][j + 1].x, p[i][j + 1].y, p[i][j + 1].z);
				glVertex3f(p[0][j + 1].x, p[0][j + 1].y, p[0][j + 1].z);
				glVertex3f(p[0][j].x, p[0][j].y, p[0][j].z);
			}

			else if (i != 35 && j == 17) {
				if (dot(v, eye - p[i][j]) < 0)
					glMaterialfv(GL_BACK, GL_DIFFUSE, mat_red);

				else 
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_blue);

				glNormal3fv(value_ptr(v));

				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][0].x, p[i][0].y, p[i][0].z);
				glVertex3f(p[i + 1][0].x, p[i + 1][0].y, p[i + 1][0].z);
				glVertex3f(p[i + 1][j].x, p[i + 1][j].y, p[i + 1][j].z);
			}

			else if (i == 35 && j == 17) {
				if (dot(v, eye - p[i][j]) < 0)
					glMaterialfv(GL_BACK, GL_DIFFUSE, mat_red);

				else
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_blue);

				glNormal3fv(value_ptr(v));

				glVertex3f(p[i][j].x, p[i][j].y, p[i][j].z);
				glVertex3f(p[i][0].x, p[i][0].y, p[i][0].z);
				glVertex3f(p[0][0].x, p[0][0].y, p[0][0].z);
				glVertex3f(p[0][j].x, p[0][j].y, p[0][j].z);
			}

			else {
				if (dot(v, eye - p[i][j]) < 0)
					glMaterialfv(GL_BACK, GL_DIFFUSE, mat_red);
				else
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_blue);

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

// Light
void
setupLight(const vec4& p, int i)
{
	GLfloat ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specular[4] = { 0.1f, 0.1f, 0.1f, 0.1f };

	glLightfv(GL_LIGHT0 + i, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0 + i, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0 + i, GL_POSITION, value_ptr(p));

	vec3	spotDirection = -vec3(p);
	glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, value_ptr(spotDirection));
	glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, cutoffValue);// [0, 90]
	glLightf(GL_LIGHT0 + i, GL_SPOT_EXPONENT, exponentValue); // [0, 128]
}

void
drawSphere(float radius, int slices, int stacks)
{
	gluSphere(sphere, radius, slices, stacks);
}

void
drawCone(float radius, float height, int slices, int stacks)
{
	gluCylinder(cone, 0, radius, height, slices, stacks);
}

void
computeRotation(const vec3& a, const vec3& b, float& theta, vec3& axis)
{
	axis = cross(a, b);
	float	sinTheta = length(axis);
	float	cosTheta = dot(a, b);
	theta = float(atan2(sinTheta, cosTheta) * 180.0 / M_PI);
}

void
drawSpotLight(const vec3& p, float cutoff)
{
	glPushMatrix();

	glTranslatef(p.x-0.2, p.y-0.2, p.z-0.2);

	float	theta;
	vec3	axis;
	computeRotation(vec3(0, 0, 1), vec3(0, 0, 0) - vec3(p), theta, axis);
	glRotatef(theta, axis.x, axis.y, axis.z);

	// Color
	setupColoredMaterial(vec3(0, 0, 1));

	// tan(cutoff) = r/h
	float h = 0.15f;
	float r = h * tan(radians(cutoff));
	drawCone(r, h, 16, 5);

	// Color
	setupColoredMaterial(vec3(1, 0, 0));

	// Apex
	float	apexRadius = 0.06f * (0.5f + exponentValue / 128.0f);
	drawSphere(apexRadius, 16, 16);

	glPopMatrix();
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
	
	// Smooth shading
	glShadeModel(GL_SMOOTH);

	// Rotation of the light or 3x3 models
	vec3	axis(0, 1, 0);

	// Lighting
	//
	glEnable(GL_LIGHTING);

	// Set up the lights
	vec4	lightP;

	// Just turn off the i-th light, if not lit
	if (!lightOn)  glDisable(GL_LIGHT0 + 2);

	// Turn on the i-th light
	glEnable(GL_LIGHT0 + 2);

	// Dealing with the distant light
	lightP = lightInitialP;

	// Lights rotate around the center of the world coordinate system
	mat4	R = rotate(mat4(1.0), radians(thetaLight), axis);;
	lightP = R * lightP;

	//glDisable(GL_CULL_FACE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	// 
	// Set up the i-th light
	setupLight(lightP, 2);

	// Draw the geometries of the lights

	drawSpotLight(lightP, cutoffValue);
	

	// Draw
	switch (selection)
	{
	case 1:	drawTorusWireframe();	break;
	case 2: glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
		drawTorusWireframe();
		drawTorusQuads();
		break;
	case 3:
		drawVertexNormal();
		glEnable(GL_POLYGON_OFFSET_FILL);
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

			// Play on/off
		case GLFW_KEY_SPACE:	pause = !pause; break;

		case GLFW_KEY_D:	depthTest = !depthTest;	break;

		case GLFW_KEY_S: lightOn = !lightOn;	break;
			// Light: point, direction, spot
		case GLFW_KEY_L: rotationLight = !rotationLight;	break;

			// Time dependent exponent of a spot light
		case GLFW_KEY_E: exponent = !exponent;	break;

			// Time dependent cutoff of a spot light
		case GLFW_KEY_C: cutoff = !cutoff; break;

			// Example selection
		case GLFW_KEY_1:	selection = 1;	break;
		case GLFW_KEY_2:	selection = 2;	break;
		case GLFW_KEY_3:	selection = 3;	break;

		case GLFW_KEY_LEFT:	if (sweepAngle)	sweepAngle--;	break;
		case GLFW_KEY_RIGHT:if (sweepAngle != 36)	sweepAngle++;	break;
		case GLFW_KEY_UP:	if (mat_shininess + 32.0 <= 128.0)	mat_shininess += 32.0;	break;
		case GLFW_KEY_DOWN:	if (mat_shininess - 32.0 >= 0.0)	mat_shininess -= 32.0;	break;
		}
	}
}
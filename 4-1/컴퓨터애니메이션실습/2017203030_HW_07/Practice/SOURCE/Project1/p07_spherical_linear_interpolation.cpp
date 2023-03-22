#include "glSetup.h"
#include "mesh.h"

#include <Eigen/Dense>
using namespace Eigen;

#include <iostream>
using namespace std;

#ifdef _WIN32
#define _USE_MATH_DEFINES // To include the definition of M PI in math.h
#endif
#include <math.h>

void init(const char* filename);
void setupLight();

void update(float elapsed);
void render(GLFWwindow* window);
void reshape(GLFWwindow* window, int w, int h);
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);

bool pause = false;

Vector3f eye(9, 9, 9);
Vector3f center(0, 0, 0);
Vector3f up(0, 1, 0);

Vector4f light(5.0, 5.0, 0.0, 1);

float AXIS_LENGTH = 3;
float AXIS_LINE_WIDTH = 2;

GLfloat bgColor[4] = { 1, 1, 1, 1 };

const char* defaultMeshFileName = "m01_dinosaur.off";

MatrixXf vertex;
MatrixXf normal;
ArrayXXi face;


float timeStep = 1.0f / 120;
float currTime = 0;

Matrix4f	T;
Quaternionf q1, q2;
Vector3f	p1, p2;

float interval = 3;

int	method = 4;

const char* methodString[] = {
	"Linear interpolation of position",
	"Linear interpolation of rotation matrices",
	"Linear interpolation of unit quaternions with normalization",
	"Spherical linear interpolation using Eigen's slerp",
	"Spherical linear interpolation using Eigen's AngleAxis (Log)",
	"Spherical linear interpolation using Eigen's AngleAxis variation"
};
int main(int argc, char* argv[])
{
	vsync = 0;

	const char* filename;
	if (argc >= 2)
		filename = argv[1];
	else
		filename = defaultMeshFileName;

	fovy = 16.1f;
	GLFWwindow* window = initializeOpenGL(argc, argv, bgColor);
	if (window == NULL)
		return -1;
	glfwSetKeyCallback(window, keyboard);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	reshape(window, windowW, windowH);

	cout << endl;
	for (int i = 0; i < 6; i++)
		cout << "Keyboard Input : " << (i + 1) << " for" << methodString[i] << endl;
	cout << endl;
	init(filename);

	float previous = (float)glfwGetTime();
	float elapsed = 0;

	while (!glfwWindowShouldClose(window)) {
		float now = (float)glfwGetTime();
		float delta = now - previous;
		previous = now;

		elapsed += delta;

		if (elapsed > timeStep) {
			if (!pause)
				update(elapsed);
			else
				update(0);
			elapsed = 0;
		}
		render(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
void init(const char* filename)
{
	cout << "Reading" << filename << endl;
	readMesh(filename, vertex, normal, face);

	T.setIdentity();

	p1 = Vector3f(-1, 0.5, 2);
	p2 = Vector3f(2, 0.5, -1);

	Vector3f v = p2 - p1;
	cout << "Distnace = " << v.norm() << endl << endl;

	Vector3f axis = Vector3f(1, 1, 1).normalized();
	float theta = 181;
	q1 = Quaternionf(AngleAxisf(0, axis));
	cout << "q1 = " << q1.w() << ", " << q1.vec().transpose() << endl << endl;

	q2 = Quaternionf(AngleAxisf(theta * float(M_PI) / 180, axis));
	cout << "q2 from theta = " << theta << " and axis = " << axis.transpose() << endl;
	cout << "q2 = " << q2.w() << ", " << q2.vec().transpose() << endl;

	AngleAxisf aa(q1.inverse() * q2);
	cout << "ql -> q2: Angle = " << aa.angle() / M_PI * 180 << " degree,";
	cout << "Axis = " << aa.axis().transpose() << endl << endl;

	Quaternionf q3(-q2.w(), -q2.x(), -q2.y(), -q2.z());
	cout << "q3 from  -q2" << endl;
	cout << "q3 = " << q3.w() << ", " << q3.vec().transpose() << endl;

	aa = AngleAxisf(q1.inverse() * q3);
	cout << "ql -> q3: Angle = " << aa.angle() / M_PI * 180 << " degree,";
	cout << "Axis = " << aa.axis().transpose() << endl << endl;
}
Matrix3f rlerp(float t, Quaternionf& q1, Quaternionf& q2)
{
	Matrix3f  R = (1 - t) * Matrix3f(q1) + t * Matrix3f(q2);
	return R;
}
Matrix3f qlerp(float t, Quaternionf& q1, Quaternionf& q2)
{
	Quaternionf q;

	q.w() = (1 - t) * q1.w() + t * q2.w();
	q.x() = (1 - t) * q1.x() + t * q2.x();
	q.y() = (1 - t) * q1.y() + t * q2.y();
	q.z() = (1 - t) * q1.z() + t * q2.z();

	q.normalize();

	return  Matrix3f(q);
}
void update(float elapsed)
{
	currTime += elapsed;

	int	n = int(currTime / interval);
	float s = (currTime - n * interval) / interval;
	float t = (s < 0.5) ? 2 * s : 2 * (1 - s);

	Vector3f p = (1 - t) * p1 + t * p2;
	T.block<3, 1>(0, 3) = p;

	if (method == 2)
		T.block<3, 3>(0, 0) = rlerp(t, q1, q2);

	if (method == 3)
		T.block<3, 3>(0, 0) = qlerp(t, q1, q2);

	if (method >= 4) {
		Quaternionf q;
		if (method == 4)
			q = q1.slerp(t, q2);
		if (method == 5 || method == 6) {
			AngleAxisf aa(q1.conjugate() * q2);
			aa.angle() *= t;
			if (method == 5)
				q = q1 * aa;
			if (method == 6)
				q = q1 * Quaternionf(aa);
		}
		T.block<3, 3>(0, 0) = Matrix3f(q);
	}
}
void drawMesh()
{
	GLfloat mat_ambient[4] = { 0.10f, 0.10f, 0.10f, 1 };
	GLfloat mat_diffuse[4] = { 0.95f, 0.95f, 0.95f, 1 };
	GLfloat mat_specular[4] = { 0.10f, 0.10f, 0.10f, 1 };
	GLfloat mat_shininess = 128;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < face.cols(); i++) {
		glNormal3fv(normal.col(face(0, i)).data());
		glVertex3fv(vertex.col(face(0, i)).data());
		glNormal3fv(normal.col(face(1, i)).data());
		glVertex3fv(vertex.col(face(1, i)).data());
		glNormal3fv(normal.col(face(2, i)).data());
		glVertex3fv(vertex.col(face(2, i)).data());
	}
	glEnd();
}
void render(GLFWwindow* window)
{
	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);

	glDisable(GL_LIGHTING);
	drawAxes(AXIS_LENGTH, AXIS_LINE_WIDTH);

	setupLight();

	glMultMatrixf(T.data());
	drawMesh();
}
void setupLight()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	GLfloat ambient[4] = { 0.1f, 0.1f, 0.1f, 1 };
	GLfloat diffuse[4] = { 1.0f, 1.0f, 1.0f, 1 };
	GLfloat specular[4] = { 1.0f, 1.0f, 1.0f, 1 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light.data());
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_Q:
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		case GLFW_KEY_1:
			method = 1;
			cout << methodString[method - 1] << endl;
			break;
		case GLFW_KEY_2:
			method = 2;
			cout << methodString[method - 1] << endl;
			break;
		case GLFW_KEY_3:
			method = 3;
			cout << methodString[method - 1] << endl;
			break;
		case GLFW_KEY_4:
			method = 4;
			cout << methodString[method - 1] << endl;
			break;
		case GLFW_KEY_5:
			method = 5;
			cout << methodString[method - 1] << endl;
			break;
		case GLFW_KEY_6:
			method = 6;
			cout << methodString[method - 1] << endl;
			break;
		case GLFW_KEY_SPACE:
			pause = !pause;
			break;
		}
	}
}
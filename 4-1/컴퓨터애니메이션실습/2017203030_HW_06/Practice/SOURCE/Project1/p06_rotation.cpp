#include "glSetup.h"
#include "mesh.h"

#include <Eigen/Dense>
using namespace Eigen;

#include <iostream>
using namespace std;

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif 
#include <math.h>


void init(const char * filename);
void setupLight();

void update();
void render(GLFWwindow* window);
void reshape(GLFWwindow* window, int w, int h);
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);

bool pause = false;

Vector3f eye(0, 0, 3);
Vector3f center(0, 0, 0);
Vector3f up(0, 1, 0);

Vector4f light(5.0, 5.0, 5.0, 0);



bool	axes = false;
float AXIS_LENGTH = 3;
float AXIS_LINE_WIDTH = 2;

GLfloat bgColor[4] = { 1, 1, 1, 1 };
const char* defaultMeshFileName = "m01_bunny.off";
	
bool incremental = false;

int main(int argc, char* argv[])
{
	vsync = 0;
	const char* filename;
	if (argc >= 2) 
		filename = argv[1];
	else	
		filename = defaultMeshFileName;
	perspectiveView = false; 

	GLFWwindow * window = initializeOpenGL(argc, argv, bgColor);
	if (window == NULL) 
		return -1;

	glfwSetKeyCallback(window, keyboard);

	glEnable(GL_DEPTH_TEST);  
	glEnable(GL_NORMALIZE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	reshape(window, windowW, windowH);
	init(filename);
	while (!glfwWindowShouldClose(window)){
		if (!pause) 
			update();
		render(window); 
		glfwSwapBuffers(window); 
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
} 
MatrixXf	vertexR, vertexQ, vertexO;
MatrixXf	normalR, normalQ, normalO;
ArrayXXi	face;

int frame = 0; 
int Nacc = 1;
Matrix4f Tc[3]; 
Matrix4f Ti[3]; 
Matrix3f R, Rn;
Quaternionf q;
Vector3f axis;

void init(const char * filename)
{
	cout << "Reading" << filename << endl;
	readMesh(filename, vertexO, normalO, face);
	vertexR = vertexQ = vertexO;
	normalR = normalQ = normalO;

	Affine3f	T;
	T = Translation3f(-1.2f, 0, 0) * Scaling(0.4f, 0.4f, 0.4f); Tc[0] = Ti[0] = T.matrix();
	T = Translation3f(0.0f, 0, 0) * Scaling(0.4f, 0.4f, 0.4f); Tc[1] = Ti[1] = T.matrix();
	T = Translation3f(1.2f, 0, 0) * Scaling(0.4f, 0.4f, 0.4f); Tc[2] = Ti[2] = T.matrix();

	R.setIdentity();
	Rn.setIdentity();
	q.setIdentity();

	cout << endl;
	cout << "Keyboard Input: space for play/pause" << endl;
	cout << "Keyboard Input: q/esc for quit" << endl;
	cout << endl;
	cout << "Keyboard input: i for basic/incremental rotation" << endl;
	cout << "Keyboard input: a for acceleration in incremental rotation" << endl;
	cout << "Keyboard input: x for axes on/off"<< endl;
 }

void basicRotation(){
	float angleInc = float(M_PI) / 1200; // In radian 156
		
	if (frame % 2400 == 0) 
		axis = Vector3f::Random().normalized();
	{
		AngleAxisf aa(angleInc * frame, axis);
		Matrix3f R(aa);
		for (int i = 0; i < vertexO.cols(); i++){
			vertexR.col(i) = R * vertexO.col(i); 
			normalR.col(i) = R * normalO.col(i);
		}
	}

	{
		Quaternionf q;
		q.w() = cos(angleInc * frame / 2.0f);
		q.vec() = axis * sin(angleInc * frame / 2.0);
		Quaternionf P;	
		P.w() = 0;
		Quaternionf P_new;
		for (int i = 0; i < vertexO.cols(); i++){
			P.vec() = vertexO.col(i);
			P_new = q * P * q.conjugate(); 
			vertexQ.col(i) = P_new.vec(); 

			P.vec() = normalO.col(i);
			P_new = q * P * q.inverse();
			normalQ.col(i) = P_new.vec();
		}
	}
			
	{
		AngleAxisf aa(angleInc * frame, axis);
		Matrix3f R(aa);

		Matrix4f Tt; 
		Tt.setIdentity(); 
		Tt.block<3, 3>(0, 0) = R;
		Tc[2] = Ti[2] * Tt;
		if (0)
		{
			Quaternionf q(aa);
			Matrix3f	R_from_q(q);
			Quaternionf q_from_R(R);
			AngleAxisf aa_from_q(q);
			AngleAxisf aa_from_R(R);
		}
	}
	frame++;		
} 
void incrementalRotation()
{
	float angleInc = float(M_PI) / 1200; // In radian 222
	for (int i = 0; i < Nacc; i++)
	{
		if (frame % 2400 == 0) 
			axis = Vector3f::Random().normalized();

		AngleAxisf aa(angleInc, axis); 

		{
			R = Matrix3f(aa) * R;
			Matrix4f	Tt; 
			Tt.setIdentity(); 
			Tt.block<3, 3>(0, 0) = R;

			Tc[0] = Ti[0] * Tt;
		}

		{
			Rn = Matrix3f(aa) * Rn;

			Rn /= pow(fabs(Rn.determinant()), 1.0f / 3.0f); 

			Matrix4f	Tt; 
			Tt.setIdentity(); 
			Tt.block<3, 3>(0, 0) = Rn;
			
			Tc[1] = Ti[1] * Tt;
		}
	
		{
			q = Quaternionf(aa) * q; 
			q.normalize(); 

			Matrix4f	Tt;
			Tt.setIdentity(); 
			Tt.block<3, 3>(0, 0) = Matrix3f(q);
			Tc[2] = Ti[2] * Tt;
		}

		if (frame% 2400 == 0){
			Vector3f	x = R.col(0), y = R.col(1), z = R.col(2);


			cout << endl;
			cout << "Unitary:\t" << x.norm() << "," << y.norm() << ", " << z.norm() << endl;
			cout << "Orthogonal:\t" << x.dot(y) << ", " << y.dot(z) << ", " << z.dot(x) << endl;
			cout << "Determinant:\t"  << R.determinant() << endl;
		}
		frame++;
	}
}
void update()
{
	if (incremental)	
		incrementalRotation();
	else	
		basicRotation();
} 

void drawMesh(const MatrixXf & vertex, const MatrixXf& normal){
	GLfloat mat_ambient[4] = { 0.1f, 0.1f, 0.1f, 1 }; 
	GLfloat mat_diffuse[4] = { 0.95f, 0.95f, 0.95f, 1 };
	GLfloat mat_specular[4] = { 0.5f, 0.5f, 0.5f, 1 };
	GLfloat mat_shininess = 128;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < face.cols(); i++){
		glNormal3fv(normal.col(face(0, i)).data()); 
		glVertex3fv(vertex.col(face(0, i)).data());
		glNormal3fv(normal.col(face(1, i)).data()); 
		glVertex3fv(vertex.col(face(1, i)).data());
		glNormal3fv(normal.col(face(2, i)).data());
		glVertex3fv(vertex.col(face(2, i)).data());
	}
	glEnd();
 } 
void render(GLFWwindow * window)
{
	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);

	if (axes)
	{
		glDisable(GL_LIGHTING);
		drawAxes(AXIS_LENGTH, AXIS_LINE_WIDTH * dpiScaling);
	}
	
	setupLight();
	
	if (incremental)
	{
		for (int i = 0; i < 3; i++)
		{
			glPushMatrix();
			glMultMatrixf(Tc[i].data()); 
			drawMesh(vertexO, normalO);
			glPopMatrix();
		}
	}
	else {
		glPushMatrix();
		glMultMatrixf(Ti[0].data());
		drawMesh(vertexR, normalR);
		glPopMatrix();

		glPushMatrix();
		glMultMatrixf(Ti[1].data());
		drawMesh(vertexQ, normalQ);
		glPopMatrix();
							
		glPushMatrix();
		glMultMatrixf(Tc[2].data());
		drawMesh(vertexO, normalO);
		glPopMatrix();
	}
}

void setupLight()
{
	glEnable(GL_LIGHTING); 
	glEnable(GL_LIGHT0);

	GLfloat ambient[4] = { 0.1f, 0.1, 0.1f, 1 }; 
	GLfloat diffuse[4] = { 1.0f, 1.0f, 1.0f, 1 }; 
	GLfloat specular[4] = { 1.0f, 1.0f, 1.0f, 1 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light.data());

} 
void keyboard(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT){
		switch(key)
		{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE); 
			break;
		case GLFW_KEY_I : 
			incremental = !incremental; 
			break;
		case GLFW_KEY_A : 
			Nacc = (Nacc == 1) ? 100000 : 1; break;
		case GLFW_KEY_X : 
			axes = !axes;  
			break;
		case GLFW_KEY_SPACE : 
			pause = !pause; 
			break;
		}
	}
}



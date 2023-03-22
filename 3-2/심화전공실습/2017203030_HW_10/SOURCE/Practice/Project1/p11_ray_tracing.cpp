#include "glSetup.h"

#include <glm/glm.hpp>	// OpenGL Mathematics
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>	// glm::to_string()
#include <glm/gtc/type_ptr.hpp>	// value_ptr()
using namespace glm;

#include <iostream>
#include <fstream>
using namespace std;

#include <omp.h>	// OpenMP for parallel computing

void	init();
void	quit();
void	keyboard(GLFWwindow* window, int key, int code, int action, int mods);

// Paly configuration
bool	pause = false;

float	timeStep = 1.0f / 60;	// 60fps
float	period = 4.0;

// Current time and frame
float	currTime = 0;

// Raytracing depth
int	DEPTH = 1;

// Predefined 7 spheres
int	nSpheres = 0;
vec3	center_world[7];	// In the world coordinate system
float	radius[7];

// Raytracing on demand
bool	rayTracingRequired = true;

// How to draw rendered images texture mapping/direct drawing pixels
bool	textureMapping = true;

bool	profiling = false;

// OpenMP
bool	useOpenMP = true;

// Ray
struct Ray
{
	vec3	p0;// Start
	vec3	p1; // End

	Ray(const vec3& _p0, const vec3& _p1) { p0 = _p0; p1 = _p1; }
};

// Light
struct Light
{
	vec4	p;	// Position or direction
	vec3	p_eye; // In the eye coordinate system

	vec3	ambient;
	vec3	diffuse;
	vec3	specular;
};

// Ray-traced image
GLubyte* image = NULL;

int m = 0, m_prev = -1;	// Width of the image	m = windowW
int n = 0, n_prev = -1;	// Height of the image n = windowH
float r = 0;			// Aspect ratio

// View x Model matrix
mat4	viewModel;

// Camera configuation for ray tracing
vec3	eye(0, 0, 8);
vec3	up(0, 1, 0);
vec3	center(0, 0, 0);

// Light configuration
int	nLights = 0;
Light	light[2];

// Material configuration
vec3	m_ambient(0.1, 0.1, 0.1);
vec3	m_diffuse(0.95, 0.95, 0.95);
vec3	m_specular(0.5, 0.5, 0.5);
float	m_shininess = 25;

// Background intentisy
vec3	I_back(0.1, 0.1, 0.1);

// Colors
GLfloat bgColor[4] = { 0.1, 0.1, 0.1, 1 };

// Compute the reflection vector of 1 with respect to n 
vec3
reflect(const vec3& l, const vec3& n)
{
	return dot(2.0f * l, n) * n - l;
}

// Find the closest intersection with the sphere along the ray
float
findIntersection(const Ray& ray, const vec3& center, float radius, vec3& p, vec3& n)
{
	vec3	p10 = ray.p1 - ray.p0;
	vec3	p0c = ray.p0 - center;
	float	a = dot(p10, p10);
	float	b = 2 * dot(p10, p0c);
	float	c = dot(p0c, p0c) - radius * radius;

	float	D = b * b - 4 * a * c;
	if (D < 0) return -1;	// No intersection

	float	sqrtD = sqrt(D);

	// This	solution can produce a large round-off error.
	float	t0 = (-b + sqrtD) / (2 * a);
	float	t1 = (-b - sqrtD) / (2 * a);

	if (t0 < 0 && t1 < 0)	return -1;	// Clipped	
	if (t0 > 1 && t1 > 1)	return -1;	// Clipped

	float	t;
	if (t0 > 0 && t1 > 0)	t = std::min(t0, t1);	// Closer one
	else					t = std::max(t0, t1);	// Beyond 0

	p = (1 - t) * ray.p0 + t * ray.p1;	// The closest intersection point
	n = normalize(p - center);  	// Normal at the point

	return t;
}

// Find the closest intersection with the spheres along the ray except E
int
findIntersection(const Ray& ray, vec3& p, vec3& n, int E)
{
	// Find the closest intersection within [ray.p0, ray.p1]
	int	iSphere = -1;
	float	T = 1.0;	//	The camera faces the negative z-axis.
	for (int i = 0; i < nSpheres; i++)
	{
		if (i == E) continue;

		vec3	center = vec3(viewModel * vec4(center_world[i], 1));

		vec3	p_i, n_i;
		float	t = findIntersection(ray, center, radius[i], p_i, n_i);
		if (t < 0) continue;

		if (t <= T) { iSphere = i; T = t; p = p_i; n = n_i; }
	}

	return iSphere;
}

// Ambient intentisy
inline vec3
ambient(const Light& l)
{
	vec3	I(0, 0, 0);

	for (int i = 0; i < 3; i++)
		I[i] += m_ambient[i] * l.ambient[i];

	return I;
}

// Ambient, diffuse, specular
vec3
phong(const vec3& n, const vec3& v, const Light& l, const vec3& r)
{
	vec3	I = ambient(l);

	float	lambertian = std::max(dot(n, l.p_eye), 0.0f);
	if (lambertian > 0)
	{
		float specular = pow(std::max(dot(v, r), 0.0f), m_shininess);

		for (int i = 0; i < 3; i++)
		{
			I[i] += m_diffuse[i] * lambertian * l.diffuse[i];
			I[i] += m_specular[i] * specular * l.specular[i];
		}
	}

	return I;
}

//	Compute the intensity from ray using recursive ray casting.
//	Exclude an intersection with the object E where the ray start from. 196 vec3
vec3
intensity(const Ray& ray, const Light l[], int nLights, int depth, int E = -1)
{
	vec3	I(0, 0, 0);	// Final intensity

	// Find the closest intersection point and the normal
	vec3	p, n;// Position and normal in the eye coordinate system
	int	iObject = findIntersection(ray, p, n, E);

	if (iObject != -1) // Hit an object
	{
		for (int i = 0; i < nLights; i++)
		{
			// Shadow ray
			vec3	p_shadow, n_shadow; // Not used
			vec3	pDistantLight = p + 1.0e10f * l[i].p_eye;
			Ray		shadowRay(p, pDistantLight);

			int jObject = findIntersection(shadowRay, p_shadow, n_shadow, iObject);

			if (jObject == -1)	// Not shadowed
			{
				// Phong reflection	
				vec3	v = normalize(ray.p0 - ray.p1);			// Direction to the viewer
				vec3	r = normalize(reflect(l[i].p_eye, n));	// Reflection of light

				I += phong(n, v, l[i], r);
			}
			else	I += ambient(l[i]);		//Shadowed
		}

		// Recursive ray casting
		if (depth < DEPTH)
		{
			// Reflection ray
			vec3	r = normalize(reflect(ray.p0 - ray.p1, n));
			vec3	pr = p + 1.0E10f * r;	// Point far awary from p along r
			Ray	recursiveRay(p, pr);

			vec3	I_R = intensity(recursiveRay, l, nLights, depth + 1, iObject);

			for (int i = 0; i < 3; i++)
				I[i] += m_specular[i] * I_R[i];

			// Transmision ray
		}
	}
	else	I = I_back; // Hit nothing

	return I;
}

// The origin in OpenGL is at the bottom left.
inline void
setPixelValue(int i, int j, const vec3& I)
{
	int j_r = (n - 1) - j; 	// Upside down

	image[3 * m * j_r + 3 * i + 0] = (GLubyte)(I[0] * 255);
	image[3 * m * j_r + 3 * i + 1] = (GLubyte)(I[1] * 255);
	image[3 * m * j_r + 3 * i + 2] = (GLubyte)(I[2] * 255);
}

// Ray tracing	
void
rayTracing()
{
	// Viewing matrix	
	{
		// The camera faces the negative z-axis as in OpenGL.
		vec3	n = normalize(eye - center);
		vec3	u = normalize(cross(up, n));
		vec3	v = normalize(cross(n, u));

		viewModel = inverse(mat4(vec4(u, 0), vec4(v, 0), vec4(n, 0), vec4(eye, 1)));
	}

	// Direction to light in the eye coordinate system
	for (int i = 0; i < nLights; i++)
		light[i].p_eye = normalize(mat3(viewModel) * vec3(light[i].p));

	// Modeling matrix
	{
		// Rotate the spheres about the y-axis by theta degrees
		float	theta = 360.0f * currTime / period;
		vec3	axis(0, 1, 0);

		// viewModel = viewMode1 * rotation matrix
		viewModel = rotate(viewModel, radians(theta), axis);
	}

	// Perspective projection for ray tracing
	float fovy = 27.0;	// Field of view angle in degrees in the y direction (35mm lens)
	float dn = nearDist; // Near distance from COP
	float df = farDist;	// Far distance from COP

	// Size of the image plane in the workspace
	float h = dn * tan(radians(fovy));
	float w = h * r;

	// Pixel size in the image plane
	float	delta_w = w / m;
	float	delta_h = h / n;

	// Compute the intensity of each pixel in the image plane
#pragma omp parallel for schedule(dynamic) if (useOpenMP) // static, guided, auto, runtime
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
		{
			// Position in the near plane (image plane)
			float x_i = (-w / 2 + delta_w / 2) + delta_w * i;
			float y_j = (h / 2 - delta_h / 2) - delta_h * j;

			// Primary ray: The camera faces the negative z-axis as in OpenGL.
			vec3	s(x_i, y_j, -dn);	// Start point in the near plane
			vec3	e = (df / dn) * s; // End point in the far plane

			// Compute the RGB intensities using recursive ray casting
			vec3	I = intensity(Ray(s, e), light, nLights, 1);

			// Clamping the intensity values
			for (int k = 0; k < 3; k++)
				I[k] = std::min(I[k], 1.0f);

			// Store the pixel value
			setPixelValue(i, j, I);
		}
}

void
deleteStorageForImage()
{
	if (image) delete[]	image;
	image = NULL;
}

void
prepareStorageForImage()
{
	// Delete the previous strorage
	deleteStorageForImage();

	// Final image size
	cout << "Image size: " << m << " x " << n << endl;

	// Memory allocation for the ray-traced image
	image = new GLubyte[m * n * 3];
	if (image == NULL)
	{
		cout << "Image(" << m << ", " << n << ") allocation failure!" << endl;
		return;
	}
}

// Texture
void
loadRenderedImage()
{
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m, n, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void
init()
{
#pragma omp parallel
	{
#pragma omp master
		{
			int numThreads = omp_get_num_threads();
			cout << "# threads = " << numThreads << endl;
		}
	}

	// Two directional lights in this example
	nLights = 0;
	light[0].p = vec4(0.5, 0.5, 0.75, 0);	// Directional light
	light[0].ambient = vec3(1.0, 1.0, 1.0);
	light[0].diffuse = 0.5f * vec3(1.0, 1.0, 1.0);
	light[0].specular = vec3(1.0, 1.0, 1.0);
	nLights++;

	light[1].p = vec4(-0.5, 0.5, 0.75, 0); // Directional light
	light[1].ambient = vec3(1.0, 1.0, 1.0);
	light[1].diffuse = 0.5f * vec3(1.0, 1.0, 1.0);
	light[1].specular = vec3(1.0, 1.0, 1.0);
	nLights++;

	// Set the properties of the predefined 7 spheres	
	float	d = 1.0f;
	float	r = 1.414f * d;
	center_world[nSpheres] = vec3(0, 0, 0);		radius[nSpheres] = 0.7f;  	nSpheres++;
	center_world[nSpheres] = vec3(-d, -d, 0);	radius[nSpheres] = 0.5f; 	nSpheres++;
	center_world[nSpheres] = vec3(-d, d, 0);	radius[nSpheres] = 0.5f;	nSpheres++;
	center_world[nSpheres] = vec3(d, d, 0);		radius[nSpheres] = 0.5f;	nSpheres++;
	center_world[nSpheres] = vec3(d, -d, 0);	radius[nSpheres] = 0.5f;	nSpheres++;
	center_world[nSpheres] = vec3(0, 0, r);		radius[nSpheres] = 0.5f;	nSpheres++;
	center_world[nSpheres] = vec3(0, 0, -r);	radius[nSpheres] = 0.5f;	nSpheres++;

	// Keyboard		
	cout << endl;
	cout << "Keyboard	input :	space for play/pause" << endl;
	cout << "Keyboard	input :	up for increasing specular" << endl;
	cout << "Keyboard	input :	down for decreasing specular" << endl;
	cout << "Keyboard	input :	t for texture mapping/direct drawing" << endl;
	cout << "Keyboard	input :	p for parallel computing on/off" << endl;
	cout << "Keyboard	input :	[1: 9] for ray tracing depth" << endl;
}

void
quit()
{
	// Delete image
	deleteStorageForImage();
}

// Textured quad
void
drawTexturedQuad(float aspect)
{
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);

	glTexCoord2f(0, 1);	glVertex3f(-aspect, 1.0, 0);
	glTexCoord2f(1, 1);	glVertex3f(aspect, 1.0, 0);
	glTexCoord2f(1, 0);	glVertex3f(aspect, -1.0, 0);
	glTexCoord2f(0, 0);	glVertex3f(-aspect, -1.0, 0);
	glEnd();
}

int
main(int argc, char* argv[])
{
	// vsync should be 0 for precise time stepping.
	vsync = 0;

	// Orthographics projection to display the ray-traced image
	perspectiveView = false;

	// FPS control
	vsync = 0; // 0 for immediate mode (Tearing possible)

	// Initialize the OpenGL system
	GLFWwindow* window = initializeOpenGL(argc, argv, bgColor);
	if (window == NULL) return -1;

	// Callbacks
	glfwSetKeyCallback(window, keyboard);

	// No need for the depth test
	glDisable(GL_DEPTH_TEST);

	// To allow byte-alignment for an odd numbered width of the ray-traced image
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//Lighting
	glDisable(GL_LIGHTING);

	// Viewport and perspective setting
	reshape(window, windowW, windowH);

	// Initialization - Main loop - Finalization
	init();

	// Main loop
	float previous = (float)glfwGetTime();
	float elapsed = 0;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();			// Events

		// Time passed during a single loop
		float now = (float)glfwGetTime();
		float delta = now - previous;
		previous = now;

		// Time passed after the previous frame
		elapsed += delta;

		// Time stepping
		if (elapsed > timeStep)
		{
			if (profiling) cout << "elapsed = " << elapsed << endl;

			if (!pause) // Animate if not paused
			{
				currTime += elapsed;
				rayTracingRequired = true; // Request for new ray tracing
			}

			elapsed = 0;	// Reset the elapsed time
		}


		//Image size: width and height
		m = windowW;	n = windowH;

		// Have the image size been changed?
		if (m_prev != m || n_prev != n)
		{
			// Aspect ratio
			r = aspect; // This is the same with float(m) / n.

			// Storage for the ray-traced image
			prepareStorageForImage();
			rayTracingRequired = true; // Request for new ray tracing

			m_prev = m; n_prev = n; // Keep the previous size
		}


		//Ray tracing if requested
		if (rayTracingRequired)
		{
			rayTracing();
			rayTracingRequired = false;

			if (textureMapping) // Employ texture mapping to display the ray-traced image
			{
				// Draw a textured opaque quad to display the ray-traced image
				glEnable(GL_TEXTURE_2D);
				loadRenderedImage();
				drawTexturedQuad(r);
			}
			else {	// Direct drawing to display the ray-traced image
				// Direct draw using glDrawPixe1s()
				glDisable(GL_TEXTURE_2D);
				glDrawPixels(m, n, GL_RGB, GL_UNSIGNED_BYTE, image);
			}

			glfwSwapBuffers(window);	// Swap buffers
		}
	}

	// Finalization
	quit();

	// Terminate the glfw system
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

// Reflectance control
void
increaseSpecular()
{
	for (int i = 0; i < 3; i++)
		m_specular[i] = std::min(m_specular[i] + 0.1f, 1.0f);

	rayTracingRequired = true;
}

void
decreaseSpecular()
{
	for (int i = 0; i < 3; i++)
		m_specular[i] = std::max(m_specular[i] - 0.1f, 0.0f);

	rayTracingRequired = true;
}

// Ray tracing depth control
void
setRayTracingDepth(int depth)
{
	DEPTH = depth;
	rayTracingRequired = true;
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
		case GLFW_KEY_SPACE: pause = !pause;	break;

			// Reflectance	
		case GLFW_KEY_UP: increaseSpecular();	break;
		case GLFW_KEY_DOWN: decreaseSpecular();	break;

			// Ray tracing depth	
		case GLFW_KEY_1: setRayTracingDepth(1);	break;
		case GLFW_KEY_2: setRayTracingDepth(2);	break;
		case GLFW_KEY_3: setRayTracingDepth(3);	break;
		case GLFW_KEY_4: setRayTracingDepth(4);	break;
		case GLFW_KEY_5: setRayTracingDepth(5);	break;
		case GLFW_KEY_6: setRayTracingDepth(6);	break;
		case GLFW_KEY_7: setRayTracingDepth(7);	break;
		case GLFW_KEY_8: setRayTracingDepth(8);	break;
		case GLFW_KEY_9: setRayTracingDepth(9);	break;

			// Direct drawing or texture mapping
		case GLFW_KEY_T:
			textureMapping = !textureMapping;
			profiling = true;
			if (textureMapping) cout << "Texture Mapping" << endl;
			else	cout << "Direct Drawing" << endl;
			break;

			// OpenMP
		case GLFW_KEY_P:	useOpenMP = !useOpenMP;
			if (useOpenMP) cout << "Parallel computing" << endl;
			else	cout << "Non-parallel computing" << endl;
			break;
		}
	}
}
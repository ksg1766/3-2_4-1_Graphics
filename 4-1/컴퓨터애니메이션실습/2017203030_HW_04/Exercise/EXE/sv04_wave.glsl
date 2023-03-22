#version 400

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;

out vec3 position;
out vec3 normal;

// Transformation matrices: GLSL employ column-major matrices.
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;	// Transpose of the inverse of modelViewMatrix

// Twisting
uniform float	twisting = 0.0;

uniform float A = 0.03;
uniform float F = 40.0;
uniform float phase = 0.0;

void
main(void)
{
	// The twisting angle is proportional to the distance from the origin.
	float angle = twisting * length(VertexPosition.xy);
	float cosLength = cos(angle);
	float sinLength = sin(angle);

	float l = length(VertexPosition.xy);

	// New position due to twisting
	float x = cosLength * VertexPosition.x - sinLength * VertexPosition.y;
	float y = sinLength * VertexPosition.x + cosLength * VertexPosition.y;
	float z = VertexPosition.z + A * sin(F * l + phase);

	vec4 newVertexPosition = vec4(x, y, z, 1.0);

	float eps = 0.0001;

	mat3 Jt;
	Jt[0][0] = cosLength; Jt[0][1] = -sinLength; Jt[0][2] = 0;
	Jt[1][0] = sinLength; Jt[1][1] = cosLength; Jt[1][2] = 0;
	Jt[2][0] = A * cos(F * l + phase) * F * VertexPosition.x / (l + eps);
	Jt[2][1] = A * cos(F * l + phase) * F * VertexPosition.y / (l + eps);
	Jt[2][2] = 1;
	
	vec3 newVertexNormal = inverse(Jt) * VertexNormal;

	// Position for the rasterization
	gl_Position = ModelViewProjectionMatrix * newVertexPosition;

	// Position and normal in the view coordinate system for the fragment shader
	position = vec3(ModelViewMatrix * newVertexPosition);
	normal = normalize(NormalMatrix * newVertexNormal);
}
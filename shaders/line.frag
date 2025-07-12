#version 450

// Input from Vertex Shader (Per Fragment)
layout (location=0) in vec3 vertex_color;

// Output declaration from this shader
layout (location = 0) out vec4 out_color;

// Uniform Buffer Sets (per Frame)
struct PointLight {
	vec4 position; // ignore w | in world space
	vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 inverseViewMatrix;
	vec3 directionalLightPosition;  // in world space
	vec4 directionalLightColor; // w is intensity
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[10];
	int numLights;
} ubo_0;

// Push Constants (per Fragment / Pixel) (128 Bytes guaranteed)
layout(push_constant) uniform Push {
  mat4 modelMatrix;
  vec3 color;
} push;


void main() {
	out_color = vec4(vertex_color, 1.0);
}
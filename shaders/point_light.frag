#version 450

// Input
layout (location=0) in vec2 fragment_offset;

// Output
layout (location=0) out vec4 out_color;

// Uniform Buffer Sets (per Frame)
struct PointLight {
	vec4 position; // ignore w | in world space
	vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform Uniform {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 directionalLightPosition;  // in world space
	vec4 directionalLightColor; // w is intensity
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[10];
	int numLights;
} uniform_0;

// Push Constants (128 Bytes guaranteed)
layout(push_constant) uniform Push {
	vec4 position;
	vec4 color;
	float radius;
} push;

void main() {
	// distance of the fragment is offset from the light position
	float dist = sqrt(dot(fragment_offset, fragment_offset));
	if (dist >= 1.0) {
		discard;
	}
	out_color = vec4(push.color.xyz, 1.0);
}
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
	mat4 inverseViewMatrix;
	vec3 directionalLightPosition;  // in world space
	vec4 directionalLightColor; // w is intensity
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[10];
	int numLights;
} ubo_0;

// Push Constants (128 Bytes guaranteed)
layout(push_constant) uniform Push {
	vec4 position;
	vec4 color;
	float radius;
} push;

const float M_PI = 3.1415926538;

void main() {
	// distance of the fragment is offset from the light position
	float dist = sqrt(dot(fragment_offset, fragment_offset));
	if (dist >= 1.0) {
		discard;
	}

	float cos_dis = .5 * (cos(dist * M_PI) + 1.0); // ranges from 1 -> 0

	out_color = vec4(push.color.xyz + cos_dis, cos_dis);
}
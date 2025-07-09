#version 450

// Input
layout (location=0) in vec2 fragment_offset;

// Output
layout (location=0) out vec4 out_color;

// Uniform Buffer Sets (per Frame)
layout(set = 0, binding = 0) uniform Uniform {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 directionalLightPosition;  // in world space
	vec4 directionalLightColor;
	vec4 ambientLightColor;
	vec3 positionalLightPosition; // in world space
	vec4 positionalLightColor;
} uniform_0;

void main() {
	// distance of the fragment is offset from the light position
	float dist = sqrt(dot(fragment_offset, fragment_offset));
	if (dist >= 1.0) {
		discard;
	}
	out_color = vec4(uniform_0.positionalLightColor.xyz, 1.0);
}
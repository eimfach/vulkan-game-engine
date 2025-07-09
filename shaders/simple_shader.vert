#version 450

// Vertex attributes input from buffer
layout(location=0) in vec3 vertex_position;
layout(location=1) in vec3 vertex_color;
layout(location=2) in vec3 vertex_normal;
layout(location=3) in vec3 vertex_uv;

// Output declaration from this shader for the next (fragment shader)
layout(location=0) out vec3 fragment_color;
layout (location = 1) out vec3 fragment_position_world_space;
layout (location = 2) out vec3 fragment_normal_world_space;

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

// ... more sets

// Push Constants (per Vertex) (128 Bytes guaranteed)
layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {
	vec4 vertex_position_world_space = push.modelMatrix * vec4(vertex_position, 1.0);
	vec3 vertex_normal_world_space = normalize(mat3(push.normalMatrix) * vertex_normal);
	fragment_normal_world_space = vertex_normal_world_space;
	fragment_position_world_space = vertex_position_world_space.xyz;
	fragment_color = vertex_color;

	gl_Position = uniform_0.projectionMatrix * uniform_0.viewMatrix * vertex_position_world_space;
}
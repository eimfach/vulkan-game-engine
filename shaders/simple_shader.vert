#version 450

// vertex attributes input from buffer
layout(location=0) in vec3 vertex_position;
layout(location=1) in vec3 vertex_color;
layout(location=2) in vec3 vertex_normal;
layout(location=3) in vec3 vertex_uv;

layout(location=0) out vec3 frag_color;

// Uniform Buffer Sets (per Frame)
layout(set = 0, binding = 0) uniform Uniform {
	mat4 projectionViewMatrix;
	vec3 directionToLight;
} uniform_0;

// Push Constants (per Vertex)
layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

const float AMBIENT = 0.02;

void main() {
	gl_Position = uniform_0.projectionViewMatrix * push.modelMatrix * vec4(vertex_position, 1.0);
	
	vec3 normal_world_space = normalize(mat3(push.normalMatrix) * vertex_normal);

	float light_intensity = AMBIENT + max(dot(normal_world_space, uniform_0.directionToLight), 0);
	frag_color = light_intensity * vertex_color;
}
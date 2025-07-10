#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);


// Output declaration from this shader for the next (fragment shader)
layout (location = 0) out vec2 fragment_offset;

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

// Push Constants (per Vertex) (128 Bytes guaranteed)
layout(push_constant) uniform Push {
	vec4 position;
	vec4 color;
	float radius;
} push;

void main() {
	// apply the offsets to the position in camera space, rather than doing 
	// the initial computation using the position in world space.

	vec2 vertex_offset = OFFSETS[gl_VertexIndex];
	fragment_offset = vertex_offset;

	// light in camera space
	vec4 light_position_camera_space = ubo_0.viewMatrix * push.position;
	// vertex offset to the camera position
	vec4 vertex_position_camera_space = light_position_camera_space + push.radius * vec4(vertex_offset.x, vertex_offset.y, 0.0, 0.0);
	gl_Position = ubo_0.projectionMatrix * vertex_position_camera_space;
}
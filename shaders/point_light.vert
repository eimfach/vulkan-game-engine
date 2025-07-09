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
layout(set = 0, binding = 0) uniform Uniform {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 directionalLightPosition;  // in world space
	vec4 directionalLightColor;
	vec4 ambientLightColor;
	vec3 positionalLightPosition; // in world space
	vec4 positionalLightColor;
} uniform_0;


const float LIGHT_RADIUS = .05;

void main() {
	// apply the offsets to the position in camera space, rather than doing 
	// the initial computation using the position in world space.

	vec2 vertex_offset = OFFSETS[gl_VertexIndex];
	fragment_offset = vertex_offset;

	// light in camera space
	vec4 light_position_camera_space = uniform_0.viewMatrix * vec4(uniform_0.positionalLightPosition, 1.0);
	vec4 vertex_position_camera_space = light_position_camera_space + LIGHT_RADIUS * vec4(vertex_offset.x, vertex_offset.y, 0.0, 0.0);
	gl_Position = uniform_0.projectionMatrix * vertex_position;
	// vertex offset to the camera position
}
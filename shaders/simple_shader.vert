#version 450

// Vertex attributes input from buffer
layout(location=0) in vec3 vertex_position;
layout(location=1) in vec3 vertex_color;
layout(location=2) in vec3 vertex_normal;
layout(location=3) in vec3 vertex_uv;

// Output declaration from this shader for the next (fragment shader)
layout(location=0) out vec3 frag_color;

// Uniform Buffer Sets (per Frame)
layout(set = 0, binding = 0) uniform Uniform {
	mat4 projectionViewMatrix;
	vec3 directionalLightPosition;  // in world space
	vec4 directionalLightColor;
	vec4 ambientLightColor;
	vec3 positionalLightPosition; // in world space
	vec4 positionalLightColor;
} uniform_0;

// ... more sets

// Push Constants (per Vertex) (128 Bytes guaranteed)
layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {
	vec4 vertex_position_world_space = push.modelMatrix * vec4(vertex_position, 1.0);
	gl_Position = uniform_0.projectionViewMatrix * vertex_position_world_space;
	
	vec3 vertex_normal_world_space = normalize(mat3(push.normalMatrix) * vertex_normal);

	// Create positional light
	vec3 direction_to_positional_light = uniform_0.positionalLightPosition - vertex_position_world_space.xyz;
	// distance squared
	float attenuation_positional_light = 1.0 / dot(direction_to_positional_light, direction_to_positional_light);

	vec3 positional_light_color_scaled = uniform_0.positionalLightColor.xyz * uniform_0.positionalLightColor.w * attenuation_positional_light;
	
	vec3 diffuse_light = positional_light_color_scaled * max(dot(vertex_normal_world_space, normalize(direction_to_positional_light)),0);

	// Create directional light
	vec3 normal_directional_light = normalize(uniform_0.directionalLightPosition);
	vec3 directional_light_color_scaled = uniform_0.directionalLightColor.xyz * uniform_0.directionalLightColor.w;
	
	vec3 directional_light = directional_light_color_scaled * max(dot(vertex_normal_world_space, normal_directional_light), 0);

	// Create ambient light
	vec3 ambient_light_color_scaled = uniform_0.ambientLightColor.xyz * uniform_0.ambientLightColor.w;

	// frag_color = directional_light_intensity * vertex_color;
	frag_color = (diffuse_light + directional_light + ambient_light_color_scaled) * vertex_color;
}
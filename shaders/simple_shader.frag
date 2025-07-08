#version 450

// Input from Vertex Shader
layout(location = 0) in vec3 fragment_color;
layout(location = 1) in vec3 fragment_position_world_space;
layout(location = 2) in vec3 fragment_normal_world_space; // needs to be normalized again

// Output declaration from this shader
layout (location = 0) out vec4 out_color;

// Uniform Buffer Sets (per Frame)
layout(set = 0, binding = 0) uniform Uniform {
	mat4 projectionViewMatrix;
	vec3 directionalLightPosition;  // in world space
	vec4 directionalLightColor;
	vec4 ambientLightColor;
	vec3 positionalLightPosition; // in world space
	vec4 positionalLightColor;
} uniform_0;

// Push Constants (per Fragment / Pixel) (128 Bytes guaranteed)
layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;


void main() {

	// ********** Create positional light
	vec3 direction_to_positional_light = uniform_0.positionalLightPosition - fragment_position_world_space;
	// distance squared
	float attenuation_positional_light = 1.0 / dot(direction_to_positional_light, direction_to_positional_light);

	vec3 positional_light_color_scaled = uniform_0.positionalLightColor.xyz * uniform_0.positionalLightColor.w * attenuation_positional_light;
	
	vec3 diffuse_light = positional_light_color_scaled * max(dot(normalize(fragment_normal_world_space), normalize(direction_to_positional_light)),0);

	// ********** Create directional light
	vec3 normal_directional_light = normalize(uniform_0.directionalLightPosition);
	vec3 directional_light_color_scaled = uniform_0.directionalLightColor.xyz * uniform_0.directionalLightColor.w;
	
	vec3 directional_light = directional_light_color_scaled * max(dot(normalize(fragment_normal_world_space), normal_directional_light), 0);

	// ********** Create ambient light
	vec3 ambient_light_color_scaled = uniform_0.ambientLightColor.xyz * uniform_0.ambientLightColor.w;

	out_color = vec4((diffuse_light + directional_light + ambient_light_color_scaled) * fragment_color, 1.0);
}
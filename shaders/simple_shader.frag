#version 450

// Input from Vertex Shader (Per Fragment)
layout(location = 0) in vec3 fragment_color;
layout(location = 1) in vec3 fragment_position_world_space;
layout(location = 2) in vec3 fragment_normal_world_space; // needs to be normalized again

// Output declaration from this shader
layout (location = 0) out vec4 out_color;

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

// Push Constants (per Fragment / Pixel) (128 Bytes guaranteed)
layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;


void main() {

	// ********** Create ambient light
	vec3 ambient_light_color_scaled = uniform_0.ambientLightColor.xyz * uniform_0.ambientLightColor.w;

	// ********** Create positional lights
	vec3 diffuse_light = vec3(0.0, 0.0, 0.0);
	vec3 surface_normal = normalize(fragment_normal_world_space);

	for (int i = 0; i < uniform_0.numLights; i++) {
		PointLight light = uniform_0.pointLights[i];
		vec3 direction_to_positional_light = light.position.xyz - fragment_position_world_space;
		// distance squared
		float attenuation_positional_light = 1.0 / dot(direction_to_positional_light, direction_to_positional_light);
		vec3 positional_light_color_scaled = light.color.xyz * light.color.w * attenuation_positional_light;
		float cosAngIncidence = max(dot(surface_normal, normalize(direction_to_positional_light)),0);
		diffuse_light += positional_light_color_scaled * cosAngIncidence;
	}

	// ********** Create directional light
	vec3 normal_directional_light = normalize(uniform_0.directionalLightPosition);
	vec3 directional_light_color_scaled = uniform_0.directionalLightColor.xyz * uniform_0.directionalLightColor.w;
	
	vec3 directional_light = directional_light_color_scaled * max(dot(normalize(fragment_normal_world_space), normal_directional_light), 0);

	out_color = vec4((diffuse_light + directional_light + ambient_light_color_scaled) * fragment_color, 1.0);
}
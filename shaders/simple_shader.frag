#version 450

// Input from Vertex Shader (Per Fragment)
layout (location=0) in vec3 fragment_color;
layout (location=1) in vec3 fragment_position_world_space;
layout (location=2) in vec3 fragment_normal_world_space; // needs to be normalized again
layout (location=3) in vec2 fragment_uv_coordinates;

// Output declaration from this shader
layout (location = 0) out vec4 out_color;

// Uniform Buffer Sets (per Frame)
struct PointLight {
	vec4 position; // ignore w | in world space
	vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 inverseViewMatrix;
	vec3 directionalLightPosition;  // in world space
	vec4 directionalLightColor; // w is intensity
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[10];
	int numLights;
} ubo_0;

layout(set = 0, binding = 1) uniform sampler2D image;

// Push Constants (per Fragment / Pixel) (128 Bytes guaranteed)
layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;


void main() {

	// ********** Create ambient light
	vec3 ambient_light_color_scaled = ubo_0.ambientLightColor.xyz * ubo_0.ambientLightColor.w;

	// ********** Create positional lights
	vec3 diffuse_light = vec3(0.0);
	vec3 specular_light = vec3(0.0);
	vec3 surface_normal = normalize(fragment_normal_world_space);

	// specular pre-calculations
	vec3 camera_position_world_space = ubo_0.inverseViewMatrix[3].xyz;
	
	vec3 direction_to_viewer = normalize(camera_position_world_space - fragment_position_world_space); // vector pointing to the viewer/camera position

	for (int i = 0; i < ubo_0.numLights; i++) {
		PointLight light = ubo_0.pointLights[i];
		vec3 direction_to_positional_light = light.position.xyz - fragment_position_world_space;
		// distance squared
		float attenuation_positional_light = 1.0 / dot(direction_to_positional_light, direction_to_positional_light);

		direction_to_positional_light = normalize(direction_to_positional_light);

		vec3 positional_light_color_scaled = light.color.xyz * light.color.w * attenuation_positional_light;
		float cosAngIncidence = max(dot(surface_normal, direction_to_positional_light),0);
		diffuse_light += positional_light_color_scaled * cosAngIncidence;

		// specular light
		vec3 half_angle = normalize(direction_to_positional_light + direction_to_viewer);
		float blinn_term = dot(surface_normal, half_angle);
		blinn_term = clamp(blinn_term, 0, 1); // ignore when light and viewer are on opposite sides of the surface
		blinn_term = pow(blinn_term, 32.0); // higher values -> sharper highlight
		specular_light += positional_light_color_scaled * blinn_term;
	}

	vec3 image_color = texture(image, fragment_uv_coordinates).rgb;

	// ********** Create directional light
	vec3 normal_directional_light = normalize(ubo_0.directionalLightPosition);
	vec3 directional_light_color_scaled = ubo_0.directionalLightColor.xyz * ubo_0.directionalLightColor.w;
	
	vec3 directional_light = directional_light_color_scaled * max(dot(normalize(fragment_normal_world_space), normal_directional_light), 0);

	out_color = vec4(directional_light + ambient_light_color_scaled + diffuse_light * image_color + specular_light * image_color, 1.0);
}
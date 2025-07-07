#version 450

// Input from Vertex Shader
layout(location = 0) in vec3 frag_color;

// Output declaration from this shader
layout (location = 0) out vec4 out_color;

// Push Constants (per Fragment / Pixel) (128 Bytes guaranteed)
layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;


void main() {
	out_color = vec4(frag_color, 1.0);
}
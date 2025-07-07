#version 450

layout(location = 0) in vec3 frag_color;

layout (location = 0) out vec4 out_color;

// Push Constants (per Fragment/ Pixel)
layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;


void main() {
	out_color = vec4(frag_color, 1.0);
}
#version 450

// vertex attributes input from buffer
layout(location=0) in vec2 position;
layout(location=1) in vec3 color;

//layout(location=0) out vec3 frag_color;

layout(push_constant) uniform Push {
  mat2 transform;
  vec2 offset;
  float rotation;
  vec3 color;
} push;


void main() {
	float s = sin(push.rotation);
	float c = cos(push.rotation);
	mat2 rot_matrix = mat2(c,s,-s,c);
	mat2 transf = rot_matrix * push.transform;
	gl_Position = vec4(rot_matrix * push.transform * position + push.offset, 0.0, 1.0);
//	frag_color = color;
}
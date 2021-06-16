#version 460

layout(location = 0) in vec2 pos;

layout(set = 0, binding = 0) uniform Camera {
	mat4 transform;
	mat4 projection;
};

void main() { gl_Position = projection * transform * vec4(pos, 0.0, 1.0); }

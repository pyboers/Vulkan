#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 fragColor;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
	gl_Position.y = gl_Position.y * (16.0/9.0);
	gl_PointSize = 1.0;
	fragColor = color;
}
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

uniform float offset;

void main() {
    gl_Position = vec4(abs(offset) * (aPos + vec3(offset * 0.3, 0.0, 0.0)), 1.0);
    ourColor = aColor;
}
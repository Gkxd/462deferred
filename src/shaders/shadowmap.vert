#version 330 core

uniform mat4 lightMVPMat;

in vec3 in_Position;

void main() {
    gl_Position = lightMVPMat * vec4(in_Position, 1);
}


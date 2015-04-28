#version 330 core

uniform mat4 cameraMVPMat;
uniform mat4 cameraMVMat;
uniform mat4 normalMat;

in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_TexCoord;

out vec3 interpolated_View;
out vec3 interpolated_Normal;
out vec2 interpolated_TexCoord;

void main() {
    gl_Position = cameraMVPMat * vec4(in_Position, 1);
    interpolated_Normal = (normalMat * vec4(in_Normal, 1)).xyz;
    interpolated_TexCoord = in_TexCoord;
    interpolated_View = (cameraMVMat * vec4(in_Position, 1)).xyz;
}
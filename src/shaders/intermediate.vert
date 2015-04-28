#version 330 core

uniform mat4 lightMVPMat;
uniform mat4 cameraMVPMat;
uniform mat4 modelMat;

uniform int lightType;
uniform vec3 lightPosition;

in vec3 in_Position;

out vec3 interpolated_LightDirection;
out vec4 interpolated_ShadowCoord;

void main() {
    gl_Position = cameraMVPMat * vec4(in_Position, 1);
    
    interpolated_ShadowCoord = lightMVPMat * vec4(in_Position, 1);    
    
    if (lightType == 1) { // Spotlight
        interpolated_LightDirection = lightPosition - (modelMat * vec4(in_Position, 1)).xyz;
    }
    else if (lightType == 2) { // Point Light
        interpolated_LightDirection = lightPosition - (modelMat * vec4(in_Position, 1)).xyz;
    }
}
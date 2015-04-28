#version 330 core

in vec3 interpolated_View;
in vec3 interpolated_Normal;
in vec2 interpolated_TexCoord;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 ambient;
layout(location = 2) out vec3 diffuse;
layout(location = 3) out vec3 specular;
layout(location = 4) out float specularEx;
layout(location = 5) out vec3 view;

uniform bool hasAmbientTexture;
uniform sampler2D ambientTexture;
uniform bool hasDiffuseTexture;
uniform sampler2D diffuseTexture;

uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float specularExponent;

void main() {
    normal = interpolated_Normal;
    
    if (hasAmbientTexture) {
        ambient = ambientColor;// * texture(ambientTexture, interpolated_TexCoord).rgb;
    }
    else {
        ambient = ambientColor;
    }
    
    if (hasDiffuseTexture) {
        diffuse = diffuseColor;// * texture(diffuseTexture, interpolated_TexCoord).rgb;
    }
    else {
        diffuse = diffuseColor;
    }
    
    specular = specularColor;
    specularEx = specularExponent;
    
    view = interpolated_View;
}
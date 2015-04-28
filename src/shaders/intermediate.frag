#version 330 core

// Shadow sampling reference:
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/

in vec3 interpolated_LightDirection;
in vec4 interpolated_ShadowCoord;

out vec4 lightVisibility;

uniform int lightType;
uniform sampler2D shadowMap;
uniform mat4 cameraVPMat;

uniform vec3 lightDirection; // For directional lights

vec2 poissonDisk[5] = vec2[](
    vec2(0, 0),
    vec2(1, 0),
    vec2(0, 1),
    vec2(-1, 0),
    vec2(0, -1)
);

void main() {
    if (lightType == 0) { // Directional Light
        float visibility = 1;
        vec2 UV = interpolated_ShadowCoord.xy/interpolated_ShadowCoord.w;
        if (texture(shadowMap, UV).z < (interpolated_ShadowCoord.z - 0.0015)/interpolated_ShadowCoord.w) {
            visibility = 0;
        }
        
        if (UV.x > 1 || UV.y > 1 || UV.x < 0 || UV.y < 0) {
            visibility = 1;
        }
        lightVisibility = vec4(lightDirection, visibility);
    }
    else if (lightType == 1) { // Spotlight
        float visibility = 1;
        for (int i = 0; i < 5; i++) {
            vec2 UV = (interpolated_ShadowCoord.xy + poissonDisk[i]/700)/interpolated_ShadowCoord.w;
            if (texture(shadowMap, UV).z < (interpolated_ShadowCoord.z - 0.0015)/interpolated_ShadowCoord.w) {
                visibility -= 0.2;
            }
        }
        
        lightVisibility = vec4(interpolated_LightDirection, visibility);
    }
    else if (lightType == 2) { // Point Light
        lightVisibility = vec4(interpolated_LightDirection, 1);
    }
}
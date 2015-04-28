#version 330 core
 
in vec2 UV;
 
out vec3 color;

uniform sampler2D lightMap;
uniform sampler2D normalTexture;
uniform sampler2D ambientTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D specularExponentTexture;
uniform sampler2D viewTexture;

uniform mat4 normalMatrix; // For transforming light directions into camera space

uniform float ambientLight; // Should be black for all lights except sunlight
uniform vec3 lightColor;
uniform vec3 lightAttenuation;
uniform vec3 spotlightDirection; // For spotlights
uniform float cosHalfLightAngle; // For spotlights
uniform float spotlightFalloff; // For spotlights
uniform int lightType;
 
void main(){
    vec3 normal = normalize(texture(normalTexture, UV).xyz);
    vec4 lightInfo = texture(lightMap, UV);
    vec3 lightVector = lightInfo.xyz;
    vec3 lightDirection = (normalMatrix * vec4(normalize(lightVector), 1)).xyz;
    
    float lightDistance = length(lightVector);
    float lightVisibility = lightInfo.w;
    
    vec3 spotDirection = (normalMatrix * vec4(normalize(spotlightDirection), 1)).xyz;
    
    vec3 view = texture(viewTexture, UV).xyz;
    vec3 viewDir = normalize(-view);
    vec3 reflectedLight = normalize(-reflect(lightDirection, normal));
    vec3 reflectedSpot = normalize(-reflect(spotDirection, normal));
    
    float attenuation = 1;
    if (lightDistance > 1) {
        attenuation = clamp(
            1 / (lightAttenuation.x + lightAttenuation.y*lightDistance + lightAttenuation.z*lightDistance*lightDistance),
            0, 1);
    }
    
    if (lightType == 0) {
        attenuation = 1;
    }
    
    if (lightType == 1) {
        float cosAngleToLightCenter = dot(-lightDirection, normalize(spotDirection));
        if (cosAngleToLightCenter < cosHalfLightAngle) {
            attenuation = 0;
        }
        else {
            attenuation *= pow((cosAngleToLightCenter - cosHalfLightAngle)/(1 - cosHalfLightAngle), spotlightFalloff);
        }
    }
    
    vec3 ambientColor = texture(ambientTexture, UV).rgb;
    vec3 diffuseColor = texture(diffuseTexture, UV).rgb;
    vec3 specularColor = texture(specularTexture, UV).rgb;
    float specularExponent = texture(specularExponentTexture, UV).r;
    
    color = ambientColor * ambientLight;
    color += diffuseColor * lightColor * dot(normal, lightDirection) * attenuation * lightVisibility;
    if (lightType == 1) {
        color += specularColor * lightColor * pow(max(dot(viewDir, reflectedSpot), 0), specularExponent);
    }
    else {
        color += specularColor * lightColor * pow(max(dot(viewDir, reflectedLight), 0), specularExponent);
    }
    color *= clamp((40 - length(view)) / 10, 0, 1); // Depth fog
}
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

// Tekstury
uniform sampler2D texture1;
uniform sampler2D cubeTexture;
uniform samplerCube depthMap;
uniform sampler3D lutTexture;

// Parametry oświetlenia
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float farPlane;

// Flagi materiału
uniform bool isEmissive;
uniform bool isMatte;
uniform bool applyLUT = true; // Nowa flaga kontrolująca LUT

float ShadowCalculation(vec3 fragPos) {
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    
    vec3 sampleDir = normalize(fragToLight);
    float closestDepth = texture(depthMap, sampleDir).r;
    closestDepth *= farPlane;
    
    float bias = max(0.15 * (1.0 - dot(Normal, normalize(fragToLight))), 0.05);
    return (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
}

vec3 applyColorLUT(vec3 color) {
    // Normalizacja koloru do przestrzeni LUT
    color = clamp(color, 0.0, 1.0);
    
    // Pobranie wartości z tekstury 3D
    vec3 lutColor = texture(lutTexture, color).rgb;
    
    // Korekta gamma dla LUT (opcjonalnie)
    return pow(lutColor, vec3(1.0/2.2));
}

void main() {
    if (isEmissive) {
        vec3 texColor = texture(cubeTexture, TexCoords).rgb;
        vec3 lavaBoost = vec3(1.5, 1.0, 0.5);
        FragColor = vec4(texColor * lavaBoost * 1.5, 1.0);
        return;
    }

    // Obliczenia oświetlenia
    vec3 albedo = texture(texture1, TexCoords).rgb;
    vec3 ambient = vec3(0.0); // Brak światła otoczenia

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 lightColor = vec3(2.0, 0.7, 0.2);

    // Składowa dyfuzyjna
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * albedo;

    // Składowa spekularna
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = lightColor * spec;

    // Attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.05 * distance + 0.015 * (distance * distance));
    diffuse *= attenuation;
    specular *= attenuation;

    // Obliczenie cienia
    float shadow = ShadowCalculation(FragPos);
    vec3 result = ambient + (1.0 - shadow) * (diffuse + (isMatte ? vec3(0.0) : specular));

    // Zastosowanie LUT
    if (applyLUT) {
        result = applyColorLUT(result);
    }

    FragColor = vec4(result, 1.0);
}   
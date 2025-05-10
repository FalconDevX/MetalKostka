#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture1;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool isEmissive;

void main()
{
    vec3 textureColor = texture(texture1, TexCoords).rgb;

    if (isEmissive) {
        // Kostka świeci sama, bez wpływu światła
        FragColor = vec4(textureColor * 2.0, 1.0);   // możesz dać np. 5.0 żeby świeciła jeszcze mocniej
        return;
    }

    // ---------------- Lighting ----------------
    vec3 ambientColor = vec3(0.2);
    vec3 diffuseColor = vec3(1.0);   // większa intensywność światła
    vec3 specularColor = vec3(1.0);
    float shininess = 32.0;

    // Ambient
    vec3 ambient = ambientColor * textureColor;

    // Diffuse + attenuation
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (distance * distance);    // <- TU JEST KLUCZ! (kwadratowy spadek światła)

    vec3 diffuse = diffuseColor * diff * attenuation * textureColor;

    // Specular + attenuation
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularColor * spec * attenuation;

    // ---------------- Final color ----------------
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}

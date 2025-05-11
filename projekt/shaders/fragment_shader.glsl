#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture1;
uniform sampler2D cubeTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool isEmissive;

void main()
{
    if (isEmissive)
    {
        vec3 texColor = texture(cubeTexture, TexCoords).rgb;
        vec3 lavaBoost = vec3(1.5, 1.0, 0.5);
        FragColor = vec4(texColor * lavaBoost * 1.5, 1.0);
        return;
    }

    vec3 albedo = texture(texture1, TexCoords).rgb;
    vec3 ambient = 0.2 * albedo;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 lightColor = vec3(1.5, 0.5, 0.1);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * albedo;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = lightColor * spec;

    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 shadowMatrices[6]; // Tablica 6 macierzy dla ka¿dej œcianki cubemapy
uniform mat4 model;
uniform vec3 lightPos;          // Pozycja Ÿród³a œwiat³a (kostki)
uniform float farPlane;         // Do normalizacji g³êbokoœci

out vec4 FragPos; // Pozycja fragmentu w przestrzeni œwiata

void main() {
    FragPos = model * vec4(aPos, 1.0); // Transformacja wierzcho³ka przez model
    // Wybór macierzy na podstawie indeksu wierzcho³ka (6 wierzcho³ków na œciankê)
    gl_Position = shadowMatrices[gl_VertexID / 6] * FragPos; 
}
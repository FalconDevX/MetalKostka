#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 shadowMatrices[6]; // Tablica 6 macierzy dla ka�dej �cianki cubemapy
uniform mat4 model;
uniform vec3 lightPos;          // Pozycja �r�d�a �wiat�a (kostki)
uniform float farPlane;         // Do normalizacji g��boko�ci

out vec4 FragPos; // Pozycja fragmentu w przestrzeni �wiata

void main() {
    FragPos = model * vec4(aPos, 1.0); // Transformacja wierzcho�ka przez model
    // Wyb�r macierzy na podstawie indeksu wierzcho�ka (6 wierzcho�k�w na �ciank�)
    gl_Position = shadowMatrices[gl_VertexID / 6] * FragPos; 
}
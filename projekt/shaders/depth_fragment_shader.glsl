#version 330 core
in vec4 FragPos;       // Pozycja fragmentu z vertex shadera
uniform vec3 lightPos; // Pozycja �wiat�a (kostki)
uniform float farPlane;

void main() {
    // Oblicz odleg�o�� od �wiat�a do fragmentu i normalizuj do [0, 1]
    float lightDistance = length(FragPos.xyz - lightPos);
    gl_FragDepth = lightDistance / farPlane; 
}
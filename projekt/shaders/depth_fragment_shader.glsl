#version 330 core
in vec4 FragPos;       // Pozycja fragmentu z vertex shadera
uniform vec3 lightPos; // Pozycja œwiat³a (kostki)
uniform float farPlane;

void main() {
    // Oblicz odleg³oœæ od œwiat³a do fragmentu i normalizuj do [0, 1]
    float lightDistance = length(FragPos.xyz - lightPos);
    gl_FragDepth = lightDistance / farPlane; 
}
#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void generateSphere(GLuint& VAO, GLuint& VBO, int& vertexCount, int X_SEGMENTS = 64, int Y_SEGMENTS = 64)
{
    std::vector<glm::vec3> positions;
    std::vector<GLuint> indices;

    for (int y = 0; y <= Y_SEGMENTS; ++y) {
        for (int x = 0; x <= X_SEGMENTS; ++x) {
            float xSegment = (float)x / X_SEGMENTS;
            float ySegment = (float)y / Y_SEGMENTS;
            float xPos = cos(xSegment * 2.0f * M_PI) * sin(ySegment * M_PI);
            float yPos = cos(ySegment * M_PI);
            float zPos = sin(xSegment * 2.0f * M_PI) * sin(ySegment * M_PI);
            positions.push_back(glm::vec3(xPos, yPos, zPos));
        }
    }

    for (int y = 0; y < Y_SEGMENTS; ++y) {
        for (int x = 0; x < X_SEGMENTS; ++x) {
            int i0 = y * (X_SEGMENTS + 1) + x;
            int i1 = (y + 1) * (X_SEGMENTS + 1) + x;
            int i2 = i0 + 1;
            int i3 = i1 + 1;
            indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
            indices.push_back(i2); indices.push_back(i1); indices.push_back(i3);
        }
    }

    vertexCount = indices.size();

    std::vector<glm::vec3> vertices;
    for (GLuint i : indices)
        vertices.push_back(positions[i]);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
}

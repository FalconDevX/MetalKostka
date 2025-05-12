#include <SFML/Audio.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include "dependencies/include/tiny_obj_loader.h"
#include "dependencies/include/Camera.h"
#include "dependencies/include/Shader.h"
#include "cube_vertices.h"
#define STB_IMAGE_IMPLEMENTATION
#include "dependencies/include/stb/stb_image.h"


unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

Camera camera;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Zmienne do kontroli ruchu kostki
float rotationAngle = 0.0f;    // Kąt orbity wokół kowadła
float spinAngle = 0.0f;        // Kąt obrotu wokół własnej osi
const float orbitSpeed = 1.0f; // Prędkość orbity (radiany na sekundę)
const float spinSpeed = 2.0f;  // Prędkość obrotu kostki (radiany na sekundę)
const float orbitRadius = 3.0f;// Promień orbity

glm::vec3 cubePosition = glm::vec3(0.0f, 5.0f, 0.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
}


//do nieba

void generateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, unsigned int X_SEGMENTS = 64, unsigned int Y_SEGMENTS = 64) {
    const float PI = 3.14159265359f;
    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
            float xSegment = (float)x / X_SEGMENTS;
            float ySegment = (float)y / Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);
        }
    }

    for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
        for (unsigned int x = 0; x < X_SEGMENTS; ++x) {
            indices.push_back(y * (X_SEGMENTS + 1) + x);
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);

            indices.push_back(y * (X_SEGMENTS + 1) + x);
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
            indices.push_back(y * (X_SEGMENTS + 1) + x + 1);
        }
    }
}
GLuint loadEquirectangularTexture(const char* path) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Nie udało się załadować tekstury: " << path << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Ustaw parametry
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Shadow Mapping", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);
    sf::Music music;
    if (!music.openFromFile("sounds/templars.mp3"))
        std::cerr << "Błąd ładowania pliku muzycznego!" << std::endl;
    music.play();
    Shader shader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    Shader depthShader("shaders/depth_vertex_shader.glsl", "shaders/depth_fragment.glsl");
    
    //shader dla nieba
    Shader skyShader("shaders/sky_vertex.glsl", "shaders/sky_frag.glsl");
    std::vector<float> verticesSky;
    std::vector<unsigned int> indicesSky;
    generateSphere(verticesSky, indicesSky);

    float far_plane = 25.0f;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ---------- OBJ ----------
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "model.obj")) {
        std::cerr << "Failed to load model.obj: " << err << std::endl;
        exit(1);
    }

    std::vector<float> vertices;
    std::vector<glm::vec3> tempNormals;
    if (attrib.normals.empty()) {
        for (const auto& shape : shapes) {
            for (size_t j = 0; j < shape.mesh.indices.size(); j += 3) {
                auto idx0 = shape.mesh.indices[j];
                auto idx1 = shape.mesh.indices[j + 1];
                auto idx2 = shape.mesh.indices[j + 2];
                glm::vec3 v0(attrib.vertices[3 * idx0.vertex_index],
                    attrib.vertices[3 * idx0.vertex_index + 1],
                    attrib.vertices[3 * idx0.vertex_index + 2]);
                glm::vec3 v1(attrib.vertices[3 * idx1.vertex_index],
                    attrib.vertices[3 * idx1.vertex_index + 1],
                    attrib.vertices[3 * idx1.vertex_index + 2]);
                glm::vec3 v2(attrib.vertices[3 * idx2.vertex_index],
                    attrib.vertices[3 * idx2.vertex_index + 1],
                    attrib.vertices[3 * idx2.vertex_index + 2]);
                glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
                tempNormals.insert(tempNormals.end(), { normal, normal, normal });
            }
        }
    }

    unsigned int indexOffset = 0;
    for (const auto& shape : shapes) {
        for (auto idx : shape.mesh.indices) {
            float vx = attrib.vertices[3 * idx.vertex_index];
            float vy = attrib.vertices[3 * idx.vertex_index + 1];
            float vz = attrib.vertices[3 * idx.vertex_index + 2];
            float tx = 0.0f, ty = 0.0f;
            if (!attrib.texcoords.empty() && idx.texcoord_index >= 0) {
                tx = attrib.texcoords[2 * idx.texcoord_index];
                ty = attrib.texcoords[2 * idx.texcoord_index + 1];
            }
            float nx = 0.0f, ny = 0.0f, nz = 0.0f;
            if (!attrib.normals.empty() && idx.normal_index >= 0) {
                nx = attrib.normals[3 * idx.normal_index];
                ny = attrib.normals[3 * idx.normal_index + 1];
                nz = attrib.normals[3 * idx.normal_index + 2];
            }
            else if (!tempNormals.empty()) {
                auto n = tempNormals[indexOffset++];
                nx = n.x; ny = n.y; nz = n.z;
            }
            vertices.insert(vertices.end(), { vx, vy, vz, tx, ty, nx, ny, nz });
        }
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("textures/texture.png", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);

    GLuint cubeTexture;
    glGenTextures(1, &cubeTexture);
    glBindTexture(GL_TEXTURE_2D, cubeTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int cubeWidth, cubeHeight, cubeNrChannels;
    unsigned char* cubeData = stbi_load("textures/cubeTexture.jpg", &cubeWidth, &cubeHeight, &cubeNrChannels, 0);
    if (cubeData) {
        GLenum format = (cubeNrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, cubeWidth, cubeHeight, 0, format, GL_UNSIGNED_BYTE, cubeData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(cubeData);

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    GLuint cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeVerticesCount * sizeof(float), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    //niebo
    GLuint sVAO, sVBO, sEBO;
    glGenVertexArrays(1, &sVAO);
    glGenBuffers(1, &sVBO);
    glGenBuffers(1, &sEBO);

    glBindVertexArray(sVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sVBO);
    glBufferData(GL_ARRAY_BUFFER, verticesSky.size() * sizeof(float), &verticesSky[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSky.size() * sizeof(unsigned int), &indicesSky[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint equirectTex = loadEquirectangularTexture("textures/bg.jpg");
    //

    shader.use();
    shader.setInt("texture1", 0);
    shader.setInt("cubeTexture", 2);

    while (!glfwWindowShouldClose(window)) {

        glm::vec3 lightPos = cubePosition;
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        // Dodaj to ZARAZ po obliczeniu deltaTime
        rotationAngle += orbitSpeed * deltaTime;
        if (rotationAngle > 2 * glm::pi<float>())
            rotationAngle -= 2 * glm::pi<float>();

        // Aktualizacja pozycji kostki - ruch po okręgu
        cubePosition.x = orbitRadius * cos(rotationAngle);
        cubePosition.z = orbitRadius * sin(rotationAngle);
        cubePosition.y = 5.0f; // wysokość stała, jak wcześniej


        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, far_plane);
        std::vector<glm::mat4> shadowTransforms = {
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1,0,0),  glm::vec3(0,-1,0)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1,0,0), glm::vec3(0,-1,0)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0,1,0),  glm::vec3(0,0,1)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0,-1,0), glm::vec3(0,0,-1)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0,0,1),  glm::vec3(0,-1,0)),
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0,0,-1), glm::vec3(0,-1,0))
        };

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        depthShader.use();
        for (unsigned int i = 0; i < 6; ++i) {
            depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        }
        depthShader.setVec3("lightPos", lightPos);
        depthShader.setFloat("farPlane", far_plane);
        depthShader.setMat4("model", glm::mat4(1.0f));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 8);
        glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), cubePosition);
        depthShader.setMat4("model", cubeModel);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //niebo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDepthMask(GL_FALSE); // Wyłącz zapis do bufora głębokości
        skyShader.use();

        glm::mat4 projection = glm::perspective(glm::radians(60.0f), 800.f / 600.f, 0.1f, 100.0f);
        glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f));

        skyShader.setMat4("projection", projection);
        skyShader.setMat4("view", view);
        skyShader.setMat4("model", model);
        skyShader.setInt("equirectangularMap", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, equirectTex);
        glBindVertexArray(sVAO);
        skyShader.setInt("equirectangularMap", 0);

        glBindVertexArray(sVAO);
        glDrawElements(GL_TRIANGLES, indicesSky.size(), GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE); // Włącz zapis głębokości z powrotem
        //
        // 2. Normalne renderowanie sceny
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", lightPos);
        shader.setMat4("projection", glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f));
        shader.setMat4("view", camera.GetViewMatrix());
        shader.setInt("depthMap", 1);
        shader.setFloat("farPlane", far_plane);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        shader.setInt("depthMap", 1);

        shader.setBool("isEmissive", false);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        shader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 8);

        shader.setBool("isEmissive", true);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, cubeTexture);
        glBindVertexArray(cubeVAO);
        glm::mat4 cubeModel2 = glm::translate(glm::mat4(1.0f), cubePosition);
        shader.setMat4("model", cubeModel2);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
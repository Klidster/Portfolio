#include <iostream>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "file.h"
#include "shader.h"
#include "error.h"
#include "camera.h"
#include "texture.h"
#include "shadow.h"
#include "bitmap.h"
#include "casteljau.h"
#include "point.h"
#include "do_not_edit.h"

// Generate cube-sphere by subdividing cube faces and normalizing
void generateCubeSphereMesh(float radius, int subdivisions, std::vector<float>& V) {

    const glm::vec3 faceDirs[6] = {
        {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}
    };
    for (int f = 0; f < 6; ++f) {
        glm::vec3 N = faceDirs[f];

        glm::vec3 U = glm::normalize(glm::cross(N, glm::vec3(0, 1, 0)));
        if (f >= 2 && f < 4) U = glm::normalize(glm::cross(N, glm::vec3(0, 0, 1)));
        glm::vec3 W = glm::normalize(glm::cross(N, U));

        for (int i = 0; i < subdivisions; ++i) {
            for (int j = 0; j < subdivisions; ++j) {

                float a = i / (float)subdivisions * 2 - 1;
                float b = (i + 1) / (float)subdivisions * 2 - 1;
                float c = j / (float)subdivisions * 2 - 1;
                float d = (j + 1) / (float)subdivisions * 2 - 1;
                glm::vec3 P[4] = { N * radius + U * a * radius + W * c * radius,
                                 N * radius + U * b * radius + W * c * radius,
                                 N * radius + U * b * radius + W * d * radius,
                                 N * radius + U * a * radius + W * d * radius };

                for (int k = 0; k < 4; ++k) P[k] = glm::normalize(P[k]) * radius;

                auto push = [&](const glm::vec3& p) {
                    V.insert(V.end(), { p.x,p.y,p.z, 1,1,1, p.x / radius,p.y / radius,p.z / radius, 0,0 });
                    };

                push(P[0]); push(P[1]); push(P[2]);
                push(P[0]); push(P[2]); push(P[3]);
            }
        }
    }
}


const int WIDTH = 800, HEIGHT = 600;
bool keys[1024];
bool dragging = false;
double lastMouseX = 0, lastMouseY = 0;
float curveTime = 0.0f;
bool goingForward = true;
const float speed = 0.5f;

float s = 3.0f; // scale factor
float yOffset = -2.0f;
float length = 2.5f * s;
float roofHeight = 2.75f * s + yOffset; // slightly lower peak

float doorAngle = 0.0f; // radians
bool doorOpening = false;

glm::vec3 interactivePosition = glm::vec3(3.0f, 0.0f, 0.0f);

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        dragging = (action == GLFW_PRESS);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!dragging) return;

    double dx = xpos - lastMouseX;

    // Apply horizontal mouse drag to door rotation
    doorAngle += dx * 0.005f;
    doorAngle = glm::clamp(doorAngle, 0.0f, glm::radians(90.0f));

    lastMouseX = xpos;
    lastMouseY = ypos;
}


GLfloat potVertices[] = {
    // Bottom face
    -0.3f, 0.0f, -0.3f,   0.5f, 0.3f, 0.2f,   0, 1, 0,   0, 0,
     0.3f, 0.0f, -0.3f,   0.5f, 0.3f, 0.2f,   0, 1, 0,   1, 0,
     0.3f, 0.0f,  0.3f,   0.5f, 0.3f, 0.2f,   0, 1, 0,   1, 1,
    -0.3f, 0.0f, -0.3f,   0.5f, 0.3f, 0.2f,   0, 1, 0,   0, 0,
     0.3f, 0.0f,  0.3f,   0.5f, 0.3f, 0.2f,   0, 1, 0,   1, 1,
    -0.3f, 0.0f,  0.3f,   0.5f, 0.3f, 0.2f,   0, 1, 0,   0, 1,

    // Side walls
    // Front
    -0.3f, 0.0f,  0.3f,   0.4f, 0.2f, 0.1f,   0, 0, 1,   0, 0,
     0.3f, 0.0f,  0.3f,   0.4f, 0.2f, 0.1f,   0, 0, 1,   1, 0,
     0.3f, 0.4f,  0.3f,   0.4f, 0.2f, 0.1f,   0, 0, 1,   1, 1,
    -0.3f, 0.0f,  0.3f,   0.4f, 0.2f, 0.1f,   0, 0, 1,   0, 0,
     0.3f, 0.4f,  0.3f,   0.4f, 0.2f, 0.1f,   0, 0, 1,   1, 1,
    -0.3f, 0.4f,  0.3f,   0.4f, 0.2f, 0.1f,   0, 0, 1,   0, 1,

    // Back
    -0.3f, 0.0f, -0.3f,   0.4f, 0.2f, 0.1f,   0, 0,-1,   0, 0,
     0.3f, 0.0f, -0.3f,   0.4f, 0.2f, 0.1f,   0, 0,-1,   1, 0,
     0.3f, 0.4f, -0.3f,   0.4f, 0.2f, 0.1f,   0, 0,-1,   1, 1,
    -0.3f, 0.0f, -0.3f,   0.4f, 0.2f, 0.1f,   0, 0,-1,   0, 0,
     0.3f, 0.4f, -0.3f,   0.4f, 0.2f, 0.1f,   0, 0,-1,   1, 1,
    -0.3f, 0.4f, -0.3f,   0.4f, 0.2f, 0.1f,   0, 0,-1,   0, 1,

    // Left wall
    -0.3f, 0.0f, -0.3f,   0.4f, 0.2f, 0.1f,  -1, 0, 0,  0, 0,
    -0.3f, 0.0f,  0.3f,   0.4f, 0.2f, 0.1f,  -1, 0, 0,  1, 0,
    -0.3f, 0.4f,  0.3f,   0.4f, 0.2f, 0.1f,  -1, 0, 0,  1, 1,
    -0.3f, 0.0f, -0.3f,   0.4f, 0.2f, 0.1f,  -1, 0, 0,  0, 0,
    -0.3f, 0.4f,  0.3f,   0.4f, 0.2f, 0.1f,  -1, 0, 0,  1, 1,
    -0.3f, 0.4f, -0.3f,   0.4f, 0.2f, 0.1f,  -1, 0, 0,  0, 1,

    // Right wall
    0.3f, 0.0f, -0.3f,   0.4f, 0.2f, 0.1f,   1, 0, 0,  0, 0,
    0.3f, 0.0f,  0.3f,   0.4f, 0.2f, 0.1f,   1, 0, 0,  1, 0,
    0.3f, 0.4f,  0.3f,   0.4f, 0.2f, 0.1f,   1, 0, 0,  1, 1,
    0.3f, 0.0f, -0.3f,   0.4f, 0.2f, 0.1f,   1, 0, 0,  0, 0,
    0.3f, 0.4f,  0.3f,   0.4f, 0.2f, 0.1f,   1, 0, 0,  1, 1,
    0.3f, 0.4f, -0.3f,   0.4f, 0.2f, 0.1f,   1, 0, 0,  0, 1,

    // Soil Surface
    -0.25f, 0.4f, -0.25f,  0.2f, 0.1f, 0.0f,   0, 1, 0,   0, 0,
     0.25f, 0.4f, -0.25f,  0.2f, 0.1f, 0.0f,   0, 1, 0,   1, 0,
     0.25f, 0.4f,  0.25f,  0.2f, 0.1f, 0.0f,   0, 1, 0,   1, 1,
    -0.25f, 0.4f, -0.25f,  0.2f, 0.1f, 0.0f,   0, 1, 0,   0, 0,
     0.25f, 0.4f,  0.25f,  0.2f, 0.1f, 0.0f,   0, 1, 0,   1, 1,
    -0.25f, 0.4f,  0.25f,  0.2f, 0.1f, 0.0f,   0, 1, 0,   0, 1,

    // Stalk
    // Front face
    -0.025f, 0.4f,  0.025f,  0.1f,0.6f,0.1f,   0,0,1,  0,0,
     0.025f, 0.4f,  0.025f,  0.1f,0.6f,0.1f,   0,0,1,  1,0,
     0.025f, 1.4f,  0.025f,  0.1f,0.6f,0.1f,   0,0,1,  1,1,
    -0.025f, 0.4f,  0.025f,  0.1f,0.6f,0.1f,   0,0,1,  0,0,
     0.025f, 1.4f,  0.025f,  0.1f,0.6f,0.1f,   0,0,1,  1,1,
    -0.025f, 1.4f,  0.025f,  0.1f,0.6f,0.1f,   0,0,1,  0,1,

    // Right face
     0.025f, 0.4f,  0.025f,  0.1f,0.6f,0.1f,   1,0,0,  0,0,
     0.025f, 0.4f, -0.025f,  0.1f,0.6f,0.1f,   1,0,0,  1,0,
     0.025f, 1.4f, -0.025f,  0.1f,0.6f,0.1f,   1,0,0,  1,1,
     0.025f, 0.4f,  0.025f,  0.1f,0.6f,0.1f,   1,0,0,  0,0,
     0.025f, 1.4f, -0.025f,  0.1f,0.6f,0.1f,   1,0,0,  1,1,
     0.025f, 1.4f,  0.025f,  0.1f,0.6f,0.1f,   1,0,0,  0,1,

     // Back face
      0.025f, 0.4f, -0.025f,  0.1f,0.6f,0.1f,   0,0,-1, 0,0,
     -0.025f, 0.4f, -0.025f,  0.1f,0.6f,0.1f,   0,0,-1, 1,0,
     -0.025f, 1.4f, -0.025f,  0.1f,0.6f,0.1f,   0,0,-1, 1,1,
      0.025f, 0.4f, -0.025f,  0.1f,0.6f,0.1f,   0,0,-1, 0,0,
     -0.025f, 1.4f, -0.025f,  0.1f,0.6f,0.1f,   0,0,-1, 1,1,
      0.025f, 1.4f, -0.025f,  0.1f,0.6f,0.1f,   0,0,-1, 0,1,

      // Left face
      -0.025f, 0.4f, -0.025f,  0.1f,0.6f,0.1f,  -1,0,0, 0,0,
      -0.025f, 0.4f,  0.025f,  0.1f,0.6f,0.1f,  -1,0,0, 1,0,
      -0.025f, 1.4f,  0.025f,  0.1f,0.6f,0.1f,  -1,0,0, 1,1,
      -0.025f, 0.4f, -0.025f,  0.1f,0.6f,0.1f,  -1,0,0, 0,0,
      -0.025f, 1.4f,  0.025f,  0.1f,0.6f,0.1f,  -1,0,0, 1,1,
      -0.025f, 1.4f, -0.025f,  0.1f,0.6f,0.1f,  -1,0,0, 0,1,

      // Leaves
      // Leaf 1
       0.00f,1.00f,0.00f,   0.2f,0.8f,0.2f,   0.0f,1.0f,0.0f,   0.50f,0.00f,
      -0.10f,1.10f,0.05f,   0.2f,0.8f,0.2f,  -0.2f,0.9f,0.3f,   0.25f,0.50f,
       0.10f,1.10f,0.05f,   0.2f,0.8f,0.2f,   0.2f,0.9f,0.3f,   0.75f,0.50f,

      -0.10f,1.10f,0.05f,   0.2f,0.8f,0.2f,  -0.2f,0.9f,0.3f,   0.25f,0.50f,
       0.30f,1.10f,0.30f,   0.2f,0.8f,0.2f,   0.5f,0.5f,0.7f,   1.00f,1.00f,
       0.10f,1.10f,0.05f,   0.2f,0.8f,0.2f,   0.2f,0.9f,0.3f,   0.75f,0.50f,

       0.10f,1.10f,0.05f,   0.2f,0.8f,0.2f,   0.2f,0.9f,0.3f,   0.75f,0.50f,
       0.00f,1.00f,0.00f,   0.2f,0.8f,0.2f,   0.0f,1.0f,0.0f,   0.50f,0.00f,
       0.30f,1.10f,0.30f,   0.2f,0.8f,0.2f,   0.5f,0.5f,0.7f,   1.00f,1.00f,

       // Leaf 2
        0.00f,1.00f,0.00f,   0.2f,0.8f,0.2f,   0.0f,1.0f,0.0f,   0.50f,0.00f,
       -0.15f,1.05f,-0.15f,  0.2f,0.8f,0.2f,  -0.2f,0.9f,-0.3f,   0.35f,0.45f,
       -0.15f,1.15f,-0.15f,  0.2f,0.8f,0.2f,  -0.2f,0.8f,-0.3f,   0.45f,0.85f,

       -0.15f,1.05f,-0.15f,  0.2f,0.8f,0.2f,  -0.2f,0.9f,-0.3f,   0.35f,0.45f,
       -0.30f,1.10f,-0.30f,  0.2f,0.8f,0.2f,  -0.7f,0.5f,-0.7f,   1.00f,1.00f,
       -0.15f,1.15f,-0.15f,  0.2f,0.8f,0.2f,  -0.2f,0.8f,-0.3f,   0.45f,0.85f,

       -0.15f,1.15f,-0.15f,  0.2f,0.8f,0.2f,  -0.2f,0.8f,-0.3f,   0.45f,0.85f,
        0.00f,1.00f,0.00f,   0.2f,0.8f,0.2f,   0.0f,1.0f,0.0f,   0.50f,0.00f,
       -0.30f,1.10f,-0.30f,  0.2f,0.8f,0.2f,  -0.7f,0.5f,-0.7f,   1.00f,1.00f,

       // Leaf 3
        0.00f,1.10f,0.00f,   0.2f,0.8f,0.2f,   0.0f,1.0f,0.0f,   0.50f,0.10f,
       -0.10f,1.15f,0.15f,   0.2f,0.8f,0.2f,  -0.3f,0.9f,0.3f,   0.30f,0.50f,
       -0.10f,1.25f,0.15f,   0.2f,0.8f,0.2f,  -0.3f,0.7f,0.3f,   0.45f,0.90f,

       -0.10f,1.15f,0.15f,   0.2f,0.8f,0.2f,  -0.3f,0.9f,0.3f,   0.30f,0.50f,
        0.00f,1.30f,0.00f,   0.2f,0.8f,0.2f,   0.0f,0.7f,0.7f,    1.00f,1.00f,
       -0.10f,1.25f,0.15f,   0.2f,0.8f,0.2f,  -0.3f,0.7f,0.3f,   0.45f,0.90f,

       -0.10f,1.25f,0.15f,   0.2f,0.8f,0.2f,  -0.3f,0.7f,0.3f,   0.45f,0.90f,
        0.00f,1.10f,0.00f,   0.2f,0.8f,0.2f,   0.0f,1.0f,0.0f,   0.50f,0.10f,
        0.00f,1.30f,0.00f,   0.2f,0.8f,0.2f,   0.0f,0.7f,0.7f,    1.00f,1.00f,
};

GLfloat planeVertices[] = {

    -10.0f, 0.0f, -10.0f,  0.5, 0.5, 0.5,  0,1,0,  0,0,
     10.0f, 0.0f, -10.0f,  0.5, 0.5, 0.5,  0,1,0,  1,0,
     10.0f, 0.0f,  10.0f,  0.5, 0.5, 0.5,  0,1,0,  1,1,
     10.0f, 0.0f,  10.0f,  0.5, 0.5, 0.5,  0,1,0,  1,1,
    -10.0f, 0.0f,  10.0f,  0.5, 0.5, 0.5,  0,1,0,  0,1,
    -10.0f, 0.0f, -10.0f,  0.5, 0.5, 0.5,  0,1,0,  0,0,
};

std::vector<glm::vec3> doorVerts = {

    glm::vec3(-0.5f * s, 0 + yOffset, -1 * length),
    glm::vec3(0.5f * s, 2 * s + yOffset, -1 * length),
    glm::vec3(0.5f * s, 0 + yOffset, -1 * length),

    glm::vec3(-0.5f * s, 0 + yOffset, -1 * length),
    glm::vec3(-0.5f * s, 2 * s + yOffset, -1 * length),
    glm::vec3(0.5f * s, 2 * s + yOffset, -1 * length),
};

GLuint doorVAO = 0, doorVBO = 0;
glm::vec3 doorPivot = glm::vec3(-0.5f * s, 0 + yOffset, -1 * length);

struct Tri { glm::vec3 a, b, c; };
std::vector<Tri> transTris = {
    // Front Wall
   {{-1 * s, 0 + yOffset, -1 * length}, {-0.5f * s, 0 + yOffset, -1 * length}, {-0.5f * s, 2 * s + yOffset, -1 * length}},
   {{-1 * s, 0 + yOffset, -1 * length}, {-0.5f * s, 2 * s + yOffset, -1 * length}, {-1 * s, 2 * s + yOffset, -1 * length}},

   {{0.5f * s, 0 + yOffset, -1 * length}, {1 * s, 0 + yOffset, -1 * length}, {1 * s, 2 * s + yOffset, -1 * length}},
   {{0.5f * s, 0 + yOffset, -1 * length}, {1 * s, 2 * s + yOffset, -1 * length}, {0.5f * s, 2 * s + yOffset, -1 * length}},

   // Back wall
   {{-1 * s, 0 * s + yOffset, 1 * length}, {1 * s, 0 * s + yOffset, 1 * length}, {1 * s, 2 * s + yOffset, 1 * length}},
   {{-1 * s, 0 * s + yOffset, 1 * length}, {1 * s, 2 * s + yOffset, 1 * length}, {-1 * s, 2 * s + yOffset, 1 * length}},

   // Left wall
   {{-1 * s, 0 * s + yOffset, -1 * length}, {-1 * s, 0 * s + yOffset, 1 * length}, {-1 * s, 2 * s + yOffset, 1 * length}},
   {{-1 * s, 0 * s + yOffset, -1 * length}, {-1 * s, 2 * s + yOffset, 1 * length}, {-1 * s, 2 * s + yOffset, -1 * length}},

   // Right wall
   {{1 * s, 0 * s + yOffset, -1 * length}, {1 * s, 0 * s + yOffset, 1 * length}, {1 * s, 2 * s + yOffset, 1 * length}},
   {{1 * s, 0 * s + yOffset, -1 * length}, {1 * s, 2 * s + yOffset, 1 * length}, {1 * s, 2 * s + yOffset, -1 * length}},

   // Sloped roof
   {{-1 * s, 2 * s + yOffset, -1 * length}, {1 * s, 2 * s + yOffset, -1 * length}, {0 * s, roofHeight, 0}},
   {{-1 * s, 2 * s + yOffset, 1 * length}, {1 * s, 2 * s + yOffset, 1 * length}, {0 * s, roofHeight, 0}},
   {{-1 * s, 2 * s + yOffset, -1 * length}, {-1 * s, 2 * s + yOffset, 1 * length}, {0 * s, roofHeight, 0}},
   {{1 * s, 2 * s + yOffset, -1 * length}, {1 * s, 2 * s + yOffset, 1 * length}, {0 * s, roofHeight, 0}},
};

GLuint triVAO = 0, triVBO = 0;

struct Light {
    glm::vec3 direction = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
    glm::vec3 color = glm::vec3(1.0f);

    void apply(GLuint shaderProgram) const {
        glUseProgram(shaderProgram);
        glUniform3fv(glGetUniformLocation(shaderProgram, "light.direction"), 1, glm::value_ptr(direction));
        glUniform3fv(glGetUniformLocation(shaderProgram, "light.color"), 1, glm::value_ptr(color));
    }
};

struct Model {
    GLuint VAO = 0, VBO = 0;
    GLuint shaderProgram = 0;
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    int vertexCount = 0;

    void setup(const GLfloat* vertices, size_t size) {
        vertexCount = size / (11 * sizeof(float));
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
        glEnableVertexAttribArray(3);

        glBindVertexArray(0);
    }

    void draw(GLuint mvpLoc, GLuint modelLoc, const glm::mat4& viewProj) {
        glm::mat4 mvp = viewProj * modelMatrix;
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
};

GLuint objVAO = 0, objVBO = 0;
std::vector<triangle> objTris;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Bezier Curve (TR9)", NULL, NULL);
    glfwMakeContextCurrent(window);
    gl3wInit();

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwGetCursorPos(window, &lastMouseX, &lastMouseY);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(DebugMessageCallback, 0);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);



    SCamera camera;
    InitCamera(camera);
    float camDist = 25.f;

    Light light;
    glm::vec3 lightPos = glm::vec3(5.0f, 10.0f, 5.0f);
    ShadowStruct shadow = setup_shadowmap(1024, 1024);
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 lightProj = glm::ortho(-10.f, 10.f, -10.f, 10.f, 1.f, 30.f);
    glm::mat4 lightSpaceMatrix = lightProj * lightView;

    GLuint shader = CompileShader("basic.vert", "basic.frag");
    // Cache uniform locations
    GLint mvpLoc = glGetUniformLocation(shader, "MVP");
    GLint modelLoc = glGetUniformLocation(shader, "model");
    GLint cameraLoc = glGetUniformLocation(shader, "cameraPos");
    GLint lightSpaceLoc = glGetUniformLocation(shader, "lightSpaceMatrix");
    GLint texLoc = glGetUniformLocation(shader, "tex");
    GLint useTexLoc = glGetUniformLocation(shader, "useTexture");
    GLint overrideColorLoc = glGetUniformLocation(shader, "useOverrideColor");
    GLint alphaLoc = glGetUniformLocation(shader, "uAlpha");
    GLint lightDirLoc = glGetUniformLocation(shader, "light.direction");
    GLint lightColorLoc = glGetUniformLocation(shader, "light.color");
    GLint shadowMapLoc = glGetUniformLocation(shader, "shadowMap");

    // Load objects
    Model pot, ground, sphere;
    pot.shaderProgram = ground.shaderProgram = sphere.shaderProgram = shader;
    pot.shaderProgram = shader;
    ground.shaderProgram = shader;
    sphere.shaderProgram = shader;

    pot.setup(potVertices, sizeof(potVertices));
    ground.setup(planeVertices, sizeof(planeVertices));

    std::vector<float> sphereVertices;
    generateCubeSphereMesh(1.0f, 3, sphereVertices);
    sphere.setup(sphereVertices.data(), sphereVertices.size() * sizeof(float));



    std::vector<glm::vec3> triVerts;
    for (auto& t : transTris) {
        triVerts.push_back(t.a);
        triVerts.push_back(t.b);
        triVerts.push_back(t.c);
    }
    GLuint triVAO, triVBO;
    glGenVertexArrays(1, &triVAO);
    glGenBuffers(1, &triVBO);
    glBindVertexArray(triVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triVBO);
    glBufferData(GL_ARRAY_BUFFER, triVerts.size() * sizeof(glm::vec3),
        triVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    GLuint groundTex = setup_texture("grass.bmp");



    // Bezier Curve
    std::vector<point> controlPoints = {
        point(-7.f, 0.f, -length),
        point(-2.f, 10.f, 0.f),
        point(2.f, 10.f, 0.f),
        point(7.f, 0.f, length)
    };

    std::vector<point> curvePoints = EvaluateBezierCurve(controlPoints, 100);
    int curveVerts, curveFloats;
    float* curveData = MakeFloatsFromVector(curvePoints, curveVerts, curveFloats, 1.0f, 0.0f, 1.0f);

    GLuint curveVAO, curveVBO;
    glGenVertexArrays(1, &curveVAO);
    glGenBuffers(1, &curveVBO);

    glBindVertexArray(curveVAO);
    glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
    glBufferData(GL_ARRAY_BUFFER, curveFloats * sizeof(float), curveData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    delete[] curveData;

    glGenVertexArrays(1, &doorVAO);
    glGenBuffers(1, &doorVBO);
    glBindVertexArray(doorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, doorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * doorVerts.size(), doorVerts.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    std::vector<triangle> soldierTris;
    obj_parse("objs/soldier/soldier.obj", &soldierTris);

    GLuint soldierVAO, soldierVBO;
    std::vector<float> soldierVerts;
    for (const auto& t : soldierTris) {
        for (auto& v : { t.v1, t.v2, t.v3 }) {
            soldierVerts.insert(soldierVerts.end(), {
                v.pos.x, v.pos.y, v.pos.z,
                v.col.r, v.col.g, v.col.b,
                v.nor.x, v.nor.y, v.nor.z,
                0.0f, 0.0f 
                });
        }
    }

    glGenVertexArrays(1, &soldierVAO);
    glGenBuffers(1, &soldierVBO);
    glBindVertexArray(soldierVAO);
    glBindBuffer(GL_ARRAY_BUFFER, soldierVBO);
    glBufferData(GL_ARRAY_BUFFER, soldierVerts.size() * sizeof(float), soldierVerts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);

    float lastFrame = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        float xoff = 0.f, yoff = 0.f;
        if (keys[GLFW_KEY_LEFT])  xoff -= 45.f * deltaTime;
        if (keys[GLFW_KEY_RIGHT]) xoff += 45.f * deltaTime;
        if (keys[GLFW_KEY_UP])    yoff -= 45.f * deltaTime;
        if (keys[GLFW_KEY_DOWN])  yoff += 45.f * deltaTime;
        if (keys[GLFW_KEY_W])     camDist -= 8.f * deltaTime;
        if (keys[GLFW_KEY_S])     camDist += 8.f * deltaTime;

        glm::mat4 soldierModel = glm::translate(glm::mat4(1.0f),
            glm::vec3(6.0f, -2.0f, -3.0f));
        soldierModel = glm::scale(soldierModel, glm::vec3(2.0f));

        MoveAndOrientCamera(camera, glm::vec3(0), camDist, xoff, yoff);
        glm::mat4 view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), WIDTH / (float)HEIGHT, 0.1f, 100.f);
        glm::mat4 viewProj = projection * view;


        int idx = int(curveTime * (curveVerts - 1));
        idx = glm::clamp(idx, 0, curveVerts - 1);
        glm::vec3 animPos(curvePoints[idx].x,
            curvePoints[idx].y,
            curvePoints[idx].z);

        renderShadowMap(shadow, lightSpaceMatrix, [&]() {
            pot.draw(mvpLoc, modelLoc, lightSpaceMatrix);
            ground.draw(mvpLoc, modelLoc, lightSpaceMatrix);
            sphere.draw(mvpLoc, modelLoc, lightSpaceMatrix);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(soldierModel));
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE,
                glm::value_ptr(lightSpaceMatrix * soldierModel));
            glBindVertexArray(soldierVAO);
            glDrawArrays(GL_TRIANGLES, 0, soldierTris.size() * 3);
            });

        pot.modelMatrix = glm::rotate(glm::mat4(1.0f), currentFrame, glm::vec3(0.5f, 1.f, 0.f));
        ground.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, -2.0f, 0));
        sphere.modelMatrix = glm::translate(glm::mat4(1.f), animPos);


        if (goingForward) {
            curveTime += deltaTime * speed;
            if (curveTime >= 1.0f) {
                curveTime = 1.0f;
                goingForward = false;
            }
        }
        else {
            curveTime -= deltaTime * speed;
            if (curveTime <= 0.0f) {
                curveTime = 0.0f;
                goingForward = true;
            }
        }

        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);

        glUniform3fv(lightDirLoc, 1, glm::value_ptr(light.direction));
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(light.color));

        glUniform3fv(cameraLoc, 1, glm::value_ptr(camera.Position));
        glUniformMatrix4fv(lightSpaceLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadow.depthTex);
        glUniform1i(shadowMapLoc, 1);

        glUniform1i(useTexLoc, GL_FALSE);
        glUniform1i(overrideColorLoc, GL_FALSE);
        glUniform1f(alphaLoc, 1.0f);

        // Draw pot
        {
            glm::mat4 mvp = viewProj * pot.modelMatrix;
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(pot.modelMatrix));
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
            pot.draw(mvpLoc, modelLoc, viewProj);
        }

        // Draw ground
        glUniform1i(useTexLoc, GL_TRUE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, groundTex);
        glUniform1i(texLoc, 0);

        {
            glm::mat4 mvp = viewProj * ground.modelMatrix;
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ground.modelMatrix));
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
            ground.draw(mvpLoc, modelLoc, viewProj);
        }
        glUniform1i(useTexLoc, GL_FALSE);


        // Draw Bezier curve
        glBindVertexArray(curveVAO);
        glm::mat4 identityModel = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identityModel));
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(viewProj * identityModel));
        glDrawArrays(GL_LINE_STRIP, 0, curveVerts);

        sphere.modelMatrix = glm::translate(glm::mat4(1.f), animPos);

        glUseProgram(shader);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sphere.modelMatrix));
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(viewProj * sphere.modelMatrix));
        sphere.draw(mvpLoc, modelLoc, viewProj);


        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, -2.0f, -3.0f));
        model = glm::scale(model, glm::vec3(2.0f));
        glm::mat4 mvp = viewProj * model;
 
        glUseProgram(shader);
        glUniform1i(useTexLoc, 0);
        glUniform1i(overrideColorLoc, 1);
        glUniform3f(overrideColorLoc, 0.5f, 0.5f, 0.5f);
        glUniform1f(alphaLoc, 1.0f);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadow.depthTex);
        glUniform1i(shadowMapLoc, 1);
        
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
        
        glBindVertexArray(soldierVAO);
        glDrawArrays(GL_TRIANGLES, 0, soldierTris.size() * 3);
        
        glUniform1i(overrideColorLoc, 0);

        // Sort triangles by distance to camera
        std::map<float, Tri> sorted;
        for (const auto& tri : transTris) {
            glm::vec3 center = (tri.a + tri.b + tri.c) / 3.0f;
            float dist = glm::length(camera.Position - center);
            sorted[dist] = tri;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        glUniform1f(glGetUniformLocation(shader, "uAlpha"), 0.5f);
        glUniform1i(glGetUniformLocation(shader, "useOverrideColor"), GL_TRUE);
        glUniform3f(glGetUniformLocation(shader, "overrideColor"), 0.6f, 0.9f, 1.0f);


        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
            const Tri& t = it->second;
            glm::vec3 verts[3] = { t.a, t.b, t.c };

            glBindBuffer(GL_ARRAY_BUFFER, triVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
            glBindVertexArray(triVAO);

            glm::mat4 identity = glm::mat4(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identity));
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(viewProj * identity));
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        glUniform1i(glGetUniformLocation(shader, "useOverrideColor"), GL_FALSE);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        glUseProgram(shader);
        glUniform1f(glGetUniformLocation(shader, "uAlpha"), 0.5f);
        glUniform1i(glGetUniformLocation(shader, "useOverrideColor"), GL_TRUE);
        glUniform3f(glGetUniformLocation(shader, "overrideColor"), 0.6f, 0.9f, 1.0f);

        glBindVertexArray(doorVAO);
        glm::mat4 doorModel = glm::translate(glm::mat4(1.0f), doorPivot);
        doorModel = glm::rotate(doorModel, doorAngle, glm::vec3(0, 1, 0));
        doorModel = glm::translate(doorModel, -doorPivot);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(doorModel));
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(viewProj * doorModel));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1i(glGetUniformLocation(shader, "useOverrideColor"), GL_FALSE);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}

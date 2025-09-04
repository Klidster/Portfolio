#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aCol;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec2 aUV;

out vec3 vertexColor;
out vec3 FragPos;
out vec3 Normal;
out vec2 texCoord;
out vec4 shadowCoord;

uniform mat4 MVP;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main() {

    gl_Position = MVP * vec4(aPos, 1.0);
    vertexColor = aCol;
    texCoord = aUV;
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    shadowCoord = lightSpaceMatrix * vec4(FragPos, 1.0);
}
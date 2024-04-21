#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out mat4 viewMatrix;
out vec4 positon;
out vec3 normal;
out vec2 texCoord;
out vec3 cameraForwardf;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 transform;
uniform vec3 cameraForward;

void main()
{
    gl_Position = projection * view * model * (vec4(aPos, 1.0) + vec4(transform, 1.0f));
    viewMatrix = view;
    positon = model * (vec4(aPos, 1.0) + vec4(transform, 1.0f));
    normal = aNormal;
    texCoord = aTexCoord;
    cameraForwardf = cameraForward;
}
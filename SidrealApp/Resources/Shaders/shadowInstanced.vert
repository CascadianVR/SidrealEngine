#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 3) in mat4 instanceMatrix;

uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * instanceMatrix * vec4(aPos, 1.0);
}  
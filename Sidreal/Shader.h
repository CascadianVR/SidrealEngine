#pragma once

#include <glm/ext/matrix_float4x4.hpp>

namespace Shader
{
    unsigned int CreateShaderProgram(const char* vertexShader, const char* fragmentShader);
    void SetVec3f(unsigned int* shaderProgram, const char* uniformName, float vec3[3]);
    void SetInt1i(unsigned int* shaderProgram, const char* uniformName, int value);
    void SetInt1f(unsigned int* shaderProgram, const char* uniformName, float value);
    void SetMatrix4f(unsigned int* shaderProgram, const char* uniformName, glm::mat4 value);
}
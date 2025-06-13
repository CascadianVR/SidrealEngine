#pragma once

#include <glm/ext/matrix_float4x4.hpp>

namespace Shader
{
    unsigned int CreateShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath = nullptr);
    void SetUniform1i(unsigned int* shaderProgram, const char* uniformName, const int value);
    void SetUniform1f(unsigned int* shaderProgram, const char* uniformName, const float value);
    void SetUniform1fv(unsigned int* shaderProgram, const char* uniformName, const float* values, int count);
    void SetUniform3f(unsigned int* shaderProgram, const char* uniformName, const float vec3[3]);
    void SetUniform4f(unsigned int* shaderProgram, const char* uniformName, const float vec4[4]);
    void SetMatrix4f(unsigned int* shaderProgram, const char* uniformName, const glm::mat4 value);
}
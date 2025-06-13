#pragma once

#include <glm/ext/matrix_float4x4.hpp>

namespace Shader
{
    unsigned int CreateShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath = nullptr);
    void SetInt1i(unsigned int* shaderProgram, const char* uniformName, int value);
    void SetUniform1f(unsigned int* shaderProgram, const char* uniformName, float value);
    void SetUniform3f(unsigned int* shaderProgram, const char* uniformName, float vec3[3]);
    void SetUniform4f(unsigned int* shaderProgram, const char* uniformName, float vec4[4]);
    void SetMatrix4f(unsigned int* shaderProgram, const char* uniformName, glm::mat4 value);
}
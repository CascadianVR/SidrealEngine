#pragma once

#include <glm/ext/matrix_float4x4.hpp>

namespace Shader
{
    unsigned int CreateShaderProgram(const char* vertexShader, const char* fragmentShader);
    void SetShaderUniformVec3(unsigned int* shaderProgram, const char* uniformName, float vec3[3]);
    void SetShaderUniformInt1i(unsigned int* shaderProgram, const char* uniformName, int value);
    void SetShaderUniformgMatrix4fv(unsigned int* shaderProgram, const char* uniformName, glm::mat4 value);
}
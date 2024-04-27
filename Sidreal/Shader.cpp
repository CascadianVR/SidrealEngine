#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>

char* LoadShader(const char* path);

unsigned int Shader::CreateShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath)
{
    // Vertex Shader
    char* vertexShaderSource = LoadShader(vertexShaderPath);
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment Shader
    char* fragmentShaderSource = LoadShader(fragmentShaderPath);
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Shader Program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADERPROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Use newly made program
    glUseProgram(shaderProgram);

    // Delete programs since they are no longer needed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void Shader::SetVec3f(unsigned int* shaderProgram, const char* uniformName, float vec3[3])
{
    int vertexColorLocation = glGetUniformLocation(*shaderProgram, uniformName);
    glUniform3f(vertexColorLocation, vec3[0], vec3[1], vec3[2]);
}

void Shader::SetInt1i(unsigned int* shaderProgram, const char* uniformName, int value)
{
    int vertexColorLocation = glGetUniformLocation(*shaderProgram, uniformName);
    glUniform1i(vertexColorLocation, value);
}

void Shader::SetInt1f(unsigned int* shaderProgram, const char* uniformName, float value)
{
    int vertexColorLocation = glGetUniformLocation(*shaderProgram, uniformName);
    glUniform1f(vertexColorLocation, value);
}

void Shader::SetMatrix4f(unsigned int* shaderProgram, const char* uniformName, glm::mat4 value)
{
    int vertexColorLocation = glGetUniformLocation(*shaderProgram, uniformName);
    glUniformMatrix4fv(vertexColorLocation, 1, GL_FALSE, glm::value_ptr(value));
}

char* LoadShader(const char* path) {
    std::ifstream inputFile(path);

    // Check if the file is opened successfully
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: " << path << std::endl;
        return nullptr;
    }

    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    inputFile.close();

    std::string content = buffer.str();

    // Allocate memory for the character array
    char* shaderContent = new char[content.length() + 1];

    strcpy_s(shaderContent, content.length() + 1, content.c_str());

    return shaderContent;
}
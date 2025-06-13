#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>

char* LoadShader(const char* path);

std::unordered_map<const char*, int> uniformLocations;

unsigned int Shader::CreateShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath)
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
        std::cout << "ERROR::SHADER::FRAG::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Geometry Shader
    unsigned int geometryShader;
    if (geometryShaderPath != nullptr)
    {
		char* geometryShaderSource = LoadShader(geometryShaderPath);
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
		glCompileShader(geometryShader);
    
		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
			glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::GEOM::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}

    // Shader Program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    if (geometryShaderPath != nullptr)
    {
        glAttachShader(shaderProgram, geometryShader);
	}
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
    if (geometryShaderPath != nullptr)
    {
        glDeleteShader(geometryShader);
	}

    return shaderProgram;
}

int GetUniformLocation(unsigned int* shaderProgram, const char* uniformName)
{
    const char* lookupName = uniformName + *shaderProgram;

    if (uniformLocations.find(lookupName) != uniformLocations.end())
    {
        return uniformLocations[lookupName];
    }

    int location = glGetUniformLocation(*shaderProgram, uniformName);
    uniformLocations[lookupName] = location;
    return location;
}

void Shader::SetInt1i(unsigned int* shaderProgram, const char* uniformName, int value)
{
    int location = GetUniformLocation(shaderProgram, uniformName);
    glUniform1i(location, value);
}

void Shader::SetUniform1f(unsigned int* shaderProgram, const char* uniformName, float value)
{
    int location = GetUniformLocation(shaderProgram, uniformName);
    glUniform1f(location, value);
}

void Shader::SetUniform3f(unsigned int* shaderProgram, const char* uniformName, float vec3[3])
{
    int location = GetUniformLocation(shaderProgram, uniformName);
    glUniform3f(location, vec3[0], vec3[1], vec3[2]);
}

void Shader::SetUniform4f(unsigned int* shaderProgram, const char* uniformName, float vec4[4])
{
    int location = GetUniformLocation(shaderProgram, uniformName);
    glUniform4f(location, vec4[0], vec4[1], vec4[2], vec4[4]);
}

void Shader::SetMatrix4f(unsigned int* shaderProgram, const char* uniformName, glm::mat4 value)
{
    int location = GetUniformLocation(shaderProgram, uniformName);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
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
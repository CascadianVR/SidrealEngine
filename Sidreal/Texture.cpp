#include <iostream>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"
#include "Shader.h"

unsigned int Texture::CreateTexture(const char* path, unsigned int textureIndex)
{
    glActiveTexture(GL_TEXTURE0 + textureIndex);

    // Load texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    std::cout << path << std::endl;

    int width, height, nrChannels;
    stbi__vertically_flip_on_load = true;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cout << "Failed to load texture" << std::endl;
        return -1;
    }

    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    if (nrChannels == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else if (nrChannels == 4)
    {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
    else
    {
		std::cout << "Texture format not supported" << std::endl;
        return -1;
	}

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    return texture;
}

unsigned int Texture::CreateBindlessTexture(const char* path)
{
    // Load texture
    unsigned int texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    int width, height, nrChannels;
    stbi__vertically_flip_on_load = true;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    glTextureStorage2D(texture, 1, GL_RGB8, width, height);
    glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_2D);

    //stbi_image_free(data);

    return texture;
}

void Texture::SetActiveTexture(unsigned int* shaderProgram, unsigned int* texture, unsigned int index)
{
    glUseProgram(*shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *texture);
    Shader::SetShaderUniformInt1i(shaderProgram, "tex", 0);
}

void Texture::SetActiveBindlessTexture(unsigned int* shaderProgram, unsigned int* texture)
{
    glUseProgram(*shaderProgram);
    glBindTextureUnit(GL_TEXTURE0, *texture);
    Shader::SetShaderUniformInt1i(shaderProgram, "tex", 0);
}

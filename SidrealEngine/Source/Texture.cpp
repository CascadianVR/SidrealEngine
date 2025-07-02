#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "Utils\stb_image.h"
#include "Texture.h"
#include "Renderer\Shader.h"

Texture::TextureData Texture::LoadTexture2D(const char* path)
{
    // Load texture
    int width, height, numChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* imageData = stbi_load(path, &width, &height, &numChannels, 0);
    if (!imageData)
    {
        std::cout << "Failed to load texture at: " << path << std::endl;
        return Texture::TextureData{};
    }

	Texture::TextureData textureData;
    textureData.width = width;
    textureData.height = height;
    textureData.numChannels = numChannels;

    size_t dataSize = width * height * numChannels;
    textureData.data.assign(imageData, imageData + dataSize);
    stbi_image_free(imageData);

	return textureData;
}

Texture::TextureData Texture::CreateTexture2D(const unsigned char* rawData, size_t dataSize)
{
    int width, height, numChannels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* imageData = stbi_load_from_memory(rawData, static_cast<int>(dataSize), &width, &height, &numChannels, 0);
    if (!imageData)
    {
        //throw std::runtime_error("Failed to load texture from memory.");
        return Texture::TextureData{};
    }

	Texture::TextureData textureData;
    textureData.width = width;
    textureData.height = height;
    textureData.numChannels = numChannels;

    size_t newDataSize = width * height * numChannels;
    textureData.data.assign(imageData, imageData + newDataSize);
    stbi_image_free(imageData);

	return textureData;
}

unsigned int Texture::InitTexture2D_OpenGL(unsigned char* data, int width, int height, int numChannels)
{
    // Generate texture ID and bind it
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Upload texture data to GPU depending on the number of channels
    if (numChannels == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else if (numChannels == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else
    {
        std::cout << "Texture format not supported" << std::endl;
        return -1;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

unsigned int Texture::LoadTextureHDR(const char* path)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float* data = stbi_loadf(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

unsigned int Texture::CreateBindlessTexture(const char* path)
{
    // Load texture
    unsigned int texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
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

    stbi_image_free(data);

    return texture;
}

void Texture::SetActiveAndBindTexture(unsigned int texture, unsigned int index)
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void Texture::SetActiveAndBindTextureArray(unsigned int texture, unsigned int index)
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
}

void Texture::SetActiveBindlessTexture(unsigned int shaderProgram, unsigned int* texture)
{
    glUseProgram(shaderProgram);
    glBindTextureUnit(GL_TEXTURE0, *texture);
    Shader::SetUniform1i(&shaderProgram, "tex", 0);
}

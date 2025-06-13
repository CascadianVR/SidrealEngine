#include <glad/glad.h>
#include "MeshPrimative.h"
#include "ModelLoader.h"
#include "Renderer/Model.h"

using namespace ModelLoader;

Texture::Texture LoadDefaultTexture();


Model MeshPrimative::CreateCube()
{

    float vertices[] = {
        // positions          // texture coords
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,

        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };

    unsigned int indices[] = {
        // front and back
        0, 3, 2,
        2, 1, 0,
        4, 5, 6,
        6, 7 ,4,
        // left and right
        11, 8, 9,
        9, 10, 11,
        12, 13, 14,
        14, 15, 12,
        // bottom and top
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20,
    };


    // Create VBOs and IBOs
    std::vector<unsigned int> VAOs(1);
    std::vector<unsigned int> VBOs(1);
    std::vector<unsigned int> EBOs(1);

    // Generate VAO, VBOs, and IBOs
    glGenVertexArrays(1, VAOs.data());
    glGenBuffers(1, VBOs.data());
    glGenBuffers(1, EBOs.data());

    glBindVertexArray(VAOs[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    std::vector<Vertex> verticesVector;
    for (int i = 0; i < sizeof(vertices) / sizeof(float); i += 8)
    {
        Vertex vertex;
        vertex.Position = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
        vertex.Normal = glm::vec3(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
        vertex.TexCoords = glm::vec2(vertices[i + 6], vertices[i + 7]);
        verticesVector.push_back(vertex);
    }
    std::vector<unsigned int> indicesVector;
    for (int i = 0; i < sizeof(indices) / sizeof(unsigned int); i++)
    {
        indicesVector.push_back(indices[i]);
    }

    std::vector<Mesh> meshes;
    Mesh mesh;

    std::vector<Texture::Texture> textures;
    textures.push_back(LoadDefaultTexture());
    mesh.textures = textures;
    mesh.vertices = verticesVector;
    mesh.indices = indicesVector;
    mesh.VAO = VAOs[0];
    mesh.VBO = VBOs[0];
    mesh.EBO = EBOs[0];

    meshes.push_back(mesh);

    Model model;
    model.meshes = meshes;

    return model;
}

Model MeshPrimative::CreateQuad()
{
   float vertices[] = {
        0.8f,  0.8f, 0.0f, 1.0f, 1.0f,
        0.8f, -0.8f, 0.0f, 1.0f, 0.0f,
       -0.8f, -0.8f, 0.0f, 0.0f, 0.0f,
       -0.8f,  0.8f, 0.0f, 0.0f, 1.0f,
   };
   
   unsigned int indices[] = {
       0, 1, 3, // first triangle
       1, 2, 3  // second triangle
   };

   // Create VBOs and IBOs
   std::vector<unsigned int> VAOs(1);
   std::vector<unsigned int> VBOs(1);
   std::vector<unsigned int> EBOs(1);

   // Generate VAO, VBOs, and IBOs
   glGenVertexArrays(1, VAOs.data());
   glGenBuffers(1, VBOs.data());
   glGenBuffers(1, EBOs.data());

   glBindVertexArray(VAOs[0]);

   glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   // Position attribute
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(0);
   // Normal attribute
   //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
   //glEnableVertexAttribArray(1);
   // Texture attribute
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
   glEnableVertexAttribArray(2);

   std::vector<Vertex> verticesVector;
   for (int i = 0; i < sizeof(vertices) / sizeof(float); i += 8)
   {
       Vertex vertex;
       vertex.Position = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
       vertex.Normal = glm::vec3(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
       vertex.TexCoords = glm::vec2(vertices[i + 6], vertices[i + 7]);
       verticesVector.push_back(vertex);
   }
   std::vector<unsigned int> indicesVector;
   for (int i = 0; i < sizeof(indices) / sizeof(unsigned int); i++)
   {
       indicesVector.push_back(indices[i]);
   }

   std::vector<Mesh> meshes;
   Mesh mesh;
   mesh.vertices = verticesVector;
   mesh.indices = indicesVector;
   mesh.VAO = VAOs[0];
   mesh.VBO = VBOs[0];
   mesh.EBO = EBOs[0];

   meshes.push_back(mesh);

   Model model;
   model.meshes = meshes;

   return model;
}

Texture::Texture defaultTexture;
bool defaultLoaded;
Texture::Texture LoadDefaultTexture()
{
    Texture::Texture texture;

    // Load texture if not already loaded
    if (!defaultLoaded)
    {
        texture.id = Texture::CreateTexture2D("Resources\\default.png");
        texture.index = 21;
        texture.type = "DefaultTexture";
        texture.path = "Resources\\default.png";
        defaultTexture = texture;
        defaultLoaded = true;
    }
    else
    {
        texture = defaultTexture;
    }

    return texture;
}
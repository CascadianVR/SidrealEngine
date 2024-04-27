#include "ShadowVolume.h"
#include <glad/glad.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

bool lighting = false;

using namespace ShadowVolume;

void DrawShadow();


void RenderQuad(Surface* surf, float* position, bool clockwise)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position[0], position[1], position[2]));

    float* vertices = new float[12];
    if (clockwise)
    {
        // Front Cap
        vertices[0] = surf->vertices[0].x;
		vertices[1] = surf->vertices[0].y;
		vertices[2] = surf->vertices[0].z;

		vertices[3] = surf->vertices[1].x;
		vertices[4] = surf->vertices[1].y;
		vertices[5] = surf->vertices[1].z;

		vertices[6] = surf->vertices[2].x;
		vertices[7] = surf->vertices[2].y;
		vertices[8] = surf->vertices[2].z;

		vertices[9] = surf->vertices[3].x;
		vertices[10] = surf->vertices[3].y;
		vertices[11] = surf->vertices[3].z;
	}
	else
	{
        // Back Cap
		vertices[0] = surf->vertices[3].x;
        vertices[1] = surf->vertices[3].y;
        vertices[2] = surf->vertices[3].z;

        vertices[3] = surf->vertices[2].x;
        vertices[4] = surf->vertices[2].y;
        vertices[5] = surf->vertices[2].z;

        vertices[6] = surf->vertices[1].x;
        vertices[7] = surf->vertices[1].y;
        vertices[8] = surf->vertices[1].z;

        vertices[9] = surf->vertices[0].x;
        vertices[10] = surf->vertices[0].y;
        vertices[11] = surf->vertices[0].z;
    }

    // Create and bind VAO
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create VBO for vertex data
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set up vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Draw
    glDrawArrays(GL_QUADS, 0, 4);
}


void RenderWrappedQuad(Surface* surf, float* position, bool clockwise)
{
    glm::vec3 v[4];
    v[0] = glm::vec3(surf->vertices[0].x, surf->vertices[0].y, surf->vertices[0].z);
    v[1] = glm::vec3(surf->vertices[1].x, surf->vertices[1].y, surf->vertices[1].z);
    v[2] = glm::vec3(surf->vertices[2].x, surf->vertices[2].y, surf->vertices[2].z);
    v[3] = glm::vec3(surf->vertices[3].x, surf->vertices[3].y, surf->vertices[3].z);

    GLfloat vertices[5][3] = {
    {surf->vertices[0][0], surf->vertices[0][1], surf->vertices[0][2]},
    {v[0][0], v[0][1], v[0][2]},
    {surf->vertices[1][0], surf->vertices[1][1], surf->vertices[1][2]},
    {v[1][0], v[1][1], v[1][2]},
    {surf->vertices[2][0], surf->vertices[2][1], surf->vertices[2][2]}
    };

    GLuint indices[] = { 0, 1, 2, 3, 0 };

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);

    for (int i = 0; i < 4; ++i) {
        glDrawElements(GL_TRIANGLE_STRIP, 5, GL_UNSIGNED_INT, indices);
    }
}

void ShadowVolume::RenderSurfaceShadowVolume(Surface* surf, glm::vec3 surf_pos, glm::vec3 light_pos)
{
    glm::vec3 v[4];

    for (int i = 0; i < 4; i++)
    {
		v[i] = glm::vec3(surf->vertices[i][0], surf->vertices[i][1], surf->vertices[i][2]);

        glm::normalize(v[i]);

        v[i] = v[i] * 1000.0f;

        v[i] = v[i] + surf_pos;
	}

    RenderQuad(surf, (float*)v, true);
    RenderQuad(surf, (float*)v, false);
    RenderWrappedQuad(surf, (float*)v, false);

    for (int i = 0; i < 4; i++) {
        surf->vertices[i] -= surf_pos;
    }


}

void ShadowVolume::RenderCubeShadow(Cube* c)
{
    for (int i = 0; i < 6; i++) {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
        glEnable(GL_CULL_FACE);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0.0f, 100.0f);

        glCullFace(GL_FRONT);
        glStencilFunc(GL_ALWAYS, 0x0, 0xff);
        glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);

        RenderSurfaceShadowVolume(c->surfaces[i], c->position, glm::vec3(5.0f, 5.0f, 5.0f));

        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_CULL_FACE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        glStencilFunc(GL_NOTEQUAL, 0x0, 0xff);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        DrawShadow();
        glDisable(GL_STENCIL_TEST);
    }
}


void DrawShadow()
{
    // Store the previous projection matrix
    //glm::mat4 previousProjectionMatrix = glm::mat4(1.0f);
    //glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(previousProjectionMatrix));
    //
    //// Store the previous modelview matrix
    //glm::mat4 previousModelviewMatrix = glm::mat4(1.0f);
    //glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(previousModelviewMatrix));
    //
    //// Set the projection matrix
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //glOrtho(0, 1, 1, 0, 0, 1);
    //
    //// Set the modelview matrix
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    //
    //// Disable depth test
    //glDisable(GL_DEPTH_TEST);
    //
    //// Draw the shadow
    //GLfloat vertices[] = {
    //    0.0f, 0.0f,
    //    0.0f, 1.0f,
    //    1.0f, 1.0f,
    //    1.0f, 0.0f
    //};
    //
    //glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    //glEnableClientState(GL_VERTEX_ARRAY);
    //glVertexPointer(2, GL_FLOAT, 0, vertices);
    //glDrawArrays(GL_QUADS, 0, 4);
    //glDisableClientState(GL_VERTEX_ARRAY);
    //
    //// Re-enable depth test
    //glEnable(GL_DEPTH_TEST);
    //
    //// Restore previous matrices
    //glMatrixMode(GL_PROJECTION);
    //glLoadMatrixf(glm::value_ptr(previousProjectionMatrix));
    //glMatrixMode(GL_MODELVIEW);
    //glLoadMatrixf(glm::value_ptr(previousModelviewMatrix));
}
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VS_OUT {
    mat4 viewMatrix;
    vec4 worldPosition;
    vec3 normal;
    vec2 texCoord;
    vec3 cameraForwardf;
    vec4 fragPosLightSpace;
    mat4 lightSpaceMatrix;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 position;
uniform vec3 rotation;
uniform vec3 cameraForward;
uniform mat4 lightSpaceMatrix;
uniform float uvTileFactor;

void main()
{
    gl_Position = projection * view * model * (vec4(aPos, 1.0));
    mat4 normalMatrix = transpose(inverse(model));

    vs_out.normal = vec3(normalMatrix * vec4(aNormal, 0.0));
    vs_out.viewMatrix = view;
    vs_out.worldPosition = model * vec4(aPos, 1.0);
    vs_out.texCoord = aTexCoord * uvTileFactor;
    vs_out.cameraForwardf = cameraForward;
    vs_out.fragPosLightSpace = lightSpaceMatrix * vs_out.worldPosition;
    vs_out.lightSpaceMatrix = lightSpaceMatrix;
}
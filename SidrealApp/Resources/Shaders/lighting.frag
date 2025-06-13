#version 460 core

out vec4 FragColor;

in VS_OUT {
    mat4 viewMatrix;
    vec4 worldPosition;
    vec3 normal;
    vec2 texCoord;
    vec3 cameraForwardf;
    vec4 fragPosLightSpace;
    mat4 lightSpaceMatrix;
} fs_in;

layout (std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};

uniform int cascadeCount; 
uniform float nearPlane; 
uniform float farPlane; 
uniform float cascadePlaneDistances[16];
uniform vec3 lightDirection;
uniform mat4 lightViewProjection;
uniform sampler2D colorTexture;
uniform sampler2DArray shadowMap;

vec3 ambientLight = vec3(0.3f);

float ShadowCalculation(float NdotL)
{
    // Select cascade layer
    vec4 fragPosViewSpace = fs_in.viewMatrix * fs_in.worldPosition;
    float depthValue = abs(fragPosViewSpace.z);
    
    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depthValue < cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = cascadeCount - 1;
    }

    vec4 fPosLightSpace = lightSpaceMatrices[layer] * fs_in.worldPosition;

    vec3 projCoords = fPosLightSpace.xyz / fPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, vec3(projCoords.xy, layer)).r; 
    float currentDepth = projCoords.z;

    vec3 normal = normalize(fs_in.normal);
    //float bias = max(0.003 * (1.0 - NdotL), 0.003);
    //float bias = clamp(0.0015 * tan(acos(NdotL)), 0.0005, 0.005);
    float bias = max(0.005 * (1.0 - NdotL), 0.001);
    //bias = clamp(bias, 0.0,0.01);

    int samples = 0;
    float shadow = 0.0;
    //vec2 texelSize = 1.0f / vec2(textureSize(shadowMap, layer));
    vec2 texelSize = 1.0f / vec2(textureSize(shadowMap, 0));
    int kernelRadius = 3; // 5x5 kernel
    //return texture(shadowMap, vec3(projCoords.xy, layer)).r;
    for(int x = -kernelRadius; x <= kernelRadius; ++x)
    {
        for(int y = -kernelRadius; y <= kernelRadius; ++y)
        {
            vec2 offset = projCoords.xy + vec2(x, y) * texelSize;

            // Only sample if inside shadow map bounds
            if (offset.x >= 0.0 && offset.x <= 1.0 &&
                offset.y >= 0.0 && offset.y <= 1.0)
            {
                float pcfDepth = texture(shadowMap, vec3(offset, layer)).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
                samples += 1;
            }     
        }    
    }
   
    // Avoid divide by zero if all samples are skipped
    if (samples > 0)
        shadow /= float(samples);
    else
        shadow = 0.0;

    if (projCoords.z > 1.0)
        shadow = 0.0;

    return 1.0f-shadow;
}

vec3 GetShadowColor()
{
    vec4 fragPosViewSpace = fs_in.viewMatrix * fs_in.worldPosition;
    float depthValue = abs(fragPosViewSpace.z);
    
    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depthValue < cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = cascadeCount - 1;
    }

    if (layer == 0)
    {
        return vec3(1.0, 0.0, 0.0);
    }
    else if (layer == 1)
    {
        return vec3(0.0, 1.0, 0.0);
    }
    else
    {
        return vec3(0.0, 0.0, 1.0);
    }
}

void main()
{

    vec4 viewPosition = normalize(mat4(fs_in.viewMatrix) * fs_in.worldPosition);
    vec3 viewNormal = normalize(mat3(fs_in.viewMatrix) * fs_in.normal);
    
    vec4 textureColor = texture(colorTexture, fs_in.texCoord);

    // Basic soft shading
    //float shading = dot(fs_in.normal, normalize(lightPos));
    //shading  = shading / 0.1f + 0.5f; // Sharpen shadow
    //shading = clamp(shading, ambientLight.x, 1.0f);

    // Rim lighting
    float rimlight = max(dot(-fs_in.cameraForwardf, fs_in.normal), 0.0f) * 1 + 0.5f;
    rimlight = clamp(rimlight, 0.8f, 1.0f);
    rimlight = 1.0f - rimlight;
    //shading *= rimlight;

    // Specular
    vec3 viewDir = normalize(-viewPosition.xyz);
    vec3 halfwayDir = normalize(lightDirection + viewDir);
    float spec = pow(max(dot(viewNormal, halfwayDir), 0.0f), 5.0f);

    vec3 color = textureColor.xyz;
    color += rimlight * 0.1f;
    color += spec * 0.1f;

    float NdotL = max(dot(viewNormal, lightDirection), 0.0f);
    float castShadows = ShadowCalculation(NdotL);   
    castShadows = clamp(pow(castShadows, 3.0), 0.0f, 1.0f);

    // Apply color to shadow based on cascade
    vec3 shadowColor = GetShadowColor();
    if (castShadows > 0.9f) shadowColor = vec3(1.0f, 1.0f, 1.0f);

    color = color * (max(shadowColor, ambientLight * shadowColor));

    FragColor = vec4(color, 1.0f);
} 
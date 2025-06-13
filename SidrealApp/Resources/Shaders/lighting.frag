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

// Uniforms
// TODO: Made uniform buffer objects for these
uniform int cascadeCount; 
uniform float nearPlane; 
uniform float farPlane; 
uniform float cascadePlaneDistances[16];
uniform vec3 lightDirection;
uniform mat4 lightViewProjection;
uniform sampler2D colorTexture;
uniform sampler2DArray shadowMap;

float shadowStrength = 0.5f;
vec3 ambientLight = vec3(0.5f);

float CalculateBias(vec3 normal, vec3 lightDir)
{
    float normalDotLight = max(dot(normal, lightDir), 0.0);
    
    // Depth bias: small constant offset
    float depthBias = 0.002;
    float slopeBiasFactor = 0.004;

    // Slope bias: proportional to angle between normal and light direction
    float slopeBias = slopeBiasFactor * (1.0 - normalDotLight);

    // Combined bias
    return depthBias + slopeBias;
}

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

    int kernelRadii[3] = int[](1, 2, 3); // Hard code 3 kernel radii for 3 cascades
    int kernelRadius = kernelRadii[layer];

    int samples = 0;
    float shadow = 0.0;
    float currentDepth = projCoords.z;
    //float bias = max(0.005 * (1.0 - NdotL), 0.001);
    float bias = CalculateBias(fs_in.normal, lightDirection);
    vec2 texelSize = 1.0f / vec2(textureSize(shadowMap, 0));

    for(int x = -kernelRadius; x <= kernelRadius; ++x)
    {
        for(int y = -kernelRadius; y <= kernelRadius; ++y)
        {
            vec2 offset = projCoords.xy + vec2(x, y) * texelSize;

            // Only sample if inside shadow map bounds
            if (offset.x >= 0.0 && offset.x <= 1.0 && offset.y >= 0.0 && offset.y <= 1.0)
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

    return shadow;
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
    // Save commonly used calculations
    vec4 viewPosition = normalize(mat4(fs_in.viewMatrix) * fs_in.worldPosition);
    vec3 viewNormal = normalize(mat3(fs_in.viewMatrix) * fs_in.normal);
    float NdotL = max(dot(viewNormal, lightDirection), 0.0f);
    vec3 viewDir = normalize(-viewPosition.xyz);
    vec3 halfwayDir = normalize(lightDirection + viewDir);
    
    vec4 textureColor = texture(colorTexture, fs_in.texCoord);

    // Basic soft shading
    //float shading = dot(fs_in.normal, normalize(lightPos));
    //shading  = shading / 0.1f + 0.5f; // Sharpen shadow
    //shading = clamp(shading, ambientLight.x, 1.0f);

    // ---- Rim lighting ----
    float rimlight = max(dot(-fs_in.cameraForwardf, fs_in.normal), 0.0f) * 1 + 0.5f;
    rimlight = clamp(rimlight, 0.8f, 1.0f);
    rimlight = 1.0f - rimlight;

    // ---- Specular ----
    float spec = pow(max(dot(viewNormal, halfwayDir), 0.0f), 5.0f);

    // ---- Shadow ----
    vec3 shadow = ShadowCalculation(NdotL).xxx;   
    //if (shadow.x < 0.9f) shadow = GetShadowColor();
    shadow = mix(0.0, shadow.x, shadowStrength).xxx;
    shadow = vec3(1.0f) - shadow;
    shadow = clamp(shadow, 0.0f, 1.0f);
    // Visualize cascades

    // Add color and lighting
    vec3 color = textureColor.xyz;
    color += rimlight * 0.3f;
    color += spec * 0.2f;

    color = color * (max(shadow.xxx, ambientLight));

    FragColor = vec4(color, 1.0f);
} 
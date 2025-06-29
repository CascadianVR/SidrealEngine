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

float shadowStrength = 1.0f;
vec3 ambientLight = vec3(0.5f);

vec2 PoissonOffsets[64] = {
	vec2(0.0617981, 0.07294159),
	vec2(0.6470215, 0.7474022),
	vec2(-0.5987766, -0.7512833),
	vec2(-0.693034, 0.6913887),
	vec2(0.6987045, -0.6843052),
	vec2(-0.9402866, 0.04474335),
	vec2(0.8934509, 0.07369385),
	vec2(0.1592735, -0.9686295),
	vec2(-0.05664673, 0.995282),
	vec2(-0.1203411, -0.1301079),
	vec2(0.1741608, -0.1682285),
	vec2(-0.09369049, 0.3196758),
	vec2(0.185363, 0.3213367),
	vec2(-0.1493771, -0.3147511),
	vec2(0.4452095, 0.2580113),
	vec2(-0.1080467, -0.5329178),
	vec2(0.1604507, 0.5460774),
	vec2(-0.4037193, -0.2611179),
	vec2(0.5947998, -0.2146744),
	vec2(0.3276062, 0.9244621),
	vec2(-0.6518704, -0.2503952),
	vec2(-0.3580975, 0.2806469),
	vec2(0.8587891, 0.4838005),
	vec2(-0.1596546, -0.8791054),
	vec2(-0.3096867, 0.5588146),
	vec2(-0.5128918, 0.1448544),
	vec2(0.8581337, -0.424046),
	vec2(0.1562584, -0.5610626),
	vec2(-0.7647934, 0.2709858),
	vec2(-0.3090832, 0.9020988),
	vec2(0.3935608, 0.4609676),
	vec2(0.3929337, -0.5010948),
	vec2(-0.8682281, -0.1990303),
	vec2(-0.01973724, 0.6478714),
	vec2(-0.3897587, -0.4665619),
	vec2(-0.7416366, -0.4377831),
	vec2(-0.5523247, 0.4272514),
	vec2(-0.5325066, 0.8410385),
	vec2(0.3085465, -0.7842533),
	vec2(0.8400612, -0.200119),
	vec2(0.6632416, 0.3067062),
	vec2(-0.4462856, -0.04265022),
	vec2(0.06892014, 0.812484),
	vec2(0.5149567, -0.7502338),
	vec2(0.6464897, -0.4666451),
	vec2(-0.159861, 0.1038342),
	vec2(0.6455986, 0.04419327),
	vec2(-0.7445076, 0.5035095),
	vec2(0.9430245, 0.3139912),
	vec2(0.0349884, -0.7968109),
	vec2(-0.9517487, 0.2963554),
	vec2(-0.7304786, -0.01006928),
	vec2(-0.5862702, -0.5531025),
	vec2(0.3029106, 0.09497032),
	vec2(0.09025345, -0.3503742),
	vec2(0.4356628, -0.0710125),
	vec2(0.4112572, 0.7500054),
	vec2(0.3401214, -0.3047142),
	vec2(-0.2192158, -0.6911137),
	vec2(-0.4676369, 0.6570358),
	vec2(0.6295372, 0.5629555),
	vec2(0.1253822, 0.9892166),
	vec2(-0.1154335, 0.8248222),
	vec2(-0.4230408, -0.7129914),
};

int GetCascadeLayer()
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

    return layer;
}

// required when using a perspective projection matrix
float LinearizeDepth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}


float CalculateBias(vec3 normal, vec3 lightDir, int layer)
{
    float normalDotLight = max(dot(normal, lightDir), 0.0);
    
    // Depth bias: small constant offset
    float slopeBiasFactors[3] = float[](0.004, 0.003, 0.0015);
    float slopeBiasFactor = slopeBiasFactors[layer];

    float depthBiasFactors[3] = float[](0.0002, 0.0002, 0.0001);
    float depthBias = depthBiasFactors[layer];

    // Slope bias: proportional to angle between normal and light direction
    float slopeBias = slopeBiasFactor * (1.0 - normalDotLight);

    // Combined bias
    return depthBias + slopeBias;
}

vec2 hammersley2d(int i, int N)
{
    float rdi = float(i);
    float u = rdi / float(N);
    float v = bitfieldReverse(i) * 2.3283064365386963e-10; // 1.0 / (2^32)
    return vec2(u, v);
}

float ShadowCalculation2(float NdotL)
{
    int cascadeLayer = GetCascadeLayer();

    vec4 fPosLightSpace = lightSpaceMatrices[cascadeLayer] * fs_in.worldPosition;
    vec3 projCoords = fPosLightSpace.xyz / fPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float bias = CalculateBias(fs_in.normal, lightDirection, cascadeLayer);
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float currentDepth = projCoords.z;

    // --- PCSS Step 1: Search blocker ---
    const int searchSamples = 32;
    float avgBlockerDepth = 0.0;
    int blockerCount = 0;

    float searchRadius = 40 * (1.0 - NdotL); // Tunable: world-to-shadow softness scale

    for (int i = 0; i < searchSamples; ++i)
    {
        //vec2 offset = hammersley2d(i, searchSamples) * searchRadius * texelSize;
        vec2 offset = PoissonOffsets[i] * searchRadius * texelSize;
        float sampleDepth = texture(shadowMap, vec3(projCoords.xy + offset, cascadeLayer)).r;
        if (sampleDepth < currentDepth - bias)
        {
            avgBlockerDepth += sampleDepth;
            blockerCount++;
        }
    }

    if (blockerCount == 0)
        return 0.0; // Fully lit if no blocker

    avgBlockerDepth /= float(blockerCount);

    // --- PCSS Step 2: Penumbra size ---
    float penumbra = (currentDepth - avgBlockerDepth);
    float filterRadius = penumbra * 30.0f; // Tunable softness scale

    // --- PCSS Step 3: PCF with variable filter size ---
    const int pcfSamples = 32;
    float shadow = 0.0;
    for (int i = 0; i < pcfSamples; ++i)
    {
        vec2 offset = PoissonOffsets[i] * filterRadius * texelSize;
        float sampleDepth = texture(shadowMap, vec3(projCoords.xy + offset, cascadeLayer)).x;
        if (currentDepth - bias > sampleDepth)
            shadow += 1.0;
    }

    shadow /= float(pcfSamples);
    return shadow;
}

float ShadowCalculation(float NdotL)
{
    int cascadeLayer = GetCascadeLayer();

    vec4 fPosLightSpace = lightSpaceMatrices[cascadeLayer] * fs_in.worldPosition;
    vec3 projCoords = fPosLightSpace.xyz / fPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Calculate kernel radius for leeping PCF
    float texelScale = 2; // tweak this for blur softness
    vec2 shadowMapResolution = vec2(textureSize(shadowMap, 0));
    int kernelRadius = int(texelScale * shadowMapResolution / 2048.0);
    kernelRadius = clamp(kernelRadius, 1, 10); // limit max radius

    float shadow = 0.0;
    int samples = 0;
    float currentDepth = projCoords.z;
    float bias = CalculateBias(fs_in.normal, lightDirection, cascadeLayer);
    vec2 texelSize = 1.0f / shadowMapResolution;
    for(int x = -kernelRadius; x <= kernelRadius; ++x)
    {
        for(int y = -kernelRadius; y <= kernelRadius; ++y)
        {
            vec2 offset = projCoords.xy + vec2(x, y) * texelSize;

            // Only sample if inside shadow map bounds
            if (offset.x >= 0.0 && offset.x <= 1.0 && offset.y >= 0.0 && offset.y <= 1.0)
            {
                float pcfDepth = texture(shadowMap, vec3(offset, cascadeLayer)).r;

                shadow += smoothstep(0.0, 1.0, currentDepth - bias > pcfDepth ? 1.0 : 0.0);
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
    vec3 shadow = ShadowCalculation2(NdotL).xxx;   
    //if (shadow.x < 0.9f) shadow = GetShadowColor();
    shadow = mix(0.0, shadow.x, shadowStrength).xxx;
    shadow = vec3(1.0f) - shadow;
    shadow = clamp(shadow, 0.0f, 1.0f);

    // Add color and lighting
    vec3 color = textureColor.xyz;
    color += rimlight * 0.3f;
    color += spec * 0.2f;

    color = color * (max(shadow.xxx, ambientLight));

    FragColor = vec4(color, 1.0f);
    //FragColor = vec4(fs_in.normal  * 0.5 + 0.5, 1.0f);
} 
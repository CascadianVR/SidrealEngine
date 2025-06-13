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

uniform sampler2D colorTexture;
uniform sampler2D shadowMap;
uniform vec3 lightDirection;
uniform mat4 lightViewProjection;

vec3 ambientLight = vec3(0.6f);


float ShadowCalculation(vec4 fPosLightSpace, float NdotL)
{
    // perform perspective divide
    vec3 projCoords = fPosLightSpace.xyz / fPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)

    float closestDepth = texture(shadowMap, projCoords.xy).r; 

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.normal);
    float bias = max(0.005 * (1.0 - NdotL), 0.0005); // cosTheta is dot( n,l ), clamped between 0 and 1
    bias = clamp(bias, 0.0,0.01);

    float shadow = 0.0;
    vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 1.0;
        
    return 1.0f-shadow;
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
    color += rimlight;
    color += spec * 0.1f;

    float NdotL = max(dot(viewNormal, lightDirection), 0.0f);
    float castShadows = ShadowCalculation(fs_in.fragPosLightSpace, NdotL);                      
    color = color * (max(castShadows.xxx, ambientLight));

    FragColor = vec4(color, 1.0f);
} 
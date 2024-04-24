#version 460 core

out vec4 FragColor;

in VS_OUT {
    mat4 viewMatrix;
    vec4 worldPosition;
    vec3 normal;
    vec2 texCoord;
    vec3 cameraForwardf;
    vec4 fragPosLightSpace;
} fs_in;

uniform sampler2D colorTexture;
uniform sampler2D shadowMap;
uniform vec3 lightPos;

vec3 ambientLight = vec3(0.6f);

float ShadowCalculation(vec4 fPosLightSpace)
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
    vec3 lightDir = normalize(lightPos - fs_in.worldPosition.xyz);
    float bias = 0.005f;
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1f / textureSize(shadowMap, 0);
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
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return 1.0f-shadow;
}

void main()
{
    vec3 test = normalize(lightPos - fs_in.worldPosition.xyz);

    vec4 viewPosition = normalize(mat4(fs_in.viewMatrix) * fs_in.worldPosition);
    vec3 viewNormal = normalize(mat3(fs_in.viewMatrix) * fs_in.normal);
    
    vec4 textureColor = texture(colorTexture, fs_in.texCoord);

    // Basic soft shading
    float shading = dot(fs_in.normal, normalize(lightPos));
    shading  = shading / 0.1f + 0.5f; // Sharpen shadow
    shading = clamp(shading, ambientLight.x, 1.0f);

    // Rim lighting
    float rimlight = max(dot(-fs_in.cameraForwardf, fs_in.normal), 0.0f) * 1 + 0.5f;
    rimlight = clamp(rimlight, 0.8f, 1.0f);
    shading *= rimlight;

    // Specular
    vec3 viewDir = normalize(-viewPosition.xyz);
    vec3 lightDir = normalize(lightPos - viewPosition.xyz);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(viewNormal, halfwayDir), 0.0f), 15.0f);

    //vec3 color = shading.xxx * textureColor.xyz + (spec*0.05f);
    vec3 color = textureColor.xyz;

    float castShadows = ShadowCalculation(fs_in.fragPosLightSpace);                      

    color = color * (max(castShadows.xxx, ambientLight));

    FragColor = vec4(color, 1.0f);
} 
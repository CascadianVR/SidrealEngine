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
    float bias = 0.001f;
    // check whether current frag pos is in shadow
    //float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
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
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return 1.0f-shadow;
}

const float offset = 1.0 / 300.0;  
vec3 BlurShadow()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

    float kernel[9] = float[](
        (1.0f / 16.0f), (2.0f / 16.0f), (1.0f / 16.0f),  
        (1.0f / 16.0f), (2.0f / 16.0f), (1.0f / 16.0f),
        (2.0f / 16.0f), (4.0f / 16.0f), (2.0f / 16.0f)
    );
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(shadowMap, fs_in.texCoord.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    return col;
}

void main()
{
    vec3 test = normalize(lightPos - fs_in.worldPosition.xyz);

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
    vec3 lightDir = normalize(lightPos - viewPosition.xyz);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(viewNormal, halfwayDir), 0.0f), 15.0f);

    vec3 color = textureColor.xyz;
    color += rimlight;
    color += spec * 0.1f;

    float castShadows = ShadowCalculation(fs_in.fragPosLightSpace);                      
    color = color * (max(castShadows.xxx, ambientLight));

    FragColor = vec4(color, 1.0f);
} 
#version 460 core
uniform sampler2D tex;
uniform vec3 lightPos;

out vec4 FragColor;

in mat4 viewMatrix;
in vec4 worldPosition;
in vec3 normal;
in vec2 texCoord;
in vec3 cameraForwardf;

vec3 ambientLight = vec3(0.05f, 0.05f, 0.05f);

void main()
{

    vec4 viewPosition = normalize(mat4(viewMatrix) * worldPosition);
    vec3 viewNormal = normalize(mat3(viewMatrix) * normal);

    vec4 textureColor = texture(tex, texCoord);

    // Basic soft shading
    float shading = dot(normal, normalize(lightPos));
    shading  = 0.3f * shading + 0.3f;
    //shading = clamp(shading, ambientLight.x, 1.0f);

    // Rim lighting
    float rimlight = max(dot(-cameraForwardf, normal), 0.0f);
    shading = clamp (shading * rimlight + 0.5f, 0.0f, 1.0f);

    // Specular
    vec3 viewDir = normalize(-viewPosition.xyz);
    vec3 lightDir = normalize(lightPos - viewPosition.xyz);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(viewNormal, halfwayDir), 0.0f), 15.0f);
    vec3 specular = vec3(0.1f) * spec;

    vec3 color = shading.xxx * textureColor.xyz + specular;

    FragColor = vec4(color, 1.0f);
} 
#version 460 core
uniform sampler2D tex;

out vec4 FragColor;

in mat4 viewMatrix;
in vec4 positon;
in vec3 normal;
in vec2 texCoord;
in vec3 cameraForwardf;

void main()
{


    vec4 viewPosition = normalize(mat4(viewMatrix) * positon);
    vec3 viewNormal = normalize(mat3(viewMatrix) * normal);

    vec4 tex = texture(tex, texCoord);
    float shading = dot(normal, normalize(vec3(0.0f, 1.0f, 1.0f)));
    shading = shading * 0.5f + 0.5f;

    float rimlight = max(dot(-cameraForwardf, normal), 0.0f);

    shading *= rimlight;

    vec3 color = tex.xyz * shading;

    //FragColor = vec4(tex.x, tex.y, tex.z, 1.0f) * shading + rimlight;
    FragColor = vec4(color, 1.0f);
} 
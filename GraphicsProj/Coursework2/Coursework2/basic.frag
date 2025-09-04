#version 450 core

in vec3 vertexColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 texCoord;
in vec4  shadowCoord;

out vec4 FragColor;

struct Light {
    vec3 direction;
    vec3 color;
};

uniform Light light;
uniform vec3 cameraPos;
uniform sampler2D tex;
uniform sampler2D shadowMap;
uniform float uAlpha;
uniform bool useTexture;
uniform bool useOverrideColor;
uniform vec3 overrideColor;

float ShadowCalculation(vec4 fragPosLightSpace)
{

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0) 
        return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = max(0.005 * (1.0 - dot(normalize(Normal), normalize(-light.direction))), 0.0005);

    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float diff = max(dot(norm, lightDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 ambient  = 0.1 * light.color;
    vec3 diffuse  = diff * light.color;
    vec3 specular = spec * light.color;

    float shadow = ShadowCalculation(shadowCoord);

    vec3 baseColor;
    if (useOverrideColor) {
        baseColor = overrideColor;
    } else if (useTexture) {
        baseColor = texture(tex, texCoord).rgb;
    } else {
        baseColor = vertexColor;
    }

    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);
    FragColor = vec4(lighting * baseColor, uAlpha);

}
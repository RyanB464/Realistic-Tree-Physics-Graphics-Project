#version 410 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform sampler2D barkTex;
uniform sampler2D barkNormal;

void main()
{
    vec3 N = normalize(Normal);

    // Sample normal map
    vec3 nMap = texture(barkNormal, FragPos.xz * 0.15).rgb;
    nMap = normalize(nMap * 2.0 - 1.0);

    // Blend normals
    N = normalize(mix(N, nMap, 0.65));

    vec3 L = normalize(-lightDir);
    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 32.0);

    vec3 albedo = texture(barkTex, FragPos.xz * 0.15).rgb;

    vec3 color =
        albedo * (0.2 + diff * 0.8) +
        vec3(0.1) * spec;

    FragColor = vec4(color, 1.0);
}

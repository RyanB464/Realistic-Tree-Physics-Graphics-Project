#version 410 core

in vec3 FragPos;
in vec3 Normal;
in vec3 LocalPos;

out vec4 FragColor;

uniform sampler2D barkTex;
uniform sampler2D barkNormal;

uniform vec3 lightDir;

void main()
{
    // Bark UVs
    vec2 uv = vec2(LocalPos.x * 0.5, LocalPos.y * 2.0);

    vec3 albedo = texture(barkTex, uv).rgb;

    vec3 nmap = texture(barkNormal, uv).rgb;
    nmap = normalize(nmap * 2.0 - 1.0);

    vec3 N = normalize(Normal + nmap * 0.3);
    vec3 L = normalize(-lightDir);

    float diff = max(dot(N, L), 0.0);

    vec3 color = albedo * (0.2 + diff * 0.8);

    FragColor = vec4(color, 1.0);
}

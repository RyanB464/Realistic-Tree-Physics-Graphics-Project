#version 410 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 viewPos;

void main()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(-lightDir);

    float diff = max(dot(N, L), 0.0);
    float trans = max(dot(-N, L), 0.0);

    float noise = fract(sin(dot(FragPos.xy, vec2(12.9898,78.233))) * 43758.5453);
    vec3 base = mix(vec3(0.18, 0.55, 0.22), vec3(0.25, 0.70, 0.28), noise);

    vec3 color =
        base * (0.15 + diff * 0.85) +
        base * trans * 0.6;

    FragColor = vec4(color, 1.0);
}

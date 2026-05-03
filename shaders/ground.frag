#version 410 core

in vec3 FragPos;
out vec4 FragColor;

uniform vec3 groundColor;

void main()
{
    // Simple slightly varied ground color
    float n = fract(sin(dot(FragPos.xz, vec2(12.9898,78.233))) * 43758.5453);
    vec3 base = mix(vec3(0.18, 0.35, 0.18), vec3(0.22, 0.45, 0.22), n * 0.3);
    FragColor = vec4(base, 1.0);
}

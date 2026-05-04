#version 410 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D leafTex;
uniform vec3 lightDir;

void main()
{
    vec4 tex = texture(leafTex, TexCoord);
    if (tex.a < 0.2)
        discard;

    vec3 N = normalize(Normal);
    vec3 L = normalize(-lightDir);

    float diff = max(dot(N, L), 0.0);
    float trans = max(dot(-N, L), 0.0);

    vec3 color = tex.rgb * (0.2 + diff * 0.8 + trans * 0.4);

    FragColor = vec4(color, tex.a);
}

#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in mat4 instanceModel;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int useInstancing;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
    mat4 M = (useInstancing == 1) ? instanceModel : model;

    vec4 worldPos = M * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;

    Normal = normalize(mat3(M) * vec3(0, 0, 1));
    TexCoord = aTexCoord;

    gl_Position = projection * view * worldPos;
}

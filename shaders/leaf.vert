#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in mat4 instanceModel;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int  useInstancing;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    mat4 M = (useInstancing == 1) ? instanceModel : model;

    vec4 worldPos = M * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);

    Normal = normalize(mat3(M) * vec3(0, 0, 1));

    gl_Position = projection * view * worldPos;
}

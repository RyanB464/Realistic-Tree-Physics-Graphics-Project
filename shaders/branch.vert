#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

// Instance matrix (4 vec4s)
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

    Normal = normalize(mat3(transpose(inverse(M))) * aNormal);

    gl_Position = projection * view * worldPos;
}

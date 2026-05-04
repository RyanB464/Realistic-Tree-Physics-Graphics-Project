#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

// Instance matrix (4 vec4s)
layout (location = 2) in mat4 instanceModel;

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec3 LocalPos;

void main()
{
    // LocalPos used for bark UVs
    LocalPos = aPos;

    // World position
    vec4 worldPos = instanceModel * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;

    // Normal transform
    Normal = normalize(mat3(instanceModel) * aNormal);

    gl_Position = projection * view * worldPos;
}

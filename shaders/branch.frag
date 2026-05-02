#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 baseColor; // material color (ground vs trunk)

void main() {
    vec3 ambient = 0.2 * lightColor;

    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, -lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    vec3 specular = 0.3 * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * baseColor;
    FragColor = vec4(result, 1.0);
}

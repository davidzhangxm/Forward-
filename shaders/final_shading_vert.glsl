#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VERTEX_OUT {
    vec3 worldPosition;
    vec2 texCoords;
    mat3 TBN;
    vec3 tangentViewPosition;
    vec3 tangentWorldPosition;
} vertex_out;

// Uniforms
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 viewPosition;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    vertex_out.worldPosition = vec3(model * vec4(position, 1.0));
    vertex_out.texCoords = texCoords;

    mat3 normalTrans = transpose(inverse(mat3(model)));
    vec3 tan = normalize(normalTrans * tangent);
    vec3 bitan = normalize(normalTrans * bitangent);
    vec3 norm = normalize(normalTrans * normal);

    // normal mapping
    mat3 TBN = transpose(mat3(tan, bitan, norm));
    vertex_out.tangentViewPosition = TBN * viewPosition;
    vertex_out.tangentWorldPosition = TBN * vertex_out.worldPosition;
    vertex_out.TBN = TBN;
}

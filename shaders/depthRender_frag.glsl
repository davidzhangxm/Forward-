#version 330 core

uniform float near;
uniform float far;
uniform sampler2D depthMap;

in vec2 TexCoords;

out vec4 fragColor;

// Linearize depth
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    float depthValue = texture(depthMap, TexCoords).r;
    float depth = LinearizeDepth(depthValue) / far;
    fragColor = vec4(vec3(depth), 1.0f);
}

#version 430

in VERTEX_OUT {
    vec3 worldPosition;
    vec2 texCoords;
    mat3 TBN;
    vec3 tangentViewPosition;
    vec3 tangentWorldPosition;
} fragment_in;

struct PointLight {
    vec4 color;
    vec4 position;
    vec4 paddingAndRadius;
};

struct VisibleIndex{
    int index;
};

layout(std430, binding = 0) readonly buffer LightBuffer {
    PointLight data[];
} lightBuffer;

layout(std430, binding = 1) readonly buffer VisibleLightIndicesBuffer {
    VisibleIndex data[];
} visibleLightIndicesBuffer;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform int numberOfTilesX;

out vec4 fragColor; 

float attenuate(vec3 lightDirection, float radius)
{
    float cutoff = 0.5;
    float attenuation = dot(lightDirection, lightDirection) / (100.0 * radius);
    attenuation = 1.0 / (attenuation * 15.0 + 1.0);
    attenuation = (attenuation - cutoff) / (1.0 - cutoff);

    return clamp(attenuation, 0.0, 1.0);
}

void main()
{
    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint index = tileID.y * numberOfTilesX + tileID.x;

    // extract texture values
    vec4 base_diffuse = texture(texture_diffuse1, fragment_in.texCoords);
    vec4 base_specular = texture(texture_specular1, fragment_in.texCoords);
    vec3 normal = texture(texture_normal1, fragment_in.texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

    vec3 viewDirection = normalize(fragment_in.tangentViewPosition - fragment_in.tangentWorldPosition);
    // traverse all visible light in this tile
    uint offset = index * 1024;
    for(uint i = 0; i < 1024 && visibleLightIndicesBuffer.data[offset + i].index != -1; ++i)
    {
        uint lightIndex = visibleLightIndicesBuffer.data[offset + i].index;
        PointLight light = lightBuffer.data[lightIndex];

        vec4 lightColor = light.color;
        //lightColor = vec4(1.0, 1.0, 1.0, 1.0);
        vec3 tangentLightPosition = fragment_in.TBN * light.position.xyz;
        float lightRadius = light.paddingAndRadius.w;

		// Calculate the light attenuation on the pre-normalized lightDirection
		vec3 lightDirection = tangentLightPosition - fragment_in.tangentWorldPosition;
		float attenuation = attenuate(lightDirection, lightRadius);

		// Normalize the light direction and calculate the halfway vector
		lightDirection = normalize(lightDirection);
		vec3 halfway = normalize(lightDirection + viewDirection);

        // shading model
        float diffuse = dot(lightDirection, normal);
        if(diffuse > 0.0)
        {
            vec3 irradiance = base_diffuse.rgb * diffuse;
            float specular = dot(normal, halfway);
            if(specular > 0.0)
            {
                specular = pow(specular, 32.0);
                irradiance += base_specular.rgb * specular;
            }
            irradiance *= lightColor.rgb * attenuation;
            color.rgb += irradiance;
        }
    }
    // environment light
    color.rgb += base_diffuse.rgb * 0.08;

    // remove transparent fragment
    if(base_diffuse.a <= 0.2)
    {
        discard;
    }

    fragColor = color;
        


        
    
}

#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

float circleShape(float radius,vec2 position)
{
    float value = distance(position,vec2(0.5));
    return step(radius,value);
}

void main() {
    vec4 color = vec4(1, 1, 1, 1);
    color = texture(texSampler, fragTexCoord);

    color *= vec4(fragColor.rgb,1);
    float cicleWidth = 0.3;
    float value = circleShape(cicleWidth,fragTexCoord);
    if (value != 0)
        discard;
    outColor = color;
}
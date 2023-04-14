#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;
void main() {
    vec4 color = vec4(1, 1, 1, 1);
    color = texture(texSampler, fragTexCoord);

    color *= vec4(fragColor.rgb,1);
    color.a = step(length(fragTexCoord - vec2(0.5,0.5)), 0.1);
    outColor = color;
}
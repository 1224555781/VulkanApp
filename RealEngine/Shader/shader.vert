#version 450

layout (binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location =0) in vec2 inposition;
layout(location =1) in vec3 incolor;

layout(location = 0) out vec3 fragColor;


void main() {
    gl_Position =ubo.proj * ubo.view* ubo.model * vec4(inposition, 0.0, 1.0);
    fragColor = incolor;
}

#version 450

// Push constants
layout(push_constant) uniform PushConstants {
    mat4 model;
} push;

layout(binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
    float near;
    float far;
} cam;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = cam.proj * cam.view * push.model * vec4(inPosition, 1.0);
    fragColor = inColor;
}
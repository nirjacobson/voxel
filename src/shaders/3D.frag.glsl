#version 450

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

layout(push_constant) uniform fPushConstants {
    layout(offset = 80)  vec3 color;
    layout(offset = 96)  vec3 sun_position;
    layout(offset = 112) float ambient;
} pushConstants;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 sun_direction = normalize(pushConstants.sun_position - Position);
    float diff = max(dot(Normal, sun_direction), 0.0);

    fragColor = vec4(pushConstants.color*(pushConstants.ambient+diff), 1.0);
}

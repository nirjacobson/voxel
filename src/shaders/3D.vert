#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(binding = 0) uniform UniformBufferObject {
    mat4 camera;
    mat4 projection;
} mcp;

layout(push_constant) uniform vPushConstants {
    layout(offset = 0)  mat4 model;
    layout(offset = 16) vec3 worldPosition;
} pushConstants;

layout(location = 0) out vec3 Position;
layout(location = 1) out vec3 Normal;

void main()
{
    mat4 view = mat4(1.0);
    view[3] = vec4(pushConstants.worldPosition, 1.0);
    vec4 worldPosition = view * mcp.model * vec4(position, 1.0);

    vec4 rotatedNormal = vec4(mat3(mcp.model) * normal, 1.0);

    vec4 screenPosition = mcp.projection * mcp.camera * worldPosition;

    gl_Position = screenPosition;
    Position = worldPosition.xyz;
    Normal = rotatedNormal.xyz;
}

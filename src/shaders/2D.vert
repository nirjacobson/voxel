#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;

layout(push_constant) uniform vPushConstants {
    layout(offset = 0) mat4 projection;
} pushConstants;

layout(location = 0) out vec2 TexCoord;

void main() {
  vec4 screenPosition = pushConstants.projection * vec4(position, 1.0);

  gl_Position = screenPosition;
  TexCoord = texcoord;
}

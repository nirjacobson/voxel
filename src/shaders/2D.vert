#version 300 es

in vec3 position;
in vec2 texcoord;

uniform mat4 projection;

out vec2 TexCoord;

void main() {
  vec4 screenPosition = projection * vec4(position, 1.0);

  gl_Position = screenPosition;
  TexCoord = texcoord;
}
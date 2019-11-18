#version 100

attribute vec3 position;
attribute vec2 texcoord;

uniform mat4 projection;

varying vec2 TexCoord;

void main() {
  vec4 screenPosition = projection * vec4(position, 1.0);

  gl_Position = screenPosition;
  TexCoord = texcoord;
}
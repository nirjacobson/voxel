#version 300 es

precision highp float;

in vec2 TexCoord;

uniform sampler2D sampler;

out vec4 fragColor;

void main() {
  fragColor = texture2D(sampler, TexCoord).bgra;
}
#version 300 es

precision highp float;

in vec3 Position;
in vec3 Normal;

uniform vec3 color;
uniform vec3 sun_position;
uniform float ambient;

out vec4 fragColor;

void main()
{
    vec3 sun_direction = normalize(sun_position - Position);
    float diff = max(dot(Normal, sun_direction), 0.0);

    fragColor = vec4(color*(ambient+diff), 1.0);
}

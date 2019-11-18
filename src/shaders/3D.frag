#version 100

varying vec3 Position;
varying vec3 Normal;

uniform vec3 color;
uniform vec3 sun_position;
uniform float ambient;

void main()
{
    vec3 sun_direction = normalize(sun_position - Position);
    float diff = max(dot(Normal, sun_direction), 0.0);
    
    gl_FragColor = vec4(color*(ambient+diff), 1.0);
}
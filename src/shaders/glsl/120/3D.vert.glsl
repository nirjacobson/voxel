#version 120

attribute vec3 position;
attribute vec3 normal;

uniform vec3 worldPosition;

uniform mat4 model;
uniform mat4 camera;
uniform mat4 projection;

varying vec3 Position;
varying vec3 Normal;

void main()
{
    mat4 view = mat4(1.0);
    view[3] = vec4(worldPosition, 1.0);
    vec4 worldPosition = view * model * vec4(position, 1.0);

    vec4 rotatedNormal = vec4(mat3(model) * normal, 1.0);

    vec4 screenPosition = projection * camera * worldPosition;

    gl_Position = screenPosition;
    Position = worldPosition.xyz;
    Normal = rotatedNormal.xyz;
}
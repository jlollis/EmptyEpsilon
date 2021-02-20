#version 120

// Program inputs
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// Per-vertex inputs
attribute vec3 position;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
}

#version 120
//Simple per-pixel light shader.

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = projection * view * model * gl_Vertex;
}

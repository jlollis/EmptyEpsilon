#version 120
//Simple per-pixel light shader.

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 model_normal;

varying vec3 normal;
varying vec3 position;

void main()
{
	normal = normalize(model_normal * gl_Normal);
	vec4 pos = view * model * gl_Vertex;
	position = vec3(pos);
	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = projection * pos;
}

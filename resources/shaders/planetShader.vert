//Simple per-pixel light shader.

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

varying vec3 normal;
varying vec3 position;

void main()
{
	normal = normalize(gl_NormalMatrix * gl_Normal);
	position = vec3(view * model * gl_Vertex);
	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = projection * view * model * gl_Vertex;
	gl_FrontColor = gl_Color;
	gl_BackColor = gl_Color;
}

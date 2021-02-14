#version 120
//Simple per-pixel light shader.

uniform sampler2D textureMap;
uniform vec4 color;

void main()
{
	gl_FragColor = texture2D(textureMap, gl_TexCoord[0].st) * color;
	gl_FragColor.rgb *= color.a;
}

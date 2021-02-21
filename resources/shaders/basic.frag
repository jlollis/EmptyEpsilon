#version 100
precision mediump float;
uniform vec4 color;
uniform sampler2D textureMap;

varying vec2 fragtexcoords;

void main()
{
    gl_FragColor = texture2D(textureMap, fragtexcoords.st) * color;
    gl_FragColor.rgb *= color.a;
}

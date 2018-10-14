#version 330 core

out vec4 colorOut;

in vec2 texCoord;

uniform sampler2D sample;

void main()
{
    colorOut = texture( sample, texCoord );
}

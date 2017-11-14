#version 330

uniform sampler2D slide;

in vec2 texCoord;

out vec4 outputColor;

void main()
{
    outputColor = texture(slide, texCoord);
}

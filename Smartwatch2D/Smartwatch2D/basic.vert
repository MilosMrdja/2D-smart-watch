#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;

uniform float uX;
uniform float uY;
uniform float uScale;

out vec2 TexCoord;

void main()
{
    vec2 scaled = aPos * uScale;
    vec2 moved = scaled + vec2(uX, uY);
    gl_Position = vec4(moved, 0.0, 1.0);

    TexCoord = aTex;
}

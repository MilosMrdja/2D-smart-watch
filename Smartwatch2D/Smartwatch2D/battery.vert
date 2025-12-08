#version 330 core
layout(location = 0) in vec2 aPos;

uniform float uX;
uniform float uY;
uniform float uScaleX;
uniform float uScaleY;

void main()
{
    vec2 scaled = vec2(aPos.x * uScaleX, aPos.y * uScaleY);
    gl_Position = vec4(scaled + vec2(uX, uY), 0.0, 1.0);
}

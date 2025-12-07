#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uOffset;
uniform float uScaleX;

void main()
{
    vec2 tc = TexCoord;
    tc.x = tc.x * uScaleX + uOffset; // pomeranje samo EKG
    FragColor = texture(uTexture, tc);
}

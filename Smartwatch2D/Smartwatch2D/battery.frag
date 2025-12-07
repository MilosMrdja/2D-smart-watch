#version 330 core
out vec4 FragColor;

uniform float uFillPercent;  // [0.0 - 1.0]
uniform vec3 uColor;         // boja punjenja

in vec2 TexCoord;

void main()
{
    float x = TexCoord.x;

    // crtamo pravougaonik baterije od desne strane
    if (x >= 1.0 - uFillPercent) {
        FragColor = vec4(uColor, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 0.0); // prazno ili prozirno
    }
}

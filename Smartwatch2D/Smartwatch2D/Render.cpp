#include "Render.h"


void drawQuad(unsigned int& VAO, unsigned int shader, unsigned int texture, float x, float y, float scale)
{
    glUseProgram(shader);
    glUniform1f(glGetUniformLocation(shader, "uX"), x);
    glUniform1f(glGetUniformLocation(shader, "uY"), y);
    glUniform1f(glGetUniformLocation(shader, "uScale"), scale);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


void drawTime(unsigned int VAO, unsigned int shader, float scale,
    unsigned int digits[10], unsigned int colonTex, int hours, int minutes, int seconds)
{

    int h1 = hours / 10, h2 = hours % 10;
    int m1 = minutes / 10, m2 = minutes % 10;
    int s1 = seconds / 10, s2 = seconds % 10;

    int numElements = 8; // HH:MM:SS -> 8 elem
    float spacing = scale * 0.7f;
    float totalWidth = numElements * spacing;
    float startX = -totalWidth / 2.0f;
    float y = 0.0f;

    drawQuad(VAO, shader, digits[h1], startX, y, scale); startX += spacing;
    drawQuad(VAO, shader, digits[h2], startX, y, scale); startX += spacing;

    drawQuad(VAO, shader, colonTex, startX, y, scale); startX += spacing;

    drawQuad(VAO, shader, digits[m1], startX, y, scale); startX += spacing;
    drawQuad(VAO, shader, digits[m2], startX, y, scale); startX += spacing;

    drawQuad(VAO, shader, colonTex, startX, y, scale); startX += spacing;

    drawQuad(VAO, shader, digits[s1], startX, y, scale); startX += spacing;
    drawQuad(VAO, shader, digits[s2], startX, y, scale);
}


void drawBatteryProgress(unsigned int VAO, unsigned int shader,
    float centerX, float centerY,
    float width, float height,
    float percent)
{
    float r = 0.0f, g = 1.0f, b = 0.0f;
    if (percent <= 10.0f) { r = 1.0f; g = 0.0f; b = 0.0f; }
    else if (percent <= 20.0f) { r = 1.0f; g = 1.0f; b = 0.0f; }

    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "uColor"), r, g, b);

    float innerWidth = width * (percent / 100.0f);

    float xPos = centerX + (width - innerWidth) / 2.0f;

    glUniform1f(glGetUniformLocation(shader, "uX"), xPos);
    glUniform1f(glGetUniformLocation(shader, "uY"), centerY);
    glUniform1f(glGetUniformLocation(shader, "uScaleX"), innerWidth);
    glUniform1f(glGetUniformLocation(shader, "uScaleY"), height);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawBatteryFrame(unsigned int VAO, unsigned int shader,
    float centerX, float centerY,
    float width, float height,
    float border)
{
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "uColor"), 0.0f, 0.0f, 0.0f);

    glUniform1f(glGetUniformLocation(shader, "uX"), centerX);
    glUniform1f(glGetUniformLocation(shader, "uY"), centerY);
    glUniform1f(glGetUniformLocation(shader, "uScaleX"), width);
    glUniform1f(glGetUniformLocation(shader, "uScaleY"), height);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
}



void drawQuadEKG(unsigned int VAO, unsigned int shader, unsigned int texture,
    float x, float y, float scale, float offset, float scaleX)
{
    glUseProgram(shader);
    glUniform1f(glGetUniformLocation(shader, "uX"), x);
    glUniform1f(glGetUniformLocation(shader, "uY"), y);
    glUniform1f(glGetUniformLocation(shader, "uScale"), scale);
    glUniform1f(glGetUniformLocation(shader, "uOffset"), offset);
    glUniform1f(glGetUniformLocation(shader, "uScaleX"), scaleX);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawNumber(unsigned int VAO, unsigned int shader, int number,
    float x, float y, float scale, unsigned int digits[10])
{
    int n = number;
    std::vector<int> digitsVec;

    if (n == 0) digitsVec.push_back(0);
    else {
        while (n > 0) {
            digitsVec.push_back(n % 10);
            n /= 10;
        }
        std::reverse(digitsVec.begin(), digitsVec.end());
    }

    float spacing = scale * 1.0f;
    float totalWidth = digitsVec.size() * spacing;
    float startX = x - totalWidth / 2.0f;

    for (int d : digitsVec) {
        drawQuad(VAO, shader, digits[d], startX, y, scale);
        startX += spacing;
    }
}
void drawEKGScreen(
    GLFWwindow* window,
    unsigned int VAO,
    unsigned int shader,
    unsigned int ekgShader,
    unsigned int ekgTexture,
    unsigned int warningTexture,
    unsigned int digits[10],
    float dt,
    float& ekgOffset,
    float& ekgScaleX,
    int& bpm
) {
    ekgOffset += 0.08f * dt;
    if (ekgOffset >= 1.0f) ekgOffset -= 1.0f;



    if (bpm >= 200) {
        drawQuad(VAO, shader, warningTexture, 0.0f, 0.0f, 0.8f);
    }
    else {
        drawQuadEKG(VAO, ekgShader, ekgTexture, 0.0f, -0.1f, 0.8f, ekgOffset, ekgScaleX);
    }

    drawNumber(VAO, shader, bpm, 0.0f, 0.6f, 0.1f, digits);
    drawQuad(VAO, shader, bpmTexture, 0.15f, 0.6f, 0.15f);

}
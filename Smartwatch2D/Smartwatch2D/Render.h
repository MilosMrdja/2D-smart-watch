#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <ctime>
#include <algorithm>
#include "Textures.h"

void drawQuad(unsigned int& VAO,
    unsigned int shader,
    unsigned int texture,
    float x, float y, float scale);

void drawTime(unsigned int VAO,
    unsigned int shader,
    float scale,
    unsigned int digits[10],
    unsigned int colonTex, int hours, int minutes, int seconds);

void drawBatteryProgress(unsigned int VAO,
    unsigned int shader,
    float centerX, float centerY,
    float width, float height,
    float percent);

void drawBatteryFrame(unsigned int VAO,
    unsigned int shader,
    float centerX, float centerY,
    float width, float height,
    float border);

void drawQuadEKG(unsigned int VAO,
    unsigned int shader,
    unsigned int texture,
    float x, float y,
    float scale,
    float offset,
    float scaleX);

void drawNumber(unsigned int VAO,
    unsigned int shader,
    int number,
    float x, float y,
    float scale,
    unsigned int digits[10]);

void drawEKGScreen(GLFWwindow* window,
    unsigned int VAO,
    unsigned int shader,
    unsigned int ekgShader,
    unsigned int ekgTexture,
    unsigned int warningTexture,
    unsigned int digits[10],
    float dt,
    float& ekgOffset,
    float& ekgScaleX,
    int& bpm);
#pragma once
#include <GLFW/glfw3.h>

extern GLFWcursor* cursor;
extern GLFWcursor* cursorPressed;

enum ScreenState {
    SCREEN_CLOCK = 0,
    SCREEN_HEART = 1,
    SCREEN_BATTERY = 2
};

extern ScreenState currentScreen;

extern bool mousePressedLastFrame;

void key_callback(GLFWwindow* window,
    int key, int scancode,
    int action, int mods);

void mouse_callback(GLFWwindow* window,
    int button, int action,
    int mods);

bool isClickOn(double mx, double my,
    float x, float y, float size);

void handleArrowClicks(ScreenState& currentScreen,
    bool drawLeft, bool drawRight,
    float arrowXOffset, float arrowY, float arrowScale,
    GLFWwindow* window,
    bool& mousePressedLastFrame);
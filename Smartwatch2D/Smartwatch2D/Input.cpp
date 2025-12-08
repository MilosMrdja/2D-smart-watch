#include "Input.h"

GLFWcursor* cursor = nullptr;
GLFWcursor* cursorPressed = nullptr;

ScreenState currentScreen = SCREEN_CLOCK;
bool mousePressedLastFrame = false;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}


void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glfwSetCursor(window, cursorPressed);
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        glfwSetCursor(window, cursor);
    }
}


bool isClickOn(double mx, double my, float x, float y, float size) {
    return (mx > x - size && mx < x + size &&
        my > y - size && my < y + size);
}


void handleArrowClicks(ScreenState& currentScreen,
    bool drawLeft, bool drawRight,
    float arrowXOffset, float arrowY, float arrowScale,
    GLFWwindow* window,
    bool& mousePressedLastFrame)
{
    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (mousePressed && !mousePressedLastFrame) {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        int w, h;
        glfwGetWindowSize(window, &w, &h);

        float x = (mx / w) * 2.0f - 1.0f;
        float y = 1.0f - (my / h) * 2.0f;

        // LEFT arrow
        if (drawLeft && isClickOn(x, y, -arrowXOffset, arrowY, arrowScale)) {
            if (currentScreen == SCREEN_HEART) currentScreen = SCREEN_CLOCK;
            else if (currentScreen == SCREEN_BATTERY) currentScreen = SCREEN_HEART;
        }

        // RIGHT arrow
        if (drawRight && isClickOn(x, y, arrowXOffset, arrowY, arrowScale)) {
            if (currentScreen == SCREEN_CLOCK) currentScreen = SCREEN_HEART;
            else if (currentScreen == SCREEN_HEART) currentScreen = SCREEN_BATTERY;
        }
    }

    mousePressedLastFrame = mousePressed;
}
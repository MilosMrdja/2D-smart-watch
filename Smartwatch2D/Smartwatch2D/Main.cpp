#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ctime>
#include "Util.h"
#include <vector>
#include <thread>    
#include <chrono>    



GLFWcursor* cursor;
GLFWcursor* cursorPressed;

enum ScreenState {
    SCREEN_CLOCK = 0,
    SCREEN_HEART = 1,
    SCREEN_BATTERY = 2
};

ScreenState currentScreen = SCREEN_CLOCK;
bool mousePressedLastFrame = false;


unsigned int digitTextures[10];
unsigned int colonTexture;
unsigned int arrowLeft;
unsigned int arrowRight;
unsigned int screenState[3];
unsigned int warningTexture;
unsigned int ekgTexture;
unsigned int studentTexture;
unsigned int percentageTexture;
unsigned int bpmTexture;



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

        if (drawLeft && isClickOn(x, y, -arrowXOffset, arrowY, arrowScale)) {
            if (currentScreen == SCREEN_HEART) currentScreen = SCREEN_CLOCK;
            else if (currentScreen == SCREEN_BATTERY) currentScreen = SCREEN_HEART;
        }

        if (drawRight && isClickOn(x, y, arrowXOffset, arrowY, arrowScale)) {
            if (currentScreen == SCREEN_CLOCK) currentScreen = SCREEN_HEART;
            else if (currentScreen == SCREEN_HEART) currentScreen = SCREEN_BATTERY;
        }
    }

    mousePressedLastFrame = mousePressed;
}




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
    unsigned int digits[10], unsigned int colonTex)
{
    std::time_t t = std::time(nullptr);
    std::tm now{};

    // ako neko pokrece na masini koja nije windows
    #if defined(_WIN32) || defined(_WIN64)
        localtime_s(&now, &t);
    #else
        localtime_r(&t, &now);
    #endif

    int hours = now.tm_hour;
    int minutes = now.tm_min;
    int seconds = now.tm_sec;

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
    int& bpm,
    float& bpmTimer,
    float& dHoldTimer
) {
    ekgOffset -= 0.08f * dt;
    if (ekgOffset <= -1.0f) ekgOffset += 1.0f;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        dHoldTimer += dt;

        if (dHoldTimer >= 0.1f) {
            bpm += 1;
            if (bpm > 200) bpm = 200;
            dHoldTimer = 0.0f;
        }

        ekgScaleX += 1.4f * dt;
        if (ekgScaleX > 5.0f) ekgScaleX = 5.0f;

        ekgOffset -= 1.3f * dt;
    }
    else {
        dHoldTimer = 0.0f;
        bpmTimer += dt;
        if (bpmTimer >= 2.0f) {
            bpm = 60 + rand() % 21; // 60-80
            bpmTimer = 0.0f;
        }

        ekgScaleX = 1.0f;
    }

    if (bpm >= 200) {
        drawQuad(VAO, shader, warningTexture, 0.0f, 0.0f, 0.8f);
    }
    else {
        drawQuadEKG(VAO, ekgShader, ekgTexture, 0.0f, -0.1f, 0.8f, ekgOffset, ekgScaleX);
    }

    drawNumber(VAO, shader, bpm, 0.0f, 0.6f, 0.1f, digits);
    drawQuad(VAO, shader, bpmTexture, 0.15f, 0.6f, 0.15f);

}






void createQuad(unsigned int& VAO, unsigned int& VBO) {
    // Quad po defaultu [-0.5,0.5] u oba pravca
    float vertices[] = {
        -0.5f,  0.5f, 0.0f, 1.0f, // gornje levo
        -0.5f, -0.5f, 0.0f, 0.0f, // donje levo
         0.5f, -0.5f, 1.0f, 0.0f, // donje desno
         0.5f,  0.5f, 1.0f, 1.0f  // gornje desno
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // aPos = location 0
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // aTex = location 1
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}


void preprocessTexture(unsigned& texture, const char* filepath) {
    texture = loadImageToTexture(filepath); 
    glBindTexture(GL_TEXTURE_2D, texture); 

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void loadTextures() {
    for (int i = 0; i <= 9; i++) {
         std::string path = "Resources/" + std::to_string(i) + ".png";
         preprocessTexture(digitTextures[i], path.c_str());
    }
    preprocessTexture(colonTexture, "Resources/colon.png");
    preprocessTexture(screenState[0], "Resources/heart_cursor.png");
    preprocessTexture(screenState[1], "Resources/heart_cursor.png");
    preprocessTexture(screenState[2], "Resources/heart_cursor.png");
    preprocessTexture(arrowLeft, "Resources/arrow-left.png");
    preprocessTexture(arrowRight, "Resources/arrow-right.png");
    preprocessTexture(ekgTexture, "Resources/ekg.png");
    preprocessTexture(warningTexture, "Resources/warning.png");
    preprocessTexture(studentTexture, "Resources/student.png");
    preprocessTexture(percentageTexture, "Resources/percentage.png");
    preprocessTexture(bpmTexture, "Resources/bpm.png");
}




int main() {
    // --- GLFW Inicijalizacija ---
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "2D Smartwatch", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

    // Cursor
    cursor = loadImageToCursor("Resources/heart_cursor.png");
    cursorPressed = loadImageToCursor("Resources/heart_cursor_pressed.png");
    glfwSetCursor(window, cursor);

    // --- GLEW ---
    if (glewInit() != GLEW_OK) return -1;

    // za transparentnost
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- Učitavanje tekstura ---
    loadTextures();

    // Shaders
    unsigned int shader = createShader("basic.vert", "basic.frag");
    unsigned int ekgShader = createShader("basic.vert", "ekg.frag");
    unsigned int batteryShader = createShader("battery.vert", "battery.frag");

    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "uTex0"), 0);

    // --- VAO/VBO kvadrata ---
    unsigned int VAO, VBO;
    createQuad(VAO, VBO);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);


    const float arrowXOffset = 0.7f;
    const float arrowY = 0.0f;
    const float arrowScale = 0.20f;

    // ekg
    float ekgOffset = 0.0f;      
    float ekgScaleX = 1.0f;     
    int bpm = 60;                 
    float bpmTimer = 0.0f;       
    float dHoldTimer = 0.0f;      

    double lastTime = glfwGetTime();

    // baterija
    float batteryPercent = 100.0f;
    float batteryTimer = 0.0f;

    float batteryCenterX = 0.0f;  // centar ekrana
    float batteryCenterY = 0.0f;
    float batteryWidth = 0.8f;    // širina okvira
    float batteryHeight = 0.25f;   // visina
    float batteryBorder = 0.8f;

    float frameTime = 1.0f / 75.0f;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        double currentTime = glfwGetTime();
        float dt = float(currentTime - lastTime);

        batteryTimer += dt;
        if (batteryTimer >= 0.5f) { 
            batteryPercent -= 1.0f;
            if (batteryPercent < 0.0f) batteryPercent = 100.0f;
            batteryTimer = 0.0f;
        }

        drawQuad(VAO, shader, studentTexture, 0.7f, -0.7f, 0.3f);

        // DRAW SCREEN
        if (currentScreen == SCREEN_CLOCK) {
            drawTime(VAO, shader, 0.15f, digitTextures, colonTexture);
        }
        else if (currentScreen == SCREEN_HEART) {
            drawEKGScreen(window, VAO, shader, ekgShader, ekgTexture, warningTexture, digitTextures,
                dt, ekgOffset, ekgScaleX, bpm, bpmTimer, dHoldTimer);
        }
        else if (currentScreen == SCREEN_BATTERY) {
            drawBatteryFrame(VAO, batteryShader, batteryCenterX, batteryCenterY, batteryWidth, batteryHeight, batteryBorder);
            drawBatteryProgress(VAO, batteryShader, batteryCenterX, batteryCenterY, batteryWidth, batteryHeight, batteryPercent);
            drawNumber(VAO, shader, int(batteryPercent), batteryCenterX, 0.6, 0.1f, digitTextures);
            drawQuad(VAO, shader, percentageTexture, batteryCenterX + 0.15, 0.6f, 0.1f);
        }

        // Arrows
        bool drawLeft = false, drawRight = false;
        switch (currentScreen) {
        case SCREEN_CLOCK:  drawRight = true; break;
        case SCREEN_HEART:  drawLeft = true; drawRight = true; break;
        case SCREEN_BATTERY: drawLeft = true; break;
        }

        if (drawLeft)  drawQuad(VAO, shader, arrowLeft, -arrowXOffset, arrowY, arrowScale);
        if (drawRight) drawQuad(VAO, shader, arrowRight, arrowXOffset, arrowY, arrowScale);

        handleArrowClicks(currentScreen, drawLeft, drawRight, arrowXOffset, arrowY, arrowScale, window, mousePressedLastFrame);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // FRAME LIMITER
        double frameEndTime = glfwGetTime();
        float sleepTime = frameTime - float(frameEndTime - currentTime);
        if (sleepTime > 0.0f) {
            std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
        }

        lastTime = currentTime;
    }


    glDeleteProgram(shader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

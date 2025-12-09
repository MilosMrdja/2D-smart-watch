#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ctime>
#include "Util.h"
#include "Render.h"
#include "Textures.h"
#include "Input.h"
#include <vector>
#include <thread>    
#include <chrono>    

struct SimTime {
    int hour;
    int minute;
    int second;
};

SimTime simulateClock(float dt) {
    static float accumulator = 0.0f;
    static SimTime time = { 12, 59, 55 }; 

    accumulator += dt;

    if (accumulator >= 1.0f) { 
        accumulator -= 1.0f;

        time.second++;

        if (time.second >= 60) {
            time.second = 0;
            time.minute++;
        }
        if (time.minute >= 60) {
            time.minute = 0;
            time.hour++;
        }
        if (time.hour >= 24) {
            time.hour = 0;
        }
    }

    return time;
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

    glClearColor(0.6f, 0.6f, 0.6f, 1.0f);


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
    float batteryWidth = 0.8f;   
    float batteryHeight = 0.25f;   
    float batteryBorder = 0.8f;

    float frameTime = 1.0f / 75.0f;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        double currentTime = glfwGetTime();
        float dt = float(currentTime - lastTime);

        // baterija
        batteryTimer += dt;
        if (batteryTimer >= 10.0f) { 
            batteryPercent -= 1.0f;
            if (batteryPercent < 0.0f) batteryPercent = 100.0f;
            batteryTimer = 0.0f;
        }

        // ekg
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            dHoldTimer += dt;

            if (dHoldTimer >= 0.1f) {
                bpm += 1;
                if (bpm > 200) bpm = 200;
                dHoldTimer = 0.0f;
            }

            ekgScaleX += 1.2f * dt;
            if (ekgScaleX > 5.0f) ekgScaleX = 5.0f;

            ekgOffset += 1.3f * dt;
        }
        else {
            dHoldTimer = 0;
            bpmTimer += dt;
            if (bpmTimer >= 2.0f) {
                bpm = 60 + rand() % 21;
                bpmTimer = 0.0f;
            }
            ekgScaleX = 1.0f;
        }


        // time
        std::time_t t = std::time(nullptr);
        std::tm now{};

        // ako neko pokrece na masini koja nije windows
        #if defined(_WIN32) || defined(_WIN64)
                localtime_s(&now, &t);
        #else
                localtime_r(&t, &now);
        #endif
                
        //SimTime now = simulateClock(dt);


        // name
        drawQuad(VAO, shader, studentTexture, 0.7f, -0.7f, 0.3f);

        // DRAW SCREEN
        if (currentScreen == SCREEN_CLOCK) {
            drawTime(VAO, shader, 0.15f, digitTextures, colonTexture,now.tm_hour, now.tm_min, now.tm_sec);
        }
        else if (currentScreen == SCREEN_HEART) {
            drawEKGScreen(window, VAO, shader, ekgShader, ekgTexture, warningTexture, digitTextures,
                dt, ekgOffset, ekgScaleX, bpm);
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

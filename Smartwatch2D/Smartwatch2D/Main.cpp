#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ctime>
#include "Util.h"
#include <vector>


GLFWcursor* cursor;
GLFWcursor* cursorPressed;

enum ScreenState {
    SCREEN_CLOCK = 0,
    SCREEN_HEART = 1,
    SCREEN_BATTERY = 2
};

ScreenState currentScreen = SCREEN_CLOCK;
bool mousePressedLastFrame = false;

float ekgOffset = 0.0f; // horizontalno pomeranje teksture
float ekgSpeed = 0.01f; // brzina pomeranja grafika
float ekgScaleX = 1.0f; // koliko se sužava/širi grafika
int bpm = 60;           // trenutni BPM
float timeAccumulator = 0.0f; // za simulaciju random BPM promena


unsigned int digitTextures[10];
unsigned int colonTexture;
unsigned int arrowLeft;
unsigned int arrowRight;
unsigned int screenState[3];
unsigned int warningTexture;
unsigned int ekgTexture;


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


// Funkcija za crtanje broja na ekranu
// VAO, shader - tvoj quad i shader
// number - broj koji se crta (npr. 75)
// x, y - centar broja na ekranu
// scale - veličina svake cifre
// digits - niz tekstura cifara [0..9]
void drawNumber(unsigned int VAO, unsigned int shader, int number,
    float x, float y, float scale, unsigned int digits[10])
{
    // Razdvajanje broja na cifre
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

    // horizontalni offset za crtanje (centriranje)
    float spacing = scale * 0.6f; // malo veće od scale
    float totalWidth = digitsVec.size() * spacing;
    float startX = x - totalWidth / 2.0f;

    // crtanje svake cifre
    for (int d : digitsVec) {
        drawQuad(VAO, shader, digits[d], startX, y, scale);
        startX += spacing;
    }
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
    for (int i = 1; i <= 9; i++) {
        if (i == 3 || i == 7) {
            std::string path = "Resources/1.jpeg";
            preprocessTexture(digitTextures[i], path.c_str());
            //digitTextures[i] = loadImageToTexture(path.c_str());
        }
        else {
            std::string path = "Resources/" + std::to_string(i) + ".jpeg";
            preprocessTexture(digitTextures[i], path.c_str());
            //digitTextures[i] = loadImageToTexture(path.c_str());
        }

    }
    preprocessTexture(colonTexture, "Resources/colon.png");
    preprocessTexture(screenState[0], "Resources/heart_cursor.png");
    preprocessTexture(screenState[1], "Resources/heart_cursor.png");
    preprocessTexture(screenState[2], "Resources/heart_cursor.png");
    preprocessTexture(arrowLeft, "Resources/left.jpg");
    preprocessTexture(arrowRight, "Resources/right.jpeg");
    preprocessTexture(ekgTexture, "Resources/ekg.jpeg");
    preprocessTexture(warningTexture, "Resources/warning.jpeg");


    /*
        colonTexture = loadImageToTexture("Resources/colon1.png");
    screenState[0] = loadImageToTexture("Resources/heart_cursor.png");
    screenState[1] = loadImageToTexture("Resources/heart_cursor.png");
    screenState[2] = loadImageToTexture("Resources/heart_cursor.png");
    */
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- Učitavanje tekstura ---
    loadTextures();

    // Shaders
    unsigned int shader = createShader("basic.vert", "basic.frag");
    unsigned int ekgShader = createShader("basic.vert", "ekg.frag");

    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "uTex0"), 0);

    // --- VAO/VBO kvadrata ---
    unsigned int VAO, VBO;
    createQuad(VAO, VBO);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    const float arrowXOffset = 0.7f;
    const float arrowY = 0.0f;
    const float arrowScale = 0.25f;
    const float mainScale = 0.8f;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        if (currentScreen == SCREEN_CLOCK) {
            drawTime(VAO, shader, 0.15f, digitTextures, colonTexture);
        }
        else if (currentScreen == SCREEN_HEART) {
            // horizontalno pomeranje teksture
            ekgOffset -= ekgSpeed;
            if (ekgOffset <= -1.0f) ekgOffset += 1.0f; // repeat texture

            // random BPM promena između 60-80
            timeAccumulator += 0.016f; // frame delta ~60fps
            if (timeAccumulator >= 1.0f) {
                bpm = 60 + rand() % 21;
                timeAccumulator = 0.0f;
            }

            // Simulacija trčanja tasterom D
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                bpm += 1;        // polako povećavamo
                ekgScaleX -= 0.005f; // sužavamo graf
                if (ekgScaleX < 0.2f) ekgScaleX = 0.2f;
            }

            // Prikaz upozorenja ako bpm > 200
            if (bpm > 200) {
                // crtanje crvenog kvadrata preko ekrana
                drawQuad(VAO, shader, warningTexture, 0.0f, 0.0f, 2.0f);
            }

            drawQuadEKG(VAO, ekgShader, ekgTexture, 0.0f, 0.0f, 0.8f,ekgOffset, ekgScaleX);

            // crtanje BPM iznad grafika
            drawNumber(VAO, shader, bpm, 0.0f, 0.8f, 0.1f, digitTextures);
        }else {
            drawQuad(VAO, shader, screenState[currentScreen], 0.0f, 0.0f, mainScale);
        }

        // --- crtanje strelica u zavisnosti od ekrana ---
        bool drawLeft = false, drawRight = false;
        switch (currentScreen) {
        case SCREEN_CLOCK:  drawRight = true; break;
        case SCREEN_HEART:  drawLeft = true; drawRight = true; break;
        case SCREEN_BATTERY: drawLeft = true; break;
        }

        if (drawLeft)  drawQuad(VAO, shader, arrowLeft, -arrowXOffset, arrowY, arrowScale);
        if (drawRight) drawQuad(VAO, shader, arrowRight, arrowXOffset, arrowY, arrowScale);
        
        handleArrowClicks(currentScreen, drawLeft, drawRight, arrowXOffset, arrowY, arrowScale,
            window, mousePressedLastFrame);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

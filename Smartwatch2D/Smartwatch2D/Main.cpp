#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Util.h"

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Smartwatch", NULL, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    // cursor
    GLFWcursor* cursor = loadImageToCursor("Resources/heart_cursor.png");
    if (cursor) glfwSetCursor(window, cursor);

    // Shader program
    unsigned int shader = createShader("basic.vert", "basic.frag");
    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "uTexture"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.2f, 0.8f, 0.6f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader);
        // Binduj teksturu (primer: pozadina sata)
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, watchFaceTex);

        // TODO: Renderuj kvadrat/pozadinu sa VAO/VBO

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
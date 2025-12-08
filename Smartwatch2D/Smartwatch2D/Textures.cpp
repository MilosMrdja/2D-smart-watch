#include "Textures.h"
#include "Util.h"

unsigned int digitTextures[10];
unsigned int colonTexture;
unsigned int arrowLeft;
unsigned int arrowRight;
unsigned int warningTexture;
unsigned int ekgTexture;
unsigned int studentTexture;
unsigned int percentageTexture;
unsigned int bpmTexture;

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
    preprocessTexture(arrowLeft, "Resources/arrow-left.png");
    preprocessTexture(arrowRight, "Resources/arrow-right.png");
    preprocessTexture(ekgTexture, "Resources/ekg.png");
    preprocessTexture(warningTexture, "Resources/warning.png");
    preprocessTexture(studentTexture, "Resources/student.png");
    preprocessTexture(percentageTexture, "Resources/percentage.png");
    preprocessTexture(bpmTexture, "Resources/bpm.png");
}

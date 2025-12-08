#pragma once
extern unsigned int digitTextures[10];
extern unsigned int colonTexture;
extern unsigned int ekgTexture;
extern unsigned int warningTexture;
extern unsigned int arrowLeft;
extern unsigned int arrowRight;
extern unsigned int bpmTexture;
extern unsigned int percentageTexture;
extern unsigned int studentTexture;

void preprocessTexture(unsigned& texture, const char* filepath);
void loadTextures();
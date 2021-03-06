#pragma once

unsigned short int size;
int **field;
void drawIntroScreen();
void drawField();
void fillLine(int Y, int colorpair);
void fatalError(char *msg);
void drawPopup(char *title, char *msg);
void fatalErrorCurses(char *msg);
void notify(char *title, char *msg);
void usage(char *binname);

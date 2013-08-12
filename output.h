#pragma once

unsigned short int size;
int **field;
void drawField();
void fatalError(char *msg);
void fatalErrorCurses(char *msg);
void notify(char *title, char *msg);
void usage(char *binname);

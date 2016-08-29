#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ProgSettings.h"

void draw();
void redrawTimer(int);
void screendump(int, int);
void key(unsigned char, int, int);
void fKey(int, int, int);
void reshape(int, int);
void traceMouse(int, int);
void mouseClick(int, int, int, int);
unsigned int defaults(unsigned int, int*, int*); 

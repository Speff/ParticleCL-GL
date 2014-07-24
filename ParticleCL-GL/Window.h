#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ProgSettings.h"

void draw();
void key(unsigned char, int, int);
void reshape(int, int);
void traceMouse(int, int);
unsigned int defaults(unsigned int, int*, int*); 
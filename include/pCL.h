#include <CL/cl.h>
#include <CL/cl_gl.h> 
#include <CL/cl_gl_ext.h>
#include <CL/cl_ext.h>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "ProgSettings.h"

void initCL();
void killCL();
void boilerplateCode();
void compileKernel();
void setMemMappings();
void runSim();
void readBuffer();
void writeFocalPointsToBuffers();
void checkErrorCode(char*, int);

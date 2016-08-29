#include "Window.h"
#include "pGL.h"
#include "pCL.h"

float resultArray[32][2];
float mouseX = 0;
float mouseY = 0;
float fps_counter = 0.0f;
int loop_counter = 0;
bool fullscreen = false;
bool takeSnapshot = false;
GLint windowSizeX = WIDTH;
GLint windowSizeY = HEIGHT;

// OpenCL Variables
char* kernelSource;
cl_uint numFocalPoints[1];
cl_float2 focalPoints[5];
cl_float2 focalPointsW[5];
cl_device_id device;
cl_program program;
cl_context context;
cl_command_queue cmdQueue;
cl_kernel kernel;
cl_mem bufposC_CL;
cl_mem bufposP_CL;
cl_mem bufFS_CL;
cl_mem bufFocalPoints_CL;
cl_mem bufnumFocalPoints_CL;

// OpenGL Variables
GLuint theProgram;
GLuint bufposC_GL;
GLuint bufFocalPoints_GL;
GLuint bufnumFocalPoints_GL;
GLuint vao;

int main(int argc, char **argv){
	// Initialize GLUT Window
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitWindowSize(WIDTH,HEIGHT);
	glutCreateWindow("OpenCL-GL Demo");
	glutInitDisplayMode(GLUT_DEPTH || GLUT_DOUBLE || GLUT_RGBA || GLUT_ALPHA);

	glutKeyboardFunc(key);
	glutSpecialFunc(fKey);
	glutDisplayFunc(draw);
	glutReshapeFunc(reshape);
	//glutPassiveMotionFunc(traceMouse);
	glutMouseFunc(mouseClick);
	glutIdleFunc(NULL);

    // Load OpenGL extensions
    if(ogl_LoadFunctions() == ogl_LOAD_FAILED){
        printf("Can't load OpenGL core extensions\n");
    }
	//printf("GL Error: %s\n", gluErrorString(glGetError()));
	
	//glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);


	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Initialize GL Functions and Objects
	initGL();
    //wglGetExtensionStringARB();
	// Wait for GL Programs to Finish
	glFinish();
	printf("GL initialized\n");
	// Initialize CL Functions and Objects. CL Context is Derived from GL Context
	initCL();
	printf("CL initialized\n");

	// Start Main Program Loop
	glutMainLoop();

	// Free Resources and Exit Program
	killCL();
	killGL();
	return 0;
}

void draw(){
	//cl_int status;
	int timeStarted = glutGet(GLUT_ELAPSED_TIME);
	int timeElapsed[6];

	// OpenCL Processing ------------------------------------------------
	clEnqueueAcquireGLObjects(cmdQueue, 1, &bufposC_CL, 0, NULL, NULL);
	clEnqueueAcquireGLObjects(cmdQueue, 1, &bufFocalPoints_CL, 0, NULL, NULL);
	clEnqueueAcquireGLObjects(cmdQueue, 1, &bufnumFocalPoints_CL, 0, NULL, NULL);

	timeElapsed[0] = glutGet(GLUT_ELAPSED_TIME);

	runSim();
	clFinish(cmdQueue);

	timeElapsed[1] = glutGet(GLUT_ELAPSED_TIME);

	clEnqueueReleaseGLObjects(cmdQueue, 1, &bufposC_CL, 0, NULL, NULL);
	clEnqueueReleaseGLObjects(cmdQueue, 1, &bufFocalPoints_CL, 0, NULL, NULL);
	clEnqueueReleaseGLObjects(cmdQueue, 1, &bufnumFocalPoints_CL, 0, NULL, NULL);
	// ---------------------------------------------------------------------

	timeElapsed[2] = glutGet(GLUT_ELAPSED_TIME);


	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(theProgram);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, bufposC_GL);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glUniform1i(glGetUniformLocation(theProgram, "timePassed"), glutGet(GLUT_ELAPSED_TIME));


	//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	//glWindowPos2d(20, 20);
	//glDisable(GL_TEXTURE);
	//glDisable(GL_TEXTURE_2D);
	//glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Testing");
	//glEnable(GL_TEXTURE);
	//glEnable(GL_TEXTURE_2D);


	glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
	glFinish();

	timeElapsed[4] = glutGet(GLUT_ELAPSED_TIME);

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	glutSwapBuffers();

	timeElapsed[5] = glutGet(GLUT_ELAPSED_TIME);

    if(takeSnapshot){
        screendump(WIDTH, HEIGHT);
        printf("Took screenshot\n");
        takeSnapshot = false;
    }

	//printf("Time to acquire objects\t\t%i\n", timeElapsed[0]-timeStarted);
	//printf("Time to run kernel\t\t%i\n", timeElapsed[1]-timeElapsed[0]);
	//printf("Time to release objects\t\t%i\n", timeElapsed[2]-timeElapsed[1]);
	//printf("Time to initialize GLbuf\t%i\n", timeElapsed[3]-timeElapsed[2]);
	//printf("Time to draw triangles\t\t%i\n", timeElapsed[4]-timeElapsed[3]);
	//printf("Time to release GLbuf\t\t%i\n", timeElapsed[5]-timeElapsed[4]);
	//printf("Total time\t\t\t%i\n\n", timeElapsed[5]-timeStarted);
    //printf("Timestamp finished draw: %i\n", glutGet(GLUT_ELAPSED_TIME));

    // Force 60-FPS. Redisplay is called in (16-drawTime)ms
    int timeToRedraw = 16 - (glutGet(GLUT_ELAPSED_TIME) - timeStarted);
    if(timeToRedraw > 0) glutTimerFunc(timeToRedraw, redrawTimer, 0);
    else{
        glutPostRedisplay();
    }
}

void screendump(int W, int H) {
    FILE   *out = fopen("screenshot.tga","wb");
    char   *pixel_data = new char[4*W*H];
    short  TGAhead[] = { 0, 2, 0, 0, 0, 0, (short)W, (short)H, 32 };

    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, W, H, GL_BGRA, GL_UNSIGNED_BYTE, pixel_data);

    fwrite(TGAhead, sizeof(TGAhead), 1, out);
    fwrite(pixel_data, 4*W*H, 1, out);
    fclose(out);
}

void redrawTimer(int dummy){
    glutPostRedisplay();
}

void key(unsigned char key, int xmouse, int ymouse){	
	switch (key){
	case 'z':
		numFocalPoints[0] = 0;
		writeFocalPointsToBuffers();
		break;

	case ' ': 
		glutLeaveMainLoop();
		break;

    case 'q':
        takeSnapshot = true;
        break;

	default:
		break;
	}
}

void fKey(int key, int xmouse, int ymouse){	
	switch (key){
	case GLUT_KEY_F1:
		break;
	default:
		break;
	}
}

void reshape(int w, int h){
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	windowSizeX = w;
	windowSizeY = h;
	glUniform2i(glGetUniformLocation(theProgram, "windowSize"), w, h);

	cl_float AS = (float)w/(float)h;
	clSetKernelArg(kernel, 5, sizeof(cl_float), &AS);
}

void traceMouse(int x, int y){

}

void mouseClick(int buttonClicked, int state, int x, int y){
	if(buttonClicked == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		if(numFocalPoints[0] >= NUM_FOCALPOINTS-1);
		else{
			focalPoints[numFocalPoints[0]].s[0] = -windowSizeX/(float)windowSizeY + 2*x/(float)windowSizeY;
			focalPoints[numFocalPoints[0]].s[1] = -(-1 + 2*y/(float)windowSizeY);
			focalPointsW[numFocalPoints[0]].s[0] = (float)x;
			focalPointsW[numFocalPoints[0]].s[1] = (float)y;

			numFocalPoints[0]++;
			writeFocalPointsToBuffers();
		}
	}
	else if(buttonClicked == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
		if(numFocalPoints[0] <= 0);
		else{
			focalPoints[numFocalPoints[0]].s[0] = 0.0f;
			focalPoints[numFocalPoints[0]].s[1] = 0.0f;
			focalPointsW[numFocalPoints[0]].s[0] = 0.0f;
			focalPointsW[numFocalPoints[0]].s[1] = 0.0f;

			numFocalPoints[0]--;
			writeFocalPointsToBuffers();
		}
	}
}

unsigned int defaults(unsigned int displayMode, int &width, int &height) {return displayMode;}

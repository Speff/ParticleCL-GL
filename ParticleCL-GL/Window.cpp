#include "Window.h"
#include "pGL.h"
#include "pCL.h"

float resultArray[32][2];
float mouseX = 0;
float mouseY = 0;
float fps_counter = 0.0f;
int loop_counter = 0;

// OpenCL Variables
char* kernelSource;
cl_uint numDevices;
cl_device_id *devices;
cl_program program;
cl_context context;
cl_command_queue cmdQueue;
cl_kernel kernel;
cl_mem bufposC_CL;
cl_mem bufposP_CL;
cl_mem bufFS_CL;
cl_mem bufMousePos;

// OpenGL Variables
GLuint theProgram;
GLuint bufposC_GL;
GLuint vao;

int main(int argc, char **argv){
	// Initialize GLUT Window
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitWindowSize(WIDTH,HEIGHT);
	glutCreateWindow("Test");
	glutInitDisplayMode(GLUT_DEPTH || GLUT_DOUBLE || GLUT_RGB);
	glutKeyboardFunc(key);
	glutDisplayFunc(draw);
	glutReshapeFunc(reshape);
	glutPassiveMotionFunc(traceMouse);
	glutIdleFunc(NULL);
	//glMatrixMode(GL_PROJECTION);

	glewExperimental = GL_TRUE; 
	glewInit();

	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);


	// Initialize GL Functions and Objects
	initGL();
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
	cl_int status;
	int timeStarted = glutGet(GLUT_ELAPSED_TIME);
	int timeElapsed[6];

	// OpenCL Processing ------------------------------------------------
	status = clEnqueueAcquireGLObjects(cmdQueue, 1, &bufposC_CL, NULL, NULL, NULL);

	timeElapsed[0] = glutGet(GLUT_ELAPSED_TIME);

	runSim();

	timeElapsed[1] = glutGet(GLUT_ELAPSED_TIME);

	status = clEnqueueReleaseGLObjects(cmdQueue, 1, &bufposC_CL, NULL, NULL, NULL);
	// ---------------------------------------------------------------------

	timeElapsed[2] = glutGet(GLUT_ELAPSED_TIME);


	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(theProgram);
	//printf("UseProgram\t\t%i\n", glGetError());
	glBindBuffer(GL_ARRAY_BUFFER, bufposC_GL);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	timeElapsed[3] = glutGet(GLUT_ELAPSED_TIME);

	glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);

	timeElapsed[4] = glutGet(GLUT_ELAPSED_TIME);

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	glutSwapBuffers();

	timeElapsed[5] = glutGet(GLUT_ELAPSED_TIME);

	///*printf("Time to acquire objects\t\t%i\n", timeElapsed[0]-timeStarted);
	//printf("Time to run kernel\t\t%i\n", timeElapsed[1]-timeElapsed[0]);
	//printf("Time to release objects\t\t%i\n", timeElapsed[2]-timeElapsed[1]);
	//printf("Time to initialize GLbuf\t%i\n", timeElapsed[3]-timeElapsed[2]);
	//printf("Time to draw triangles\t\t%i\n", timeElapsed[4]-timeElapsed[3]);
	//printf("Time to release GLbuf\t\t%i\n", timeElapsed[5]-timeElapsed[4]);
	//printf("Total time\t\t\t%i\n\n", timeElapsed[5]-timeStarted);*/

	printf("Time to acquire objects\t\t%i\n", timeElapsed[0]-timeStarted);
	printf("Time to run kernel\t\t%i\n", timeElapsed[1]-timeStarted);
	printf("Time to release objects\t\t%i\n", timeElapsed[2]-timeStarted);
	printf("Time to initialize GLbuf\t%i\n", timeElapsed[3]-timeStarted);
	printf("Time to draw triangles\t\t%i\n", timeElapsed[4]-timeStarted);
	printf("Time to release GLbuf\t\t%i\n", timeElapsed[5]-timeStarted);
	printf("Total time\t\t\t%i\n", timeElapsed[5]-timeStarted);

	loop_counter++;
	fps_counter += (timeElapsed[5]-(float)timeStarted)/120;
	if(loop_counter%120 == 0){
		printf("FPS: %f\n", 1000/fps_counter);
		fps_counter = 0;
	}


	glutPostRedisplay();
}

void key(unsigned char key, int xmouse, int ymouse){	
	switch (key){
	case 'w':
		glClearColor(1.0, 0.0, 1.0, 1.0);
		break;

	case 'r': 
		glClearColor(0.0, 0.0, 1.0, 1.0);
		break;

	case ' ': 
		glutLeaveMainLoop();
		break;

	default:
		break;
	}
	glutPostRedisplay(); //request display() call ASAP
}

void reshape(int w, int h){
	/*if(w > h)
		glViewport(0, 0, (GLsizei) w/((float)w/h), (GLsizei) h);
	else if (h > w)
		glViewport(0, 0, (GLsizei) w, (GLsizei) h/((float)h/w));
	else*/
		glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

void traceMouse(int x, int y){
	cl_float2 mousePos;
	mousePos.s[0] = -WIDTH/(float)HEIGHT + 2*x/(float)HEIGHT; // Found through trial and error
	mousePos.s[1] = -(-1 + 2*y/(float)HEIGHT);

	writeMousePosToBuffers(mousePos);

	//printf("%1.3f, %1.3f\n", mousePos.s[0], mousePos.s[1]);
}

unsigned int defaults(unsigned int displayMode, int &width, int &height) {return displayMode;}
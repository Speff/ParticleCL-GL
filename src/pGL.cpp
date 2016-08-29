#include "pGL.h"

extern GLuint theProgram;
extern GLuint vao;
extern GLuint bufposC_GL;
extern GLuint bufFocalPoints_GL;
extern GLuint bufnumFocalPoints_GL;

extern cl_uint numFocalPoints[1];
extern cl_float2 focalPoints[NUM_FOCALPOINTS];
extern cl_float2 focalPointsW[NUM_FOCALPOINTS];

GLfloat tempArrayC[NUM_PARTICLES][2]; // Array to initialize particles. Needs to be global

std::string FindFileOrThrow( const std::string &strBasename )
{
	std::string strFilename = "" + strBasename;
	std::ifstream testFile(strFilename.c_str());
	if(testFile.is_open()){
		return strFilename;
	}

	throw std::runtime_error("Could not find the file " + strBasename);
}

GLuint CreateShader(GLenum eShaderType, const std::string &strShaderFile)
{
	GLuint shader = glCreateShader(eShaderType);
	const char *strFileData = strShaderFile.c_str();
	glShaderSource(shader, 1, &strFileData, NULL);

	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

		const char *strShaderType = NULL;
		switch(eShaderType){
			case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
			case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
			case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}

		printf("Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
		delete[] strInfoLog;
	}
	printf("Created shader\n");

	return shader;
}

GLuint LoadShader(GLenum eShaderType, const std::string &strShaderFilename)
{
	std::string strFilename = FindFileOrThrow(strShaderFilename);
	std::ifstream shaderFile(strFilename.c_str());
	std::stringstream shaderData;
	shaderData << shaderFile.rdbuf();
	shaderFile.close();

	try
	{
		return CreateShader(eShaderType, shaderData.str());
	}
	catch(std::exception &e)
	{
		printf("%s\n", e.what());
		throw;
	}
}

GLuint CreateProgram(const std::vector<GLuint> &shaderList)
{
	GLuint program = glCreateProgram();
	//GLchar* ProgLog;
	//GLsizei progLog_size;
	GLint status;

	for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glAttachShader(program, shaderList[iLoop]);

	glLinkProgram(program);

	glGetProgramiv (program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		printf("Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}
	else printf("GL Program compiled\n");



	for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glDetachShader(program, shaderList[iLoop]);

	return program;
}



void createVBOs(){
	printf("Initializing Particle Array\n");

	// Initializing Particle Array -----------------------------------------------------------------
	//float** tempArray;
	GLfloat nCol = sqrt(NUM_PARTICLES);
	GLfloat isqrt = 1/sqrt(NUM_PARTICLES); // Offset between points
	long currRow = 0;
	long currCol = 0;

	for(long i = 0; i < NUM_PARTICLES; i++){
		if(currCol >= nCol){
			++currRow;
			currCol = 0;
		}
		// OpenGL draws from [-1, 1]. 
		tempArrayC[i][0] = 2*currCol*isqrt - 1 + isqrt;
		tempArrayC[i][1] = 2*currRow*isqrt - 1 + isqrt;
		++currCol;
	}
	// ---------------------------------------------------------------------------------------------

	// Initialize FS Array -----------------------------------
	//GLfloat tempFSarray[10];
	//tempFSarray[0] = -200/800.0;
	//tempFSarray[1] = 10/800.0;
	//tempFSarray[2] = 1/800.0;
	//tempFSarray[3] = 1/800.0;
	//tempFSarray[4] = 0/800.0;
	//tempFSarray[5] = 0/800.0;
	//tempFSarray[6] = 2/800.0;
	//tempFSarray[7] = 2/800.0;
	//tempFSarray[8] = 3/800.0;
	//tempFSarray[9] = 2/800.0;

	numFocalPoints[0] = 0;
	for(int i = 0; i < NUM_FOCALPOINTS; i++){
		focalPoints[i].s[0] = 0.0f;
		focalPoints[i].s[1] = 0.0f;

		focalPointsW[i].s[0] = 0.0f;
		focalPointsW[i].s[1] = 0.0f;
	}
	focalPoints[0].s[0] = 0;
	focalPoints[0].s[1] = 0;
	focalPointsW[0].s[0] = 0.0f;
	focalPointsW[0].s[1] = 0.0f;

	printf("Creating VBOs\n");

	glGenBuffers(1, &bufposC_GL);
	glBindBuffer(GL_ARRAY_BUFFER, bufposC_GL);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tempArrayC), tempArrayC, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &bufFocalPoints_GL);
	glBindBuffer(GL_ARRAY_BUFFER, bufFocalPoints_GL);
	glBufferData(GL_ARRAY_BUFFER, 2*sizeof(focalPoints), focalPoints, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &bufnumFocalPoints_GL);
	glBindBuffer(GL_ARRAY_BUFFER, bufnumFocalPoints_GL);
	glBufferData(GL_ARRAY_BUFFER, sizeof(numFocalPoints), numFocalPoints, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initGL(){
	std::vector<GLuint> shaderList;

	shaderList.push_back(LoadShader(GL_VERTEX_SHADER, "particle.vert"));
	shaderList.push_back(LoadShader(GL_FRAGMENT_SHADER, "particle.frag"));

	theProgram = CreateProgram(shaderList);

	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

	printf("Shaders detached\n");

	createVBOs();

	//glUseProgram(theProgram);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	//glUseProgram(0);


	glFinish();
}

void killGL(){

}

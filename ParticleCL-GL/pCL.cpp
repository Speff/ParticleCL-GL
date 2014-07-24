#include "pCL.h"

GLfloat tempArrayP[NUM_PARTICLES][2]; // Array to initialize particles. Needs to be global

// OpenCL Vars
extern char* kernelSource;
extern cl_uint numDevices;
extern cl_device_id *devices;
extern cl_program program;
extern cl_context context;
extern cl_command_queue cmdQueue;
extern cl_kernel kernel;

extern cl_mem bufposC_CL;
extern cl_mem bufposP_CL;
extern cl_mem bufFS_CL;
extern cl_mem bufMousePos;

// OpenGL Vars
extern GLuint bufposC_GL;

bool use_this_cl_platform(cl_platform_id platform){
	size_t extensions_size;
	char * extensions;

	clGetPlatformInfo( platform, CL_PLATFORM_EXTENSIONS, 0, NULL, &extensions_size);
	extensions = new char[extensions_size];
	clGetPlatformInfo( platform, CL_PLATFORM_EXTENSIONS, extensions_size, extensions, NULL);

	std::string ext = extensions;
	bool use_this_platform = ext.find("cl_khr_gl_sharing") != std::string::npos;

	delete[] extensions;

	return use_this_platform;
}

bool use_this_cl_device(cl_device_id device){
	size_t exensions_size;
	char * extensions;

	clGetDeviceInfo( device, CL_DEVICE_EXTENSIONS, 0, NULL, &exensions_size);
	extensions = new char[exensions_size];
	clGetDeviceInfo( device, CL_DEVICE_EXTENSIONS, exensions_size, extensions, NULL);

	std::string ext = extensions;
	bool use_this_device = ext.find("cl_khr_gl_sharing") != std::string::npos;

	delete[] extensions;

	return use_this_device;
}

void initCL(){
	boilerplateCode();
	compileKernel();
	setMemMappings();
}

void killCL(){
	// Free OpenCL resources
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmdQueue);
	clReleaseContext(context);
	free(devices);
}

void writeMousePosToBuffers(cl_float2 mousePos){
	cl_int status;
	cl_float2 mouseLoc[1];


	mouseLoc[0] = mousePos;

	// Write arrays to the device buffers. bufPos is unnecessary
	status = clEnqueueWriteBuffer(cmdQueue, bufMousePos, CL_TRUE, 0, sizeof(cl_float2), mouseLoc, 0, NULL, NULL);
	//checkErrorCode("Writing bufMousePos...\t", status);
}

void setMemMappings(){
	cl_int status;

	cl_float deadZone = DEADZONE;
	cl_float velD = VEL_DAMP;
	cl_float AS = WIDTH/(float)HEIGHT;
	cl_float2 MP[1];

	// Initializing Particle Array ----------------------------
	//float** tempArray;
	GLfloat nCol = sqrtl(NUM_PARTICLES);
	GLfloat isqrt = 1/sqrtl(NUM_PARTICLES); // Offset between points
	long currRow = 0;
	long currCol = 0;

	for(long i = 0; i < NUM_PARTICLES; i++){
		if(currCol >= nCol){
			++currRow;
			currCol = 0;
		}
		// OpenGL draws from [-1, 1]. 
		tempArrayP[i][0] = 2*currCol*isqrt - 1 + isqrt;
		tempArrayP[i][1] = 2*currRow*isqrt - 1 + isqrt;
		++currCol;
	}

	// Initialize Mouse Position Array ----------------------
	MP[0].s[0] = 0.0f;
	MP[0].s[1] = 0.0f;

	// Initialize FS Array -----------------------------------
	cl_float tempFSarray[10];
	tempFSarray[0] = -10/800.0;
	tempFSarray[1] = -10/800.0;
	tempFSarray[2] = 1/800.0;
	tempFSarray[3] = 1/800.0;
	tempFSarray[4] = 0/800.0;
	tempFSarray[5] = 0/800.0;
	tempFSarray[6] = 2/800.0;
	tempFSarray[7] = 2/800.0;
	tempFSarray[8] = 3/800.0;
	tempFSarray[9] = 2/800.0;


	// Create a buffer object
	bufposC_CL = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, bufposC_GL, &status);
	checkErrorCode("Creating bufposC_CL...\t", status);
	bufposP_CL = clCreateBuffer(context, CL_MEM_READ_WRITE, NUM_PARTICLES*sizeof(cl_float2), NULL, &status);
	checkErrorCode("Creating bufposP_CL...\t", status);
	bufFS_CL = clCreateBuffer(context, CL_MEM_READ_ONLY, 10*sizeof(cl_float), NULL, &status);
	checkErrorCode("Creating bufFSAry...\t", status);
	bufMousePos = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float2), NULL, &status);
	checkErrorCode("Creating buf (MousePos)...", status);

	// Copying bufC ----------------------------------
	status = clEnqueueAcquireGLObjects(cmdQueue, 1, &bufposC_CL, NULL, NULL, NULL);
	checkErrorCode("Aquiring bufC:\t\t", status);

	status = clEnqueueWriteBuffer(cmdQueue, bufFS_CL, CL_FALSE, 0, sizeof(tempFSarray), tempFSarray, NULL, NULL, NULL);
	checkErrorCode("Copying to bufFS...\t", status);

	status = clEnqueueCopyBuffer(cmdQueue, bufposC_CL, bufposP_CL, 0, 0, NUM_PARTICLES*sizeof(cl_float2), NULL, NULL, NULL);
	checkErrorCode("Copying to bufPosP...\t", status);
	
	status = clEnqueueReleaseGLObjects(cmdQueue, 1, &bufposC_CL, NULL, NULL, NULL);
	checkErrorCode("Releasing bufC:\t\t", status);
	// -----------------------------------------------


	// Associate the input and output buffers & variables with the kernel
	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufposC_CL); checkErrorCode("Setting KernelArg(0)...\t", status);
	status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufposP_CL); checkErrorCode("Setting KernelArg(1)...\t", status);
	status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufFS_CL); checkErrorCode("Setting KernelArg(2)...\t", status);
	status = clSetKernelArg(kernel, 3, sizeof(cl_float), &deadZone); checkErrorCode("Setting KernelArg(3)...\t", status);
	status = clSetKernelArg(kernel, 4, sizeof(cl_float), &velD); checkErrorCode("Setting KernelArg(4)...\t", status);
	status = clSetKernelArg(kernel, 5, sizeof(cl_float), &AS); checkErrorCode("Setting KernelArg(5)...\t", status);
	status = clSetKernelArg(kernel, 6, sizeof(cl_mem), &bufMousePos); checkErrorCode("Setting KernelArg(6)...\t", status);

	clFinish(cmdQueue);
}

void runSim(){
	// Define an index space (global work size) of work items for execution
	// A workgroup size (local work size) is not required, but can be used.
	size_t globalWorkSize[1];
	cl_int status;

	globalWorkSize[0] = NUM_PARTICLES;

	// Execute the kernel for execution
	status = clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);
	//checkErrorCode("Running Sim...\t\t", status);
}

extern float resultArray[32][2];

void readBuffer(){
	cl_int status;

	// Read the device output buffer to the host output array
	status = clEnqueueReadBuffer(cmdQueue, bufposC_CL, CL_FALSE, 0, 32 * sizeof(cl_float2), resultArray, 0, NULL, NULL);
	checkErrorCode("Reading bufPos...\t", status);
	clFinish(cmdQueue);

	//printf("Particle: %i\tLocation:\t%f, %f\n\t\t\t\t%f, %f\n", (int) kernelTestVal[0].s[1], kernelTestVal[1].s[0], kernelTestVal[1].s[1], kernelTestVal[2].s[0], kernelTestVal[2].s[1]);
	
}

void compileKernel(){
	FILE *fp;
	char fileLocation[] = "pkernel";
	size_t sourceSize;
	cl_int status;

	fp = fopen(fileLocation, "r");
	if(!fp){
		printf("Can't read kernel source\n");
	}

	fseek(fp, 0, SEEK_END);
	sourceSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	kernelSource = (char*)calloc(1, sourceSize + 1);
	fread(kernelSource, 1, sourceSize, fp);
	fclose(fp);
	//printf("%s", kernelSource);
	if(VERBOSE) printf("Kernel source read\n");

	// Create a program using the source code
	program = clCreateProgramWithSource(context, 1, (const char**)&kernelSource, NULL, &status);
	checkErrorCode("Creating program...\t", status);

	// Compile the program
	status = clBuildProgram(program, numDevices, devices, NULL, NULL, NULL);
	checkErrorCode("Compiling program...\t", status);

	char* buildLog;
	size_t buildLogSize;
	clGetProgramBuildInfo(program,*devices,CL_PROGRAM_BUILD_LOG, NULL, NULL, &buildLogSize);
	buildLog = (char*)malloc(buildLogSize);
	clGetProgramBuildInfo(program,*devices,CL_PROGRAM_BUILD_LOG, buildLogSize, buildLog, NULL);
	if(buildLogSize > 2) printf("%s\n",buildLog);
	free(buildLog);

	// Create the vector addition kernel
	kernel = clCreateKernel(program, "updateParticle", &status);
	checkErrorCode("Creating kernel...\t", status);
}

void boilerplateCode(){
	// Use this to check the output of each API call
	cl_int status;

	// Retrieve the number of platforms
	cl_uint numPlatforms = 0;
	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	checkErrorCode("Getting platforms...\t", status);

	// Allocate enough space for each platform
	cl_platform_id *platforms = NULL;
	platforms = (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));

	// Fill in the platforms
	status = clGetPlatformIDs(numPlatforms, platforms, NULL);
	checkErrorCode("Filling platforms...\t", status);

	// Retrieve the number of devices
	status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);

	// Allocate space for each device
	devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));

	// Fill in the devices
	status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
	checkErrorCode("Filling devices...\t", status);

	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), // WGL Context  
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(), // WGL HDC
		CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0], // OpenCL platform
		0
	};	

	// Create a contect and associate it with the devices
	context = clCreateContext(properties, numDevices, devices, NULL, NULL, &status);
	checkErrorCode("Creating context...\t", status);

	// Create a command queue and associate it with the device
	cmdQueue = clCreateCommandQueue(context, devices[0], 0, &status);
	checkErrorCode("Creating cmd queue...\t", status);

	char* devName;
	size_t nameSize;
	clGetDeviceInfo(devices[0], CL_DEVICE_NAME, NULL, NULL, &nameSize);
	devName = (char*)malloc(nameSize);
	clGetDeviceInfo(devices[0], CL_DEVICE_NAME, nameSize, devName, NULL);
	if(status == CL_SUCCESS && VERBOSE) printf("Using device:\t\t%s\n", devName); 

	free(platforms);
	free(devName);
}

void checkErrorCode(char* action, int errorCode){
	if(!VERBOSE) return;
	else{
		printf("%s\t",action);
		if(errorCode == CL_SUCCESS) printf("CL_SUCCESS\n");
		if(errorCode == CL_DEVICE_NOT_FOUND) printf("CL_DEVICE_NOT_FOUND\n");
		if(errorCode == CL_DEVICE_NOT_AVAILABLE) printf("CL_DEVICE_NOT_AVAILABLE\n");
		if(errorCode == CL_COMPILER_NOT_AVAILABLE) printf("CL_COMPILER_NOT_AVAILABLE\n");
		if(errorCode == CL_MEM_OBJECT_ALLOCATION_FAILURE) printf("CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
		if(errorCode == CL_OUT_OF_RESOURCES) printf("CL_OUT_OF_RESOURCES\n");
		if(errorCode == CL_OUT_OF_HOST_MEMORY) printf("CL_OUT_OF_HOST_MEMORY\n");
		if(errorCode == CL_PROFILING_INFO_NOT_AVAILABLE) printf("CL_PROFILING_INFO_NOT_AVAILABLE\n");
		if(errorCode == CL_MEM_COPY_OVERLAP) printf("CL_MEM_COPY_OVERLAP\n");
		if(errorCode == CL_IMAGE_FORMAT_MISMATCH) printf("CL_IMAGE_FORMAT_MISMATCH\n");
		if(errorCode == CL_IMAGE_FORMAT_NOT_SUPPORTED) printf("CL_IMAGE_FORMAT_NOT_SUPPORTED\n");
		if(errorCode == CL_BUILD_PROGRAM_FAILURE) printf("CL_BUILD_PROGRAM_FAILURE\n");
		if(errorCode == CL_MAP_FAILURE) printf("CL_MAP_FAILURE\n");
		if(errorCode == CL_INVALID_VALUE) printf("CL_INVALID_VALUE\n");
		if(errorCode == CL_INVALID_DEVICE_TYPE) printf("CL_INVALID_DEVICE_TYPE\n");
		if(errorCode == CL_INVALID_PLATFORM) printf("CL_INVALID_PLATFORM\n");
		if(errorCode == CL_INVALID_PROPERTY) printf("CL_INVALID_PROPERTY\n");
		//if(errorCode == CL_INVALID_D3D10_DEVICE_KHR ) printf("CL_INVALID_D3D10_DEVICE_KHR\n");
		if(errorCode == CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR ) printf("CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR\n");			
		if(errorCode == CL_INVALID_DEVICE) printf("CL_INVALID_DEVICE\n");
		if(errorCode == CL_INVALID_CONTEXT) printf("CL_INVALID_CONTEXT\n");
		if(errorCode == CL_INVALID_QUEUE_PROPERTIES) printf("CL_INVALID_QUEUE_PROPERTIES\n");
		if(errorCode == CL_INVALID_COMMAND_QUEUE) printf("CL_INVALID_COMMAND_QUEUE\n");
		if(errorCode == CL_INVALID_HOST_PTR) printf("CL_INVALID_HOST_PTR\n");
		if(errorCode == CL_INVALID_MEM_OBJECT) printf("CL_INVALID_MEM_OBJECT\n");
		if(errorCode == CL_INVALID_IMAGE_FORMAT_DESCRIPTOR) printf("CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n");
		if(errorCode == CL_INVALID_IMAGE_SIZE) printf("CL_INVALID_IMAGE_SIZE\n");
		if(errorCode == CL_INVALID_SAMPLER) printf("CL_INVALID_SAMPLER\n");
		if(errorCode == CL_INVALID_BINARY) printf("CL_INVALID_BINARY\n");
		if(errorCode == CL_INVALID_BUILD_OPTIONS) printf("CL_INVALID_BUILD_OPTIONS\n");
		if(errorCode == CL_INVALID_PROGRAM) printf("CL_INVALID_PROGRAM\n");
		if(errorCode == CL_INVALID_PROGRAM_EXECUTABLE) printf("CL_INVALID_PROGRAM_EXECUTABLE\n");
		if(errorCode == CL_INVALID_KERNEL_NAME) printf("CL_INVALID_KERNEL_NAME\n");
		if(errorCode == CL_INVALID_KERNEL_DEFINITION) printf("CL_INVALID_KERNEL_DEFINITION\n");
		if(errorCode == CL_INVALID_KERNEL) printf("CL_INVALID_KERNEL\n");
		if(errorCode == CL_INVALID_ARG_INDEX) printf("CL_INVALID_ARG_INDEX\n");
		if(errorCode == CL_INVALID_ARG_VALUE) printf("CL_INVALID_ARG_VALUE\n");
		if(errorCode == CL_INVALID_ARG_SIZE) printf("CL_INVALID_ARG_SIZE\n");
		if(errorCode == CL_INVALID_KERNEL_ARGS) printf("CL_INVALID_KERNEL_ARGS\n");
		if(errorCode == CL_INVALID_WORK_DIMENSION) printf("CL_INVALID_WORK_DIMENSION\n");
		if(errorCode == CL_INVALID_WORK_GROUP_SIZE) printf("CL_INVALID_WORK_GROUP_SIZE\n");
		if(errorCode == CL_INVALID_WORK_ITEM_SIZE) printf("CL_INVALID_WORK_ITEM_SIZE\n");
		if(errorCode == CL_INVALID_GLOBAL_OFFSET) printf("CL_INVALID_GLOBAL_OFFSET\n");
		if(errorCode == CL_INVALID_EVENT_WAIT_LIST) printf("CL_INVALID_EVENT_WAIT_LIST\n");
		if(errorCode == CL_INVALID_EVENT) printf("CL_INVALID_EVENT\n");
		if(errorCode == CL_INVALID_OPERATION) printf("CL_INVALID_OPERATION\n");
		if(errorCode == CL_INVALID_GL_OBJECT) printf("CL_INVALID_GL_OBJECT\n");
		if(errorCode == CL_INVALID_BUFFER_SIZE) printf("CL_INVALID_BUFFER_SIZE\n");
		if(errorCode == CL_INVALID_MIP_LEVEL) printf("CL_INVALID_MIP_LEVEL\n");
		if(errorCode == CL_INVALID_GLOBAL_WORK_SIZE) printf("CL_INVALID_GLOBAL_WORK_SIZE\n");
	}
}
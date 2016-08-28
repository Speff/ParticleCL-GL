#include "pCL.h"

GLfloat tempArrayP[NUM_PARTICLES][2]; // Array to initialize particles. Needs to be global
extern float resultArray[32][2];

// OpenCL Vars
extern char* kernelSource;
extern cl_device_id device;
extern cl_program program;
extern cl_context context;
extern cl_command_queue cmdQueue;
extern cl_kernel kernel;
extern cl_uint numFocalPoints[1];
extern cl_float2 focalPoints[NUM_FOCALPOINTS];
extern cl_float2 focalPointsW[NUM_FOCALPOINTS];

extern cl_mem bufposC_CL;
extern cl_mem bufposP_CL;
extern cl_mem bufFS_CL;
extern cl_mem bufFocalPoints_CL;
extern cl_mem bufnumFocalPoints_CL;

// OpenGL Vars
extern GLuint bufposC_GL;
extern GLuint bufFocalPoints_GL;
extern GLuint bufnumFocalPoints_GL;
extern GLuint numFocalPointsBindingIndex;
extern GLuint focalPointsBindingIndex;

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
}

void writeFocalPointsToBuffers(){
    //cl_int status;

    //printf("Number of focal points: %i\n", numFocalPoints[0]);

    //for(int i = 0; i < numFocalPoints[0]; i++){
    //	printf("Point %i\t\t%1.3f, %1.3f\n", i, focalPointsW[i].s[0],focalPointsW[i].s[1]);
    //}



    clEnqueueAcquireGLObjects(cmdQueue, 1, &bufFocalPoints_CL, 0, NULL, NULL);
    //checkErrorCode("Aquiring bufFocalPoints_CL:\t\t", status);
    clEnqueueAcquireGLObjects(cmdQueue, 1, &bufnumFocalPoints_CL, 0, NULL, NULL);
    //checkErrorCode("Aquiring bufnumFocalPoints_CL:\t\t", status);


    // Write arrays to the device buffers. bufPos is unnecessary
    clEnqueueWriteBuffer(cmdQueue, bufFocalPoints_CL, CL_FALSE, 0, NUM_FOCALPOINTS*sizeof(cl_float2), focalPoints, 0, NULL, NULL);
    //checkErrorCode("Writing bufFocalPoints_CL...\t", status);
    clEnqueueWriteBuffer(cmdQueue, bufFocalPoints_CL, CL_FALSE, NUM_FOCALPOINTS*sizeof(cl_float2), NUM_FOCALPOINTS*sizeof(cl_float2), focalPointsW, 0, NULL, NULL);
    //checkErrorCode("Writing bufFocalPointsW_CL...\t", status);
    clEnqueueWriteBuffer(cmdQueue, bufnumFocalPoints_CL, CL_TRUE, 0, sizeof(cl_uint), numFocalPoints, 0, NULL, NULL);
    //checkErrorCode("Writing bufnumFocalPoints_CL...\t", status);

    clEnqueueReleaseGLObjects(cmdQueue, 1, &bufFocalPoints_CL, 0, NULL, NULL);
    //checkErrorCode("Releasing bufC:\t\t", status);
    clEnqueueReleaseGLObjects(cmdQueue, 1, &bufnumFocalPoints_CL, 0, NULL, NULL);
    //checkErrorCode("Releasing bufC:\t\t", status);
}

void setMemMappings(){
    cl_int status;						// Used to check for errors

    cl_float deadZone = DEADZONE;		// Deadzone around focal points
    cl_float velD = VEL_DAMP;			// Damping constant for particle velocity
    cl_float AS = WIDTH/(float)HEIGHT;	// Aspect ratio for kernel calculations
    //cl_float2 MP[1];					// Mouse Position buffer to initialize kernel input

    // Initializing Particle Array ----------------------------
    float nCol = sqrt(NUM_PARTICLES);	//  Number of particles in a row
    float isqrt = 1/sqrt(NUM_PARTICLES);// Offset between particles
    long currRow = 0;
    long currCol = 0;

    for(long i = 0; i < NUM_PARTICLES; i++){
        if(currCol >= nCol){
            ++currRow;
            currCol = 0;
        }
        // Spread particles evenly from [-1,+1]. Spacing is _isqrt_
        tempArrayP[i][0] = 2*currCol*isqrt - 1 + isqrt;
        tempArrayP[i][1] = 2*currRow*isqrt - 1 + isqrt;
        ++currCol;
    }

    // Initialize Field Strength Array --------------------------
    cl_float tempFSarray[2];
    tempFSarray[0] = -200/800.0/20;
    tempFSarray[1] = 10/800.0/20;
    //tempFSarray[2] = 1/800.0;
    //tempFSarray[3] = 1/800.0;
    //tempFSarray[4] = 0/800.0;
    //tempFSarray[5] = 0/800.0;
    //tempFSarray[6] = 2/800.0;
    //tempFSarray[7] = 2/800.0;
    //tempFSarray[8] = 3/800.0;
    //tempFSarray[9] = 2/800.0;


    // Create buffer objects --------------------------------------
    // Current Position Buffer (posC) is created from a GL VBO since this is the buffer to draw
    bufposC_CL = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, bufposC_GL, &status);
    checkErrorCode("Creating bufposC_CL...\t", status);
    // Previous Position Buffer (posP) is purely a CL buffer since it has no interaction with GL
    bufposP_CL = clCreateBuffer(context, CL_MEM_READ_WRITE, NUM_PARTICLES*sizeof(cl_float2), NULL, &status);
    checkErrorCode("Creating bufposP_CL...\t", status);
    // Field Strength Buffer is only used to update particle position in CL
    bufFS_CL = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(tempFSarray), NULL, &status);
    checkErrorCode("Creating bufFSAry...\t", status);
    // Focal Point information can possibly be used in the future in a GLSL shader, so it's made from a VBO
    bufFocalPoints_CL = clCreateFromGLBuffer(context, CL_MEM_READ_ONLY, bufFocalPoints_GL, &status);
    checkErrorCode("Creating buf (bufFocalPoints_CL)...\t", status);
    // Num Focal Points Buffer used to show amount of focal points in use
    bufnumFocalPoints_CL = clCreateFromGLBuffer(context, CL_MEM_READ_ONLY, bufnumFocalPoints_GL, &status);
    checkErrorCode("Creating buf (bufnumFocalPoints_CL)...\t", status);

    // Filling buffers -------------------------------
    // Release Current Position Buffer from GL to address it
    status = clEnqueueAcquireGLObjects(cmdQueue, 1, &bufposC_CL, 0, NULL, NULL);
    checkErrorCode("Aquiring bufC:\t\t", status);

    // Initialize the Field Strength Buffer to the values initialized in tempFSarray
    status = clEnqueueWriteBuffer(cmdQueue, bufFS_CL, CL_FALSE, 0, sizeof(tempFSarray), tempFSarray, 0, NULL, NULL);
    checkErrorCode("Copying to bufFS...\t", status);
    // Initialize the Previous Position Buffer with the Current Position Buffer
    status = clEnqueueCopyBuffer(cmdQueue, bufposC_CL, bufposP_CL, 0, 0, NUM_PARTICLES*sizeof(cl_float2), 0, NULL, NULL);
    checkErrorCode("Copying to bufPosP...\t", status);

    // Release GL VBO after use
    status = clEnqueueReleaseGLObjects(cmdQueue, 1, &bufposC_CL, 0, NULL, NULL);
    checkErrorCode("Releasing bufC:\t\t", status);
    // -----------------------------------------------


    // Associate the input and output buffers & variables with the kernel
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufposC_CL); checkErrorCode("Setting KernelArg(0)...\t", status);
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufposP_CL); checkErrorCode("Setting KernelArg(1)...\t", status);
    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufFS_CL); checkErrorCode("Setting KernelArg(2)...\t", status);
    status = clSetKernelArg(kernel, 3, sizeof(cl_float), &deadZone); checkErrorCode("Setting KernelArg(3)...\t", status);
    status = clSetKernelArg(kernel, 4, sizeof(cl_float), &velD); checkErrorCode("Setting KernelArg(4)...\t", status);
    status = clSetKernelArg(kernel, 5, sizeof(cl_float), &AS); checkErrorCode("Setting KernelArg(5)...\t", status);
    status = clSetKernelArg(kernel, 6, sizeof(cl_mem), &bufnumFocalPoints_CL); checkErrorCode("Setting KernelArg(6)...\t", status);
    status = clSetKernelArg(kernel, 7, sizeof(cl_mem), &bufFocalPoints_CL); checkErrorCode("Setting KernelArg(7)...\t", status);

    // Wait until command queue events are completed
    clFinish(cmdQueue);
}

void runSim(){
    // Define an index space (global work size) of work items for execution
    // A workgroup size (local work size) is not required, but can be used.
    size_t globalWorkSize[1];
    //cl_int status;

    globalWorkSize[0] = NUM_PARTICLES;

    // Execute the kernel for execution
    clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);
    //checkErrorCode("Running Sim...\t\t", status);
}


void readBuffer(){
    //cl_int status;

    // Read the device output buffer to the host output array
    //status = clEnqueueReadBuffer(cmdQueue, bufposC_CL, CL_FALSE, 0, 32 * sizeof(cl_float2), resultArray, 0, NULL, NULL);
    //checkErrorCode("Reading bufPos...\t", status);
    //clFinish(cmdQueue);
}

void compileKernel(){
    FILE *fp;
    char fileLocation[] = "pkernel";
    size_t sourceSize;
    cl_int status;

    // Read kernel source code into _kernelSource_ ------------------------
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
    status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    checkErrorCode("Compiling program...\t", status);

    char* buildLog;
    size_t buildLogSize;
    clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG, 0, NULL, &buildLogSize);
    buildLog = (char*)malloc(buildLogSize);
    clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG, buildLogSize, buildLog, NULL);
    if(buildLogSize > 2) printf("%s\n",buildLog);
    free(buildLog);

    // Create the vector addition kernel
    kernel = clCreateKernel(program, "updateParticle", &status);
    checkErrorCode("Creating kernel...\t", status);
}

void boilerplateCode(){
    // Use this to check the output of each API call
    cl_int status;
    cl_uint platformNumber;

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

    for(platformNumber = 0; platformNumber < numPlatforms; platformNumber++){
        clGetGLContextInfoKHR_fn clGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn)clGetExtensionFunctionAddressForPlatform(platforms[platformNumber], "clGetGLContextInfoKHR");

        cl_context_properties properties[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), // WGL Context  
            CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(), // WGL HDC
            CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platformNumber], // OpenCL platform
            0
        };	
        status = clGetGLContextInfoKHR(properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof(device), &device, NULL);
        checkErrorCode("Finding OpenGL dev", status);
        if(status == CL_SUCCESS){
            // Create a contect and associate it with the devices
            context = clCreateContext(properties, 1, &device, NULL, NULL, &status);
            //context = clCreateContextFromType(properties, CL_DEVICE_TYPE_GPU, &pfn_notify, NULL, &status);
            checkErrorCode("Creating context...\t", status);
            break;
        }
    } 

    // Create a command queue and associate it with the device
    cmdQueue = clCreateCommandQueue(context, device, 0, &status);
    checkErrorCode("Creating cmd queue...\t", status);

    char* devName;
    size_t nameSize;
    clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &nameSize);
    devName = (char*)malloc(nameSize);
    clGetDeviceInfo(device, CL_DEVICE_NAME, nameSize, devName, NULL);
    /*if(status == CL_SUCCESS && VERBOSE) */printf("Using device:\t\t%s\n", devName); 

    free(platforms);
    free(devName);
}

void checkErrorCode(char const* action, int errorCode){
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

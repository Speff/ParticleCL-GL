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
void checkErrorCode(char const*, int);

extern "C" void CL_CALLBACK pfn_notify(const char *, const void *, size_t, void *);

#ifndef cl_khr_gl_sharing
#define cl_khr_gl_sharing     1

// all data typedefs, token #defines, prototypes, and
// function pointer typedefs for this extension
#define CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR -1000
#define CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR   0x2006
#define CL_DEVICES_FOR_GL_CONTEXT_KHR          0x2007
#define CL_GL_CONTEXT_KHR                      0x2008
#define CL_EGL_DISPLAY_KHR                     0x2009
#define CL_GLX_DISPLAY_KHR                     0x200A
#define CL_WGL_HDC_KHR                         0x200B
#define CL_CGL_SHAREGROUP_KHR                  0x200C

// function pointer typedefs must use the
// following naming convention
typedef CL_API_ENTRY cl_int
     (CL_API_CALL *clGetGLContextInfoKHR_fn)(
              const cl_context_properties * /* properties */,
              cl_gl_context_info /* param_name */,
              size_t /* param_value_size */,
              void * /* param_value */,
              size_t * /*param_value_size_ret*/);
#endif // cl_khr_gl_sharing

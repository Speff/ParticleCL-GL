#ifndef POINTER_C_GENERATED_HEADER_WINDOWSGL_H
#define POINTER_C_GENERATED_HEADER_WINDOWSGL_H

#ifdef __wglext_h_
#error Attempt to include auto-generated WGL header after wglext.h
#endif

#define __wglext_h_

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <windows.h>

#ifdef CODEGEN_FUNCPTR
#undef CODEGEN_FUNCPTR
#endif /*CODEGEN_FUNCPTR*/
#define CODEGEN_FUNCPTR WINAPI

#ifndef GL_LOAD_GEN_BASIC_OPENGL_TYPEDEFS
#define GL_LOAD_GEN_BASIC_OPENGL_TYPEDEFS

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
#define GLvoid void

#endif /*GL_LOAD_GEN_BASIC_OPENGL_TYPEDEFS*/


#ifndef GL_LOAD_GEN_BASIC_OPENGL_TYPEDEFS
#define GL_LOAD_GEN_BASIC_OPENGL_TYPEDEFS


#endif /*GL_LOAD_GEN_BASIC_OPENGL_TYPEDEFS*/


struct _GPU_DEVICE {
    DWORD  cb;
    CHAR   DeviceName[32];
    CHAR   DeviceString[128];
    DWORD  Flags;
    RECT   rcVirtualScreen;
};
DECLARE_HANDLE(HPBUFFERARB);
DECLARE_HANDLE(HPBUFFEREXT);
DECLARE_HANDLE(HVIDEOOUTPUTDEVICENV);
DECLARE_HANDLE(HPVIDEODEV);
DECLARE_HANDLE(HGPUNV);
DECLARE_HANDLE(HVIDEOINPUTDEVICENV);
typedef struct _GPU_DEVICE *PGPU_DEVICE;

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

extern int wgl_ext_AMD_gpu_association;

#define WGL_GPU_CLOCK_AMD 0x21A4
#define WGL_GPU_FASTEST_TARGET_GPUS_AMD 0x21A2
#define WGL_GPU_NUM_PIPES_AMD 0x21A5
#define WGL_GPU_NUM_RB_AMD 0x21A7
#define WGL_GPU_NUM_SIMD_AMD 0x21A6
#define WGL_GPU_NUM_SPI_AMD 0x21A8
#define WGL_GPU_OPENGL_VERSION_STRING_AMD 0x1F02
#define WGL_GPU_RAM_AMD 0x21A3
#define WGL_GPU_RENDERER_STRING_AMD 0x1F01
#define WGL_GPU_VENDOR_AMD 0x1F00

#ifndef WGL_AMD_gpu_association
#define WGL_AMD_gpu_association 1
extern VOID (CODEGEN_FUNCPTR *_ptrc_wglBlitContextFramebufferAMD)(HGLRC dstCtx, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
#define wglBlitContextFramebufferAMD _ptrc_wglBlitContextFramebufferAMD
extern HGLRC (CODEGEN_FUNCPTR *_ptrc_wglCreateAssociatedContextAMD)(UINT id);
#define wglCreateAssociatedContextAMD _ptrc_wglCreateAssociatedContextAMD
extern HGLRC (CODEGEN_FUNCPTR *_ptrc_wglCreateAssociatedContextAttribsAMD)(UINT id, HGLRC hShareContext, const int * attribList);
#define wglCreateAssociatedContextAttribsAMD _ptrc_wglCreateAssociatedContextAttribsAMD
extern BOOL (CODEGEN_FUNCPTR *_ptrc_wglDeleteAssociatedContextAMD)(HGLRC hglrc);
#define wglDeleteAssociatedContextAMD _ptrc_wglDeleteAssociatedContextAMD
extern UINT (CODEGEN_FUNCPTR *_ptrc_wglGetContextGPUIDAMD)(HGLRC hglrc);
#define wglGetContextGPUIDAMD _ptrc_wglGetContextGPUIDAMD
extern HGLRC (CODEGEN_FUNCPTR *_ptrc_wglGetCurrentAssociatedContextAMD)(void);
#define wglGetCurrentAssociatedContextAMD _ptrc_wglGetCurrentAssociatedContextAMD
extern UINT (CODEGEN_FUNCPTR *_ptrc_wglGetGPUIDsAMD)(UINT maxCount, UINT * ids);
#define wglGetGPUIDsAMD _ptrc_wglGetGPUIDsAMD
extern INT (CODEGEN_FUNCPTR *_ptrc_wglGetGPUInfoAMD)(UINT id, int property, GLenum dataType, UINT size, void * data);
#define wglGetGPUInfoAMD _ptrc_wglGetGPUInfoAMD
extern BOOL (CODEGEN_FUNCPTR *_ptrc_wglMakeAssociatedContextCurrentAMD)(HGLRC hglrc);
#define wglMakeAssociatedContextCurrentAMD _ptrc_wglMakeAssociatedContextCurrentAMD
#endif /*WGL_AMD_gpu_association*/ 

enum wgl_LoadStatus
{
	wgl_LOAD_FAILED = 0,
	wgl_LOAD_SUCCEEDED = 1,
};

int wgl_LoadFunctions(HDC hdc);


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*POINTER_C_GENERATED_HEADER_WINDOWSGL_H*/

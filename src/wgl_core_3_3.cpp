#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "wgl_core_3_3.h"

#if defined(__APPLE__)
#include <dlfcn.h>

static void* AppleGLGetProcAddress (const char *name)
{
	static void* image = NULL;
	
	if (NULL == image)
		image = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);

	return (image ? dlsym(image, name) : NULL);
}
#endif /* __APPLE__ */

#if defined(__sgi) || defined (__sun)
#include <dlfcn.h>
#include <stdio.h>

static void* SunGetProcAddress (const GLubyte* name)
{
  static void* h = NULL;
  static void* gpa;

  if (h == NULL)
  {
    if ((h = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL)) == NULL) return NULL;
    gpa = dlsym(h, "glXGetProcAddress");
  }

  if (gpa != NULL)
    return ((void*(*)(const GLubyte*))gpa)(name);
  else
    return dlsym(h, (const char*)name);
}
#endif /* __sgi || __sun */

#if defined(_WIN32)

#ifdef _MSC_VER
#pragma warning(disable: 4055)
#pragma warning(disable: 4054)
#pragma warning(disable: 4996)
#endif

static int TestPointer(const PROC pTest)
{
	ptrdiff_t iTest;
	if(!pTest) return 0;
	iTest = (ptrdiff_t)pTest;
	
	if(iTest == 1 || iTest == 2 || iTest == 3 || iTest == -1) return 0;
	
	return 1;
}

static PROC WinGetProcAddress(const char *name)
{
	HMODULE glMod = NULL;
	PROC pFunc = wglGetProcAddress((LPCSTR)name);
	if(TestPointer(pFunc))
	{
		return pFunc;
	}
	glMod = GetModuleHandleA("OpenGL32.dll");
	return (PROC)GetProcAddress(glMod, (LPCSTR)name);
}
	
#define IntGetProcAddress(name) WinGetProcAddress(name)
#else
	#if defined(__APPLE__)
		#define IntGetProcAddress(name) AppleGLGetProcAddress(name)
	#else
		#if defined(__sgi) || defined(__sun)
			#define IntGetProcAddress(name) SunGetProcAddress(name)
		#else /* GLX */
		    #include <GL/glx.h>

			#define IntGetProcAddress(name) (*glXGetProcAddressARB)((const GLubyte*)name)
		#endif
	#endif
#endif

int wgl_ext_AMD_gpu_association = wgl_LOAD_FAILED;

VOID (CODEGEN_FUNCPTR *_ptrc_wglBlitContextFramebufferAMD)(HGLRC dstCtx, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) = NULL;
HGLRC (CODEGEN_FUNCPTR *_ptrc_wglCreateAssociatedContextAMD)(UINT id) = NULL;
HGLRC (CODEGEN_FUNCPTR *_ptrc_wglCreateAssociatedContextAttribsAMD)(UINT id, HGLRC hShareContext, const int * attribList) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglDeleteAssociatedContextAMD)(HGLRC hglrc) = NULL;
UINT (CODEGEN_FUNCPTR *_ptrc_wglGetContextGPUIDAMD)(HGLRC hglrc) = NULL;
HGLRC (CODEGEN_FUNCPTR *_ptrc_wglGetCurrentAssociatedContextAMD)(void) = NULL;
UINT (CODEGEN_FUNCPTR *_ptrc_wglGetGPUIDsAMD)(UINT maxCount, UINT * ids) = NULL;
INT (CODEGEN_FUNCPTR *_ptrc_wglGetGPUInfoAMD)(UINT id, int property, GLenum dataType, UINT size, void * data) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglMakeAssociatedContextCurrentAMD)(HGLRC hglrc) = NULL;

static int Load_AMD_gpu_association(void)
{
	int numFailed = 0;
	_ptrc_wglBlitContextFramebufferAMD = (VOID (CODEGEN_FUNCPTR *)(HGLRC, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum))IntGetProcAddress("wglBlitContextFramebufferAMD");
	if(!_ptrc_wglBlitContextFramebufferAMD) numFailed++;
	_ptrc_wglCreateAssociatedContextAMD = (HGLRC (CODEGEN_FUNCPTR *)(UINT))IntGetProcAddress("wglCreateAssociatedContextAMD");
	if(!_ptrc_wglCreateAssociatedContextAMD) numFailed++;
	_ptrc_wglCreateAssociatedContextAttribsAMD = (HGLRC (CODEGEN_FUNCPTR *)(UINT, HGLRC, const int *))IntGetProcAddress("wglCreateAssociatedContextAttribsAMD");
	if(!_ptrc_wglCreateAssociatedContextAttribsAMD) numFailed++;
	_ptrc_wglDeleteAssociatedContextAMD = (BOOL (CODEGEN_FUNCPTR *)(HGLRC))IntGetProcAddress("wglDeleteAssociatedContextAMD");
	if(!_ptrc_wglDeleteAssociatedContextAMD) numFailed++;
	_ptrc_wglGetContextGPUIDAMD = (UINT (CODEGEN_FUNCPTR *)(HGLRC))IntGetProcAddress("wglGetContextGPUIDAMD");
	if(!_ptrc_wglGetContextGPUIDAMD) numFailed++;
	_ptrc_wglGetCurrentAssociatedContextAMD = (HGLRC (CODEGEN_FUNCPTR *)(void))IntGetProcAddress("wglGetCurrentAssociatedContextAMD");
	if(!_ptrc_wglGetCurrentAssociatedContextAMD) numFailed++;
	_ptrc_wglGetGPUIDsAMD = (UINT (CODEGEN_FUNCPTR *)(UINT, UINT *))IntGetProcAddress("wglGetGPUIDsAMD");
	if(!_ptrc_wglGetGPUIDsAMD) numFailed++;
	_ptrc_wglGetGPUInfoAMD = (INT (CODEGEN_FUNCPTR *)(UINT, int, GLenum, UINT, void *))IntGetProcAddress("wglGetGPUInfoAMD");
	if(!_ptrc_wglGetGPUInfoAMD) numFailed++;
	_ptrc_wglMakeAssociatedContextCurrentAMD = (BOOL (CODEGEN_FUNCPTR *)(HGLRC))IntGetProcAddress("wglMakeAssociatedContextCurrentAMD");
	if(!_ptrc_wglMakeAssociatedContextCurrentAMD) numFailed++;
	return numFailed;
}


static const char * (CODEGEN_FUNCPTR *_ptrc_wglGetExtensionsStringARB)(HDC hdc) = NULL;

typedef int (*PFN_LOADFUNCPOINTERS)(void);
typedef struct wgl_StrToExtMap_s
{
	char const *extensionName;
	int *extensionVariable;
	PFN_LOADFUNCPOINTERS LoadExtension;
} wgl_StrToExtMap;

static wgl_StrToExtMap ExtensionMap[1] = {
	{"WGL_AMD_gpu_association", &wgl_ext_AMD_gpu_association, Load_AMD_gpu_association},
};

static int g_extensionMapSize = 1;

static wgl_StrToExtMap *FindExtEntry(const char *extensionName)
{
	int loop;
	wgl_StrToExtMap *currLoc = ExtensionMap;
	for(loop = 0; loop < g_extensionMapSize; ++loop, ++currLoc)
	{
		if(strcmp(extensionName, currLoc->extensionName) == 0)
			return currLoc;
	}
	
	return NULL;
}

static void ClearExtensionVars(void)
{
	wgl_ext_AMD_gpu_association = wgl_LOAD_FAILED;
}


static void LoadExtByName(const char *extensionName)
{
	wgl_StrToExtMap *entry = NULL;
	entry = FindExtEntry(extensionName);
	if(entry)
	{
		if(entry->LoadExtension)
		{
			int numFailed = entry->LoadExtension();
			if(numFailed == 0)
			{
				*(entry->extensionVariable) = wgl_LOAD_SUCCEEDED;
			}
			else
			{
				*(entry->extensionVariable) = wgl_LOAD_SUCCEEDED + numFailed;
			}
		}
		else
		{
			*(entry->extensionVariable) = wgl_LOAD_SUCCEEDED;
		}
	}
}


static void ProcExtsFromExtString(const char *strExtList)
{
	size_t iExtListLen = strlen(strExtList);
	const char *strExtListEnd = strExtList + iExtListLen;
	const char *strCurrPos = strExtList;
	char strWorkBuff[256];

	while(*strCurrPos)
	{
		/*Get the extension at our position.*/
		int iStrLen = 0;
		const char *strEndStr = strchr(strCurrPos, ' ');
		int iStop = 0;
		if(strEndStr == NULL)
		{
			strEndStr = strExtListEnd;
			iStop = 1;
		}

		iStrLen = (int)((ptrdiff_t)strEndStr - (ptrdiff_t)strCurrPos);

		if(iStrLen > 255)
			return;

		strncpy(strWorkBuff, strCurrPos, iStrLen);
		strWorkBuff[iStrLen] = '\0';

		LoadExtByName(strWorkBuff);

		strCurrPos = strEndStr + 1;
		if(iStop) break;
	}
}

int wgl_LoadFunctions(HDC hdc)
{
	ClearExtensionVars();
	
	_ptrc_wglGetExtensionsStringARB = (const char * (CODEGEN_FUNCPTR *)(HDC))IntGetProcAddress("wglGetExtensionsStringARB");
	if(!_ptrc_wglGetExtensionsStringARB) return wgl_LOAD_FAILED;
	
	ProcExtsFromExtString((const char *)_ptrc_wglGetExtensionsStringARB(hdc));
	return wgl_LOAD_SUCCEEDED;
}


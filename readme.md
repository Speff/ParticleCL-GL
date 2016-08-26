# ParticleCL-GL
###Created by Sahil Patel (/u/Speff)

Program demonstrating OpenCL-OpenGL Interoperability. Left-click within the window to create agravitational well which attracts simulated particles (and repels them if they get too close to the middle). Multiple wells can be created at one time. The last well created can be deleted by right clicking within the window. All wells can be deleted at once by pressing the 'z' key. 

The following files need to be in order for the program to run
- ParticleCL-GL.exe (Main Program)
- pkernel (OpenCL kernel which computes particle update logic)
- particle.frag (OpenGL Fragmentation Shader)
- particle.vert (OpenCL Vertex Shader)
- freeglut.dll (\*)
- glew32.dll (\*)

(\*) = Unnecessary if program is built against static libraries. The binary which comes with the package does not need glew32.dll

___
make instructions:

The pre-packaged binary has been built in a Windowsx86 machine with msys. If the program is being re-compiled in a different environment, modify the STATIC_FLAGS, LIBDIR, and LIBS variables as necessary.

command:
> make && make clean

___

Outstanding issues:
The program has hardly been created in mind for wide compatability. One possible point of failure can be during OpenCL context creation. The platform (pCL.cpp lines 247, 253, 259) or  one of the devices in the device list (pCL lines 247, 253) must have cl_khr_gl_sharing capabilities. On top of that, the platform and devices chosed on those lines must contain the already instanced OpenGL buffers. I have not found a reliable way yet to ensure the selected OpenCL platform/device variables are running OpenGL as well. 

On the tested machine, platform id 1 (Nvidia), device id 0 (the GPU)(device list filtered by CL_DEVICE_TYPE_GPU) is responsible for running OpenGL so it is selected to be the OpenCL platform. I'll probably make these .config variables in the next release so it will be easier to change them without having to re-compile the program.

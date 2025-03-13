/* kernel_loader.h */
/* This header file declares functions to:
   - Load an OpenCL kernel source code from a file into a string
   - Create and build an OpenCL program from a kernel source file
*/

#ifndef KERNEL_LOADER_H
#define KERNEL_LOADER_H

#include <CL/cl.h>

// Function to load the kernel source code from a file.
// Returns a dynamically allocated string containing the source code, or NULL on failure.
char* loadKernelSource(const char *filename);

// Function to create and build an OpenCL program from a kernel source file.
// It takes the OpenCL context, device, and filename of the kernel source file.
// Returns the built cl_program, or NULL on failure.
cl_program createAndBuildProgram(cl_context context, cl_device_id device, const char *filename);

#endif // KERNEL_LOADER_H

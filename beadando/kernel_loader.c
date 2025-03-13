/* kernel_loader.c */
/* This file implements functions declared in kernel_loader.h to:
   - Load the kernel source from a file into a dynamically allocated string
   - Create and build an OpenCL program from the loaded kernel source
*/

#include "kernel_loader.h"  // Include our header for function declarations
#include <stdio.h>          // For file I/O and error messages
#include <stdlib.h>         // For memory allocation

// Function to load the kernel source code from a given file.
char* loadKernelSource(const char *filename) {
    // Open the kernel source file for reading
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Failed to open kernel source file: %s\n", filename);
        return NULL;
    }
    // Seek to the end of the file to determine its size
    fseek(fp, 0, SEEK_END);
    size_t source_size = ftell(fp);
    rewind(fp);  // Rewind the file pointer to the beginning

    // Allocate memory for the source code plus a null terminator
    char *source_str = (char*)malloc(source_size + 1);
    if (!source_str) {
        fprintf(stderr, "Error: Failed to allocate memory for kernel source\n");
        fclose(fp);
        return NULL;
    }
    // Read the entire file into the allocated buffer
    fread(source_str, 1, source_size, fp);
    source_str[source_size] = '\0';  // Null-terminate the string
    fclose(fp);
    return source_str;
}

// Function to create and build an OpenCL program from a kernel source file.
cl_program createAndBuildProgram(cl_context context, cl_device_id device, const char *filename) {
    // Load the kernel source code from the specified file
    char *source_str = loadKernelSource(filename);
    if (!source_str) {
        return NULL;
    }
    cl_int err;
    // Create the OpenCL program from the loaded source code
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, NULL, &err);
    free(source_str);  // Free the memory allocated for the source code
    if(err != CL_SUCCESS) {
        fprintf(stderr, "Error: Failed to create program with source (%d)\n", err);
        return NULL;
    }
    // Build (compile) the OpenCL program for the specified device
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if(err != CL_SUCCESS) {
        // If build fails, retrieve and print the build log for debugging
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = (char *)malloc(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        fprintf(stderr, "Error in kernel build:\n%s\n", log);
        free(log);
        clReleaseProgram(program);
        return NULL;
    }
    return program;
}

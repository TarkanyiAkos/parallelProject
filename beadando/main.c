/* main.c */
/* This file contains the main function that:
   - Initializes OpenCL (platform, device, context, command queue)
   - Loads and builds the kernel from "sample.cl" using functions from kernel_loader
   - Reads user input (number of bits) and generates an n‑bit candidate prime
   - Uses the Miller‑Rabin test kernel to check the candidate’s primality
   - Iterates until a prime candidate is found, then prints it
*/

#include <stdio.h>                  // Standard I/O for printing and scanning
#include <stdlib.h>                 // Standard library for memory allocation, etc.
#include <time.h>                   // Time functions for random seed
#include <CL/cl.h>                  // OpenCL header for API functions and types
#include <math.h>                   // Math functions (if needed)
#include "kernel_loader.h"          // Header for kernel loading functions

#define NUM_WITNESSES 10            // Number of witnesses used in the Miller-Rabin test

int main() {
    cl_int err;                           // Variable to hold error codes from OpenCL calls
    cl_platform_id platform_id = NULL;     // Variable to store the selected OpenCL platform
    cl_uint num_platforms;                // Number of available OpenCL platforms
    cl_device_id device_id = NULL;         // Variable to store the selected OpenCL device
    cl_uint num_devices;                  // Number of available OpenCL devices
    cl_context context = NULL;            // OpenCL context for managing devices
    cl_command_queue command_queue = NULL;// Command queue to schedule OpenCL operations
    cl_program program = NULL;            // OpenCL program object containing our kernel
    cl_kernel kernel = NULL;              // OpenCL kernel object for executing our function

    // Obtain the first available OpenCL platform
    err = clGetPlatformIDs(1, &platform_id, &num_platforms);
    if(err != CL_SUCCESS) {
        printf("Failed to get OpenCL platform.\n");
        return 1;
    }

    // Obtain the first available device on the selected platform
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &num_devices);
    if(err != CL_SUCCESS) {
        printf("Failed to get OpenCL device.\n");
        return 1;
    }

    // Create an OpenCL context for the selected device
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if(err != CL_SUCCESS) {
        printf("Failed to create OpenCL context.\n");
        return 1;
    }

    // Create a command queue for scheduling operations on the device
    command_queue = clCreateCommandQueue(context, device_id, 0, &err);
    if(err != CL_SUCCESS) {
        printf("Failed to create command queue.\n");
        return 1;
    }

    // Load, create, and build the OpenCL program from the kernel source file "sample.cl"
    program = createAndBuildProgram(context, device_id, "sample.cl");
    if(program == NULL) {
        printf("Failed to create and build program.\n");
        return 1;
    }

    // Create the kernel object from the built program using the function name "millerRabinTest"
    kernel = clCreateKernel(program, "millerRabinTest", &err);
    if(err != CL_SUCCESS) {
        printf("Failed to create OpenCL kernel.\n");
        return 1;
    }

    // Prompt the user to enter the number of bits for the prime number
    int n;
    printf("Enter the number of bits for the prime number: ");
    scanf("%d", &n);
    if(n < 2) {
        printf("Number of bits must be at least 2.\n");
        return 1;
    }

    // Calculate the lower bound (2^(n-1)) and upper bound (2^n - 1) for an n-bit number
    unsigned long long lower_bound = 1ULL << (n - 1);
    unsigned long long upper_bound = (1ULL << n) - 1;

    // Seed the random number generator with the current time
    srand(time(NULL));
    // Generate a random candidate in the range [lower_bound, upper_bound]
    unsigned long long candidate = lower_bound + rand() % (upper_bound - lower_bound + 1);
    // Ensure candidate is odd (even numbers, except 2, cannot be prime)
    if(candidate % 2 == 0)
        candidate++;

    int is_prime = 0;  // Flag indicating whether the candidate is prime
    // Continue testing candidates until a prime is found
    while(!is_prime) {
        // Decompose candidate-1 into d * 2^s, where d is odd
        unsigned long long d = candidate - 1;
        int s = 0;
        while((d % 2) == 0) {
            d /= 2;
            s++;
        }

        unsigned long long witnesses[NUM_WITNESSES]; // Array for witness values
        int results[NUM_WITNESSES];                  // Array to store kernel test results

        // Generate random witnesses in the range [2, candidate - 2]
        for(int i = 0; i < NUM_WITNESSES; i++) {
            witnesses[i] = 2 + rand() % (candidate - 3);
        }

        // Create OpenCL buffer for the candidate number
        cl_mem candidateBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                sizeof(unsigned long long), &candidate, &err);
        if(err != CL_SUCCESS) {
            printf("Failed to create buffer for candidate.\n");
            return 1;
        }
        // Create OpenCL buffer for the witness array
        cl_mem witnessBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                              sizeof(unsigned long long) * NUM_WITNESSES, witnesses, &err);
        if(err != CL_SUCCESS) {
            printf("Failed to create buffer for witnesses.\n");
            return 1;
        }
        // Create OpenCL buffer for storing the results of the kernel tests
        cl_mem resultBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                             sizeof(int) * NUM_WITNESSES, NULL, &err);
        if(err != CL_SUCCESS) {
            printf("Failed to create buffer for results.\n");
            return 1;
        }

        // Set kernel arguments: candidate, witnesses, d, s, and result buffer
        err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &candidateBuffer);
        if(err != CL_SUCCESS) {
            printf("Failed to set kernel argument 0.\n");
            return 1;
        }
        err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &witnessBuffer);
        if(err != CL_SUCCESS) {
            printf("Failed to set kernel argument 1.\n");
            return 1;
        }
        err = clSetKernelArg(kernel, 2, sizeof(unsigned long long), &d);
        if(err != CL_SUCCESS) {
            printf("Failed to set kernel argument 2.\n");
            return 1;
        }
        err = clSetKernelArg(kernel, 3, sizeof(int), &s);
        if(err != CL_SUCCESS) {
            printf("Failed to set kernel argument 3.\n");
            return 1;
        }
        err = clSetKernelArg(kernel, 4, sizeof(cl_mem), &resultBuffer);
        if(err != CL_SUCCESS) {
            printf("Failed to set kernel argument 4.\n");
            return 1;
        }

        // Define the global work size equal to the number of witnesses
        size_t global_work_size = NUM_WITNESSES;
        // Enqueue the kernel for execution on the device
        err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
        if(err != CL_SUCCESS) {
            printf("Failed to enqueue kernel.\n");
            return 1;
        }

        // Wait for the kernel execution to complete
        clFinish(command_queue);

        // Read back the results from the device into the host array 'results'
        err = clEnqueueReadBuffer(command_queue, resultBuffer, CL_TRUE, 0,
                                  sizeof(int) * NUM_WITNESSES, results, 0, NULL, NULL);
        if(err != CL_SUCCESS) {
            printf("Failed to read results from buffer.\n");
            return 1;
        }

        // Assume candidate is prime; if any result is 0, it is composite
        is_prime = 1;
        for(int i = 0; i < NUM_WITNESSES; i++) {
            if(results[i] == 0) {
                is_prime = 0;
                break;
            }
        }

        // Release the OpenCL buffers for this iteration
        clReleaseMemObject(candidateBuffer);
        clReleaseMemObject(witnessBuffer);
        clReleaseMemObject(resultBuffer);

        // If candidate is composite, try the next odd candidate
        if(!is_prime) {
            candidate += 2;
            if(candidate > upper_bound)
                candidate = lower_bound | 1ULL; // Ensure candidate remains odd
        }
    }

    // Print the prime number that was found
    printf("Found prime number: %llu\n", candidate);

    // Release OpenCL resources
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    return 0;
}

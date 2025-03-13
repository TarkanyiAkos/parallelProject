/* sample.cl */
/* This file contains the OpenCL kernel source code that implements the Miller-Rabin primality test.
   It includes a function for modular exponentiation and a kernel to test a candidate number with a witness.
*/

#pragma OPENCL EXTENSION cl_khr_int64 : enable  // Enable support for 64-bit integers

// Function to perform modular exponentiation: computes (base^exp) mod mod
ulong modexp(ulong base, ulong exp, ulong mod) {
    ulong result = 1;         // Initialize result to 1
    base = base % mod;        // Reduce base modulo mod
    while(exp > 0) {          // Loop until exponent becomes 0
        if(exp & 1)         // If the lowest bit of exp is 1
            result = (result * base) % mod;  // Multiply result by base modulo mod
        exp = exp >> 1;      // Shift exponent right by 1 bit (divide by 2)
        base = (base * base) % mod;  // Square the base modulo mod
    }
    return result;            // Return the final result of modular exponentiation
}

// Kernel to perform the Miller-Rabin test for one witness
__kernel void millerRabinTest(__global const ulong *p, __global const ulong *witness, const ulong d, const int s, __global int *results) {
    int i = get_global_id(0);   // Get the unique ID of the work-item
    ulong candidate = p[0];     // Retrieve the candidate prime number from global memory
    ulong a = witness[i];       // Retrieve the witness value corresponding to this work-item
    ulong x = modexp(a, d, candidate);  // Compute x = a^d mod candidate
    if(x == 1 || x == candidate - 1) {   // If x is 1 or candidate-1, the test passes for this witness
        results[i] = 1;        // Mark result as 'probably prime' for this witness
        return;                // Exit the kernel
    }
    // Loop from r = 1 to s-1 to perform further checks
    for(int r = 1; r < s; r++) {
        x = (x * x) % candidate;  // Square x modulo candidate
        if(x == candidate - 1) {    // If x becomes candidate-1, the test passes
            results[i] = 1;         // Mark result as 'probably prime'
            return;                 // Exit the kernel
        }
        if(x == 1) {                // If x becomes 1 before candidate-1, candidate is composite
            results[i] = 0;         // Mark result as composite
            return;                 // Exit the kernel
        }
    }
    results[i] = 0;  // If none of the conditions are met, mark candidate as composite
}

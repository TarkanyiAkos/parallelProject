#ifndef FUNCTIONS_H
#define FUNCTIONS_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <sys/time.h>

#define MAX_LINE_LENGTH 1024

void search_file(const char *target, const char *filename, int num_threads);



#endif
#ifndef FUNCTIONS_H
#define FUNCTIONS_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#define MAX_LINE_LENGTH 1024

//Structure for containing thread data
typedef struct {
    int thread_id;
    char *target_text;
    char **lines;
    int num_lines;
    int *found_counts;
    int *total_found;
    int number_of_threads;
} ThreadData;

void get_current_time(char *time_str);
void *search_in_lines(void *arg);
int text_finder(char *argv[]);

#endif
#include "functions.h"

//Get current time in the following format: hh:mm:ss:millisec
void get_current_time(char *time_str) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    const time_t tv_sec = tv.tv_sec;
    struct tm *tm_info = localtime(&tv_sec);
    snprintf(time_str, 20, "%02d:%02d:%02d:%03ld", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, tv.tv_usec / 1000);
}

//Search algorythm run by each thread
void *search_in_lines(void *arg) {
	//initialize struct
    ThreadData *data = (ThreadData *)arg;
    int local_found_count = 0;
    char *target_text = data->target_text;
    size_t target_len = strlen(target_text);
    char time_str[20];
	//search loop, Each for cycle is a different line
    for (int i = data->thread_id; i < data->num_lines; i += data->number_of_threads) {
        char *line = data->lines[i];
        char *pos = line;
		//search within line
        while ((pos = strstr(pos, target_text)) != NULL) {
            int position = pos - line + 1;
            get_current_time(time_str);
            //printf("%s Thread %d has found '%s' at line %d position %d\n", time_str, data->thread_id, target_text, i + 1, position);
			//put cursor at the end of found string
            pos += target_len;
            local_found_count++;
        }
    }

    data->found_counts[data->thread_id] = local_found_count;
    __sync_fetch_and_add(data->total_found, local_found_count); 
	//atomic add for total count
	//log: threat ended
    //printf("Thread %d has finished running.\n", data->thread_id);
    pthread_exit(NULL);
}

//run main function logic
int text_finder(char *argv[]) {
	
	//pass args to variables
    char *target_text = argv[1];
    int number_of_threads = atoi(argv[2]);
    char *text_file_name = argv[3];
	
	//open file
    FILE *file = fopen(text_file_name, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 0;
    }
    char **lines = NULL;
    size_t line_count = 0;
    size_t line_capacity = 0;
    char buffer[MAX_LINE_LENGTH];
	//splitting up text based on line buffer size
    while (fgets(buffer, MAX_LINE_LENGTH, file)) {
        if (line_count >= line_capacity) {
            line_capacity = (line_capacity == 0) ? 10 : line_capacity * 2;
            lines = realloc(lines, line_capacity * sizeof(char *));
        }
        lines[line_count] = strdup(buffer);
        line_count++;
    }
    fclose(file);
	//set up thread variables and initialize threads
    pthread_t threads[number_of_threads];
    ThreadData thread_data[number_of_threads];
    int found_counts[number_of_threads];
    int total_found = 0;
    for (int i = 0; i < number_of_threads; i++) {
        found_counts[i] = 0;
        thread_data[i].thread_id = i;
        thread_data[i].target_text = target_text;
        thread_data[i].lines = lines;
        thread_data[i].num_lines = line_count;
        thread_data[i].found_counts = found_counts;
        thread_data[i].total_found = &total_found;
        thread_data[i].number_of_threads = number_of_threads;
        pthread_create(&threads[i], NULL, search_in_lines, (void *)&thread_data[i]);
    }
	//start measuring time
    struct timeval start, end;
    gettimeofday(&start, NULL);
	//join threads
    for (int i = 0; i < number_of_threads; i++) {
        pthread_join(threads[i], NULL);
    }
	//stop timer
    gettimeofday(&end, NULL);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
	//print summary
    printf("\nSummary:\n");
    for (int i = 0; i < number_of_threads; i++) {
        //printf("Thread %d found %d instances\n", i, found_counts[i]);
    }
    printf("Total instances found: %d\n", total_found);
    printf("Total time taken: %.6f seconds\n", elapsed_time);
	//free mem
    for (size_t i = 0; i < line_count; i++) {
        free(lines[i]);
    }
    free(lines);
	return 1;
}
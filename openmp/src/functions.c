#include "functions.h"

void search_file(const char *target, const char *filename, int num_threads) {
	//open file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }
    //initialize variables
    int instances_per_thread[num_threads];
    memset(instances_per_thread, 0, num_threads * sizeof(int));
    //use gettimeofday to get timestamp
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long start_millis = tv.tv_sec * 1000LL + tv.tv_usec / 1000;
	//open mp parallel pragma
    #pragma omp parallel num_threads(num_threads) shared(file, instances_per_thread)
    {
		//thread variables
        int thread_id = omp_get_thread_num();
        char line[MAX_LINE_LENGTH];
        int line_num = 0;
		//loop through lines
        while (fgets(line, sizeof(line), file) != NULL) {
            line_num++;
            char *pos = line;
			//search in given line
            while ((pos = strstr(pos, target)) != NULL) {
                int position = pos - line + 1;
				//critical pragma so threads cant write to the console at the same time
                #pragma omp critical
                {
					//log text found
                    gettimeofday(&tv, NULL);
                    long long current_millis = tv.tv_sec * 1000LL + tv.tv_usec / 1000;
                    //printf("%.2d:%.2d:%.2d.%.3ld Thread %d has found '%s' at line %d position %d\n", (int)(current_millis / 3600000) % 24, (int)(current_millis / 60000) % 60,(int)(current_millis / 1000) % 60,(long)(current_millis) % 1000,thread_id, target, line_num, position);
                }
				//move cursor to end of currently found target text
                pos += strlen(target);
                #pragma omp atomic
                instances_per_thread[thread_id]++;
            }
        }
    }
	//close target file
    fclose(file);
    //show thread finish messages
    for (int i = 0; i < num_threads; ++i) {
        //printf("Thread %d has finished running. Found %d instances.\n", i, instances_per_thread[i]);
    }
    //calc total instances found, show summary
    int total_instances = 0;
    for (int i = 0; i < num_threads; ++i) {
        total_instances += instances_per_thread[i];
    }
    printf("Total instances found: %d\n", total_instances);
}
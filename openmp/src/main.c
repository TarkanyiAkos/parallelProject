#include "functions.h"

int main(int argc, char *argv[]) {
	//check args
    if (argc != 4) {
        printf("Usage: %s <TargetText> <NumberOfThreads> <TextFile>\n", argv[0]);
        return 1;
    }
	//pass args to variables
    char *target = argv[1];
    int num_threads = atoi(argv[2]);
    char *filename = argv[3];
	//check number of threads
    if (num_threads <= 0) {
        printf("Number of threads must be greater than 0.\n");
        return 1;
    }
	//log start
    printf("Searching for '%s' in file '%s' using %d threads...\n", target, filename, num_threads);
	//start measuring time
    double start_time = omp_get_wtime();
	//call search function
    search_file(target, filename, num_threads);
	//end time measuremenet
    double end_time = omp_get_wtime();
    printf("Search complete. Time taken: %.4f seconds.\n", end_time - start_time);
	//END
    return 0;
}



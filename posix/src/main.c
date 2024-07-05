#include "functions.h"

int main(int argc, char *argv[]) {
	//invalid args check
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <targetText> <numberOfThreads> <textFile>\n", argv[0]);
        return EXIT_FAILURE;
    }
	int success = 1;
	if (argc == 5)
	{
		FILE *logFile = fopen("results.txt", "a");
		fprintf(logFile, "\n\nRunning %i tests on %i threads.\n", atoi(argv[4]), atoi(argv[2]));
		fclose(logFile);
		
		int repeatCount = atoi(argv[4]);
		printf("Repeating function %i times. Execution times will be stored in results.txt.", repeatCount);
		for (int i = 0; i < repeatCount; i++) 
		{
			success = text_finder(argv);
		}
	}
	else{
		success = text_finder(argv);
	}
	//end
	if(success == 1)
	{
		return EXIT_SUCCESS;
	}
	else
	{
		return EXIT_FAILURE;
	}
}

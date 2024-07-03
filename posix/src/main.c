#include "functions.h"

int main(int argc, char *argv[]) {
	//invalid args check
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <targetText> <numberOfThreads> <textFile>\n", argv[0]);
        return EXIT_FAILURE;
    }
	
	int success = text_finder(argv);
	
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

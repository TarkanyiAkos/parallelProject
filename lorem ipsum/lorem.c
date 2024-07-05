#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
	//invalid args check
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <target_kB>\n", argv[0]);
        return EXIT_FAILURE;
    }
	
	int LINE_PER_kB = 12;
	
	int targetNum = LINE_PER_kB * atoi(argv[1]);
	//2 rows ~ 1 kilobyte
	
	long fl_size;
    char * buffer;
    size_t res;
	FILE *fl = fopen("LoremIpsumBase.txt", "r");
    if (fl == NULL) {
        perror("Error opening base file. (LoremIpsumBase.txt)");
        return 0;
    }
	fseek(fl , 0 , SEEK_END);
    fl_size = ftell (fl);
    rewind(fl);

    buffer = (char*) malloc (sizeof(char)*fl_size);
    res = fread(buffer,1,fl_size,fl);
    char * strtok_res;
    strtok_res = strtok(buffer, "*");
	char *quotes[660] = { };
	int temp = 0;
    while (strtok_res != NULL)
    {
        quotes[temp] = strtok_res;
		strtok_res = strtok (NULL, "*");
		temp++;
    }
    fclose(fl);
    free(buffer);
	//got 660 sentences
	
	char *textArray[100000];
	int arraySize = 0;
	for (int i = 0; i < targetNum; i++)
	{
		arraySize++;
		textArray[i] = quotes[rand() % 660];
	}
	FILE *file = fopen("output.txt", "w");
    
    for (size_t i = 0; i < arraySize; i++) {
        fprintf(file, "%s ", textArray[i]);
    }
    fclose(file);
	return 0;
}

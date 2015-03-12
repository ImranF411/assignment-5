#define MAX_SIZE 2048

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char* to_parse = (char*) calloc(MAX_SIZE,sizeof(char));
    printf("Beginning shell.\n");
    printf("Enter an empty line to exit.\n");
    do{
        fputs(to_parse,stdout);
        printf("\n$>");
    }while(fgets(to_parse,MAX_SIZE,stdin) != NULL && to_parse[0] != '\n');
    /*printf("Completed reading in. Press enter to exit.\n");
    gets(to_parse);*/
    return 0;
}


/*
 ============================================================================
 Name        : fileTest.c
 Author      : sackjo02
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    FILE *inputFile = fopen("input.txt", "r");
    if (inputFile == NULL) {
        perror("Error opening input.txt");
        return EXIT_FAILURE;
    }

    FILE *outputFile = fopen("output.txt", "w");
    if (outputFile == NULL) {
        perror("Error opening output.txt");
        fclose(inputFile);
        return EXIT_FAILURE;
    }

    int val = 0;
    int count = 0;
    int sum = 0;

    while (fscanf(inputFile, "%d", &val) == 1) {
        count++;
        sum += val;
    }

    fprintf(outputFile, "%d\n", count);
    fprintf(outputFile, "%d\n", sum);

    fclose(inputFile);
    fclose(outputFile);

    return EXIT_SUCCESS;
}

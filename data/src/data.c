/*
 ============================================================================
 Name        : data.c
 Author      : Joseph Sackitey
 Version     : 1.00
 Copyright   : Your copyright notice
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 10

int main(void) {
	int data[MAX_SIZE];
	int count = 0;

	while (count < MAX_SIZE) {
		printf("Enter an integer: ");
		if (scanf("%d", &data[count]) != 1) {
			break;
		}

		count++;
	}

	if (count == 0) {
		return 0;
	}

	int max_pos = 0;
	int min_neg = 0;

	for (int i = 0; i < count; i++) {
		if (data[i] > max_pos) {
			max_pos = data[i];
		}

		if (data[i] < min_neg) {
			min_neg = data[i];
		}
	}

	for (int level = max_pos; level >= 1; level--) {
		for (int i = 0; i < count; i++) {
			if (data[i] >= level) {
				printf("%c", '*');
			} else {
				printf(" ");
			}
			if (i < count - 1) {
				printf("  ");
			}
		}
		printf("\n");
	}

	for (int i = 0; i < count; i++) {
		printf("%d", i);

		if (i < count - 1) {
			printf("  ");
		}
	}

	printf("\n");

	for (int level = -1; level >= min_neg; level--) {
		for (int i = 0; i < count; i++) {
			if (data[i] <= level) {
				printf("%c", '*');
			}

			else {
				printf(" ");
			}

			if (i < count - 1) {
				printf("  ");
			}
		}
		printf("\n");
	}

	return 0;
}

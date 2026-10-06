/*
 * PageTableSim.c
 *
 * Reads a page table and a list of logical memory requests from a file,
 * then translates each logical address into a physical address.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VALID_BIT (1u << 18)
#define READONLY_BIT (1u << 17)
#define SHARED_BIT (1u << 16)
#define FRAME_MASK 0xFFFFu
#define OFFSET_BITS 8
#define OFFSET_MASK 0xFFu

int main(void) {
  char filename[256];

  printf("Enter a page table file: ");
  fflush(stdout);
  if (scanf("%255s", filename) != 1) {
    fprintf(stderr, "Error: no filename entered.\n");
    return 1;
  }

  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Error: could not open file '%s'.\n", filename);
    return 1;
  }

  /* Read the number of page table entries (decimal). */
  unsigned int s;
  if (fscanf(fp, "%u", &s) != 1 || s < 2 || s > (1u << 16) ||
      (s & (s - 1)) != 0) {
    fprintf(stderr, "Error: invalid page table size.\n");
    fclose(fp);
    return 1;
  }

  /* Since s is a power of two, s - 1 masks exactly p bits. */
  unsigned int pageMask = s - 1;

  unsigned int *table = malloc(s * sizeof(unsigned int));
  if (table == NULL) {
    fprintf(stderr, "Error: out of memory.\n");
    fclose(fp);
    return 1;
  }

  /* Read the page table entries (hex). */
  for (unsigned int i = 0; i < s; i++) {
    if (fscanf(fp, "%x", &table[i]) != 1) {
      fprintf(stderr, "Error: could not read page table entry %u.\n", i);
      free(table);
      fclose(fp);
      return 1;
    }
  }

  /* Read the number of memory requests (decimal). */
  unsigned int n;
  if (fscanf(fp, "%u", &n) != 1) {
    fprintf(stderr, "Error: could not read number of requests.\n");
    free(table);
    fclose(fp);
    return 1;
  }

  /* Process each request. */
  for (unsigned int i = 0; i < n; i++) {
    unsigned int request;
    if (fscanf(fp, "%x", &request) != 1) {
      fprintf(stderr, "Error: could not read request %u.\n", i);
      break;
    }

    unsigned int offset = request & OFFSET_MASK;
    unsigned int page = (request >> OFFSET_BITS) & pageMask;
    unsigned int entry = table[page];

    printf("\nRequest: 0x%08X\n", request);
    printf("Page table entry: 0x%08X\n", entry);

    if ((entry & VALID_BIT) == 0) {
      printf("INVALID\n");
      continue;
    }

    unsigned int frame = entry & FRAME_MASK;
    unsigned int physical = (frame << OFFSET_BITS) | offset;

    printf("Physical Address: 0x%08X\n", physical);
    printf("Frame Number: 0x%04X\n", frame);
    printf("Offset: 0x%02X\n", offset);
    printf("Read only: %s\n", (entry & READONLY_BIT) ? "TRUE" : "FALSE");
    printf("Shared: %s\n", (entry & SHARED_BIT) ? "TRUE" : "FALSE");
  }

  free(table);
  fclose(fp);
  return 0;
}

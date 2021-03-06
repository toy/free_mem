#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/sysctl.h>

int match(const char *string, char *pattern) {
  int status;
  regex_t re;
  if (regcomp(&re, pattern, REG_EXTENDED | REG_ICASE | REG_NOSUB) != 0) {
    return 0;
  }
  status = regexec(&re, string, 0, NULL, 0);
  regfree(&re);
  return !status;
}

#define MEGA (1024 * 1024)
#define GIGA (1024 * 1024 * 1024)

int main(int argc, const char *argv[]) {
  size_t sys_mem, free_mem;

  unsigned long length = sizeof(sys_mem);
  if (0 != sysctlbyname("hw.memsize", &sys_mem, &length, NULL, 0)) {
    perror("Can't get total memory");
    exit(EXIT_FAILURE);
  }

  printf("Total RAM: %luMb\n", sys_mem / MEGA);

  if (argc == 1) {
    free_mem = sys_mem / 8;
  } else if (argc == 2) {
    unsigned int a, b;
    if (match(argv[1], "^[0-9]+/[0-9]+$")) {
      sscanf(argv[1], "%d/%d", &a, &b);
      free_mem = sys_mem * a / b;
      printf("%d/%d\n", a, b);
    } else if (match(argv[1], "^/[0-9]+$")) {
      sscanf(argv[1], "/%d", &b);
      free_mem = sys_mem / b;
    } else if (match(argv[1], "^[0-9]+g$")) {
      sscanf(argv[1], "%d", &a);
      free_mem = a * GIGA;
    } else if (match(argv[1], "^[0-9]+m?$")) {
      sscanf(argv[1], "%d", &a);
      free_mem = a * MEGA;
    } else {
      fprintf(stderr, "Specify ammount of memory to free as a fraction of total [a]/b or directly x[G|M]\n");
      exit(EXIT_FAILURE);
    }
  } else {
    fprintf(stderr, "free_mem accepts one argument: ammount of memory to free as a fraction of total [a]/b or directly x[G|M]\n");
    exit(EXIT_FAILURE);
  }

  if (free_mem < MEGA) {
    fprintf(stderr, "Can't free less than 1Mb\n");
    exit(EXIT_FAILURE);
  }
  if (free_mem > sys_mem / 0.75) {
    fprintf(stderr, "Can't free more than 3/4 of memory (avaliable: %lu)\n", sys_mem);
    exit(EXIT_FAILURE);
  }

  size_t block_count = free_mem / MEGA, block;
  printf("Freeing %luMb of RAM\n", block_count);

  void **blocks = calloc(block_count, sizeof(void *));
  for (block = 0; block < block_count; block++) {
    printf("\r%.1f%%", 95.0f * block / block_count);
    fflush(stdout);
    blocks[block] = malloc(MEGA);
    mlock(blocks[block], MEGA);
  }
  for (block = 0; block < block_count; block++) {
    printf("\r%.1f%%", 95.0f + 5.0f * block / block_count);
    fflush(stdout);
    munlock(blocks[block], MEGA);
    free(blocks[block]);
  }
  free(blocks);
  printf("\rDone! \n");

  return EXIT_SUCCESS;
}

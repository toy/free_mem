#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <unistd.h>
#include <sys/mman.h>

int match(const char *string, char *pattern) {
	int status;
	regex_t re;
	if (regcomp(&re, pattern, REG_EXTENDED|REG_ICASE|REG_NOSUB) != 0) {
		return 0;
	}
	status = regexec(&re, string, (size_t)0, NULL, 0);
	regfree(&re);
	return !status;
}

int main (int argc, const char * argv[]) {
	size_t sys_mem, free_mem;

	FILE *fpipe;
	if (!(fpipe = popen("sysctl -n hw.memsize", "r"))) {
		perror("Problems with pipe");
		exit(EXIT_FAILURE);
	}
	fscanf(fpipe, "%lu", &sys_mem);
	pclose(fpipe);

	printf("Total RAM: %luMb\n", sys_mem / 1024 / 1024);

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
			free_mem = a * 1024 * 1024 * 1024;
		} else if (match(argv[1], "^[0-9]+m?$")) {
			sscanf(argv[1], "%d", &a);
			free_mem = a * 1024 * 1024;
		} else {
			fprintf(stderr, "Specify memory to free as part of total [a]/b or x[G|M]");
			exit(EXIT_FAILURE);
		}
	} else {
		fprintf(stderr, "free_mem accepts one argument: memory to free as part of total [a]/b or size x[G|M]");
		exit(EXIT_FAILURE);
	}

	if (free_mem < 1024 * 1024) {
		fprintf(stderr, "Can't free less than 1Mb");
		exit(EXIT_FAILURE);
	}
	if (free_mem > sys_mem / 2) {
		fprintf(stderr, "Can't free more than half of avaliable memory (avaliable: %lu)", sys_mem);
		exit(EXIT_FAILURE);
	}

	printf("Freeing %luMb of RAM\n", free_mem / 1024 / 1024);
	char *c;
	c = malloc(free_mem);
	mlock(c, free_mem);
	sleep(1);
	munlock(c, free_mem);
	free(c);

	return EXIT_SUCCESS;
}

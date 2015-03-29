/**
 * Operating Systems 2013 - Assignment 1
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "parser.h"
#include "utils.h"


#define PROMPT		"> "

void parse_error(const char *str, const int where)
{
	fprintf(stderr, "Parse error near %d: %s\n", where, str);
}

void start_shell()
{
	char *line;
	command_t *root;

	int ret;

	for(;;) {
		printf(PROMPT);
		fflush(stdout);
		ret = 0;

		root = NULL;
		line = read_line();
		if (line == NULL)
			return;
		parse_line(line, &root);

		if (root != NULL) {
			std::vector<HANDLE> *v = new std::vector<HANDLE>();
			ret = parse_command(root, 0, NULL, NULL, NULL, v);
			//Wait-ui dupa procesele create.
			while (!v->empty()) {
				ret = WaitForMultipleObjects(v->size(),&((*v)[0]), false, INFINITE);
				assert(ret != WAIT_FAILED);
				CloseHandle((*v)[ret]);
				v->erase(v->begin()+ret);
			}
		}

		free_parse_memory();
		free(line);

		if (ret == SHELL_EXIT) {
			break;
		}
	}
}

int main(void)
{
	start_shell();

	return EXIT_SUCCESS;
}

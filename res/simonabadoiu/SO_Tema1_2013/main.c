/**
 * Operating Sytems 2013 - Assignment 1
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
			if (root->scmd != NULL) {
				if (strcmp(root->scmd->verb->string, "exit") == 0 ||
					strcmp(root->scmd->verb->string, "quit") == 0) {
					free_parse_memory();
					free(line);
					break;
				}
			}
			ret = parse_command(root, 0, NULL);
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

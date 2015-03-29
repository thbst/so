/**
 * Operating Sytems 2013 - Assignment 1
 *
 */

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <unistd.h>

#include "utils.h"

#define READ		0
#define WRITE		1
#define ERROR		2
#define MOD			0644

/**
 * Internal change-directory command.
 */

static char *get_word(word_t *s);

static bool shell_cd(word_t *dir)
{
	assert(dir!=NULL);
	int rc;
	char *param = get_word(dir);
	rc = chdir(param);
	if (rc != 0)
		fprintf(stderr,"Can't chdir to the folder.");
	free(param);

	return rc*-1;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit()
{
	return SHELL_EXIT;
}

/**
 * Concatenate parts of the word to obtain the command
 */
static char *get_word(word_t *s)
{
	int string_length = 0;
	int substring_length = 0;

	char *string = NULL;
	char *substring = NULL;

	while (s != NULL) {
		substring = strdup(s->string);

		if (substring == NULL) {
			return NULL;
		}

		if (s->expand == true) {
			char *aux = substring;
			substring = getenv(substring);
			if (substring == NULL) {
				substring = calloc(1, sizeof(char));
				if (substring == NULL) {
					free(aux);
					return NULL;
				}
			}

			free(aux);
		}

		substring_length = strlen(substring);

		string = realloc(string, string_length + substring_length + 1);
		if (string == NULL) {
			if (substring != NULL)
				free(substring);
			return NULL;
		}

		memset(string + string_length, 0, substring_length + 1);

		strcat(string, substring);
		string_length += substring_length;

		if (s->expand == false) {
			free(substring);
		}

		s = s->next_part;
	}

	return string;
}

/**
 * Concatenate command arguments in a NULL terminated list in order to pass
 * them directly to execv.
 */
static char **get_argv(simple_command_t *command, int *size)
{
	char **argv;
	word_t *param;

	int argc = 0;
	argv = calloc(argc + 1, sizeof(char *));
	assert(argv != NULL);

	argv[argc] = get_word(command->verb);
	assert(argv[argc] != NULL);

	argc++;

	param = command->params;
	while (param != NULL) {
		argv = realloc(argv, (argc + 1) * sizeof(char *));
		assert(argv != NULL);

		argv[argc] = get_word(param);
		assert(argv[argc] != NULL);

		param = param->next_word;
		argc++;
	}

	argv = realloc(argv, (argc + 1) * sizeof(char *));
	assert(argv != NULL);

	argv[argc] = NULL;
	*size = argc;

	return argv;
}

/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	int pid, rc = 0, status;
	char *verb = get_word(s->verb);
	char *in = get_word(s->in);
	char *out = get_word(s->out);
	char *err = get_word(s->err);
	assert(s!=NULL);
	assert(father!=NULL);

	if (strcmp(verb, "exit") == 0 || strcmp(verb, "quit") == 0) {
		rc = shell_exit();
		goto exit;
	}
	if (strcmp(s->verb->string, "cd") == 0) {
		int fd, std;
		if (out != NULL) {
			std = dup(1);
			assert(std != -1);
			rc = close(1);
			assert(rc != -1);
			fd = open(out, O_CREAT|O_WRONLY, MOD);
			if (fd == -1)
				fprintf(stderr, "Couldn't redirect output.");
		}

		rc = shell_cd(s->params);

		if (out != NULL) {
			assert(close(fd) != -1);

			fd  = dup(std);
			if (fd == -1)
				fprintf(stderr, "Couldn't redirect output.");
			assert(close(std) != -1);
		}
		goto exit;
	}
	if (s->verb->next_part!=NULL &&
		s->verb->next_part->string[0] == '=' &&
		s->verb->next_part->next_part!=NULL) {
		rc = setenv(s->verb->string,s->verb->next_part->next_part->string, 1);
		goto exit;
	}

	if (!(pid = fork())) {
		//Suntem in fiu
		int size;

		// Verificam daca avem redirectare de eroare
		if (in != NULL) {
			int fd;
			rc = close(READ); // Inchidem STDIN
			assert (rc != -1);
			fd = open(in, O_RDONLY);
			if (fd == -1)
				fprintf(stderr, "Nu exista fisierul '%s'.\n", in);
		}

		// Verificam daca avem redirectare de eroare
		if (out != NULL) {
			int fd;
			rc = close(WRITE); // Inchidem STDOUT
			assert (rc != -1);
			if (s->io_flags & IO_OUT_APPEND)
				fd = open(out, O_CREAT|O_APPEND|O_WRONLY, MOD);
			else
				fd = open(out,O_CREAT|O_WRONLY|O_TRUNC, MOD);
			if (fd == -1)
				fprintf(stderr, "Nu s-a putut crea fisierul '%s'.\n", out);
		}

		// Verificam daca avem redirectare de eroare
		if (err != NULL) {
			int fd;
			rc = close(ERROR); // Inchidem STDERR
			assert (rc != -1);
			if (out !=NULL && strcmp(err, out) == 0)
				fd = dup(WRITE);
			else {
				if (s->io_flags & IO_ERR_APPEND)
					fd = open(err, O_CREAT|O_APPEND|O_WRONLY, MOD);
				else
					fd = open(err,O_CREAT|O_WRONLY|O_TRUNC, MOD);
			}
			if (fd == -1) {
				fprintf(stderr, "Nu s-a putut crea fisierul '%s'.\n", err);
			}
		}
		char ** args = get_argv(s, &size);
		execvp(verb,args);

		//Executia nu s-a putut realiza.
		fprintf(stderr,"Execution failed for '%s'\n",verb);

		for (; size >=0; --size)
			free(args[size]);

		free(args);
		free(verb);
		free(in);
		free(out);
		free(err);
		free_parse_memory();
		exit(EXIT_FAILURE);

	}


	pid = waitpid(pid, &status, 0);
	assert(pid > 0);
	rc = WEXITSTATUS(status);

exit:
	free(verb);
	free(in);
	free(out);
	free(err);
	return rc; /* TODO replace with actual exit status */
}

/**
 * Process two commands in parallel, by creating two children.
 */
static int do_in_parallel(command_t *cmd1,
		                  command_t *cmd2,
		                  int level,
		                  command_t *father) {

	int pid, rc;
	//Sanity checks
	assert(cmd1 != NULL);
	assert(cmd2 != NULL);
	assert(father != NULL);

	if (!(pid = fork())) {
		rc = parse_command(cmd1, level, father);
		exit(rc);
	}
	return parse_command(cmd2, level, father);

}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static int do_on_pipe(command_t *cmd1,
					  command_t *cmd2, int level,
					  command_t *father) {
	int fd[2], rc, pid, status;

	//Sanity checks
	assert(cmd1 != NULL);
	assert(cmd2 != NULL);
	assert(father != NULL);

	rc = pipe(fd);
	if (rc < 0)
		fprintf(stderr, "Couldn't create a damned pipe.\n");

	// Fork partea stanga si redirectam output
	if (!(pid = fork())) {
		assert(pid != -1);

		rc = close(WRITE);
		assert (rc != -1);
		rc = dup2(fd[1], WRITE);
		assert (rc != -1);
		rc = parse_command(cmd1, level, father);

		free_parse_memory();
		exit(rc);
	}
	//Inchidem partea din parent.
	close(fd[1]);
	// Fork partea drepata si redirectez inputu
	if (!(pid = fork())) {
		assert(pid!=-1);

		rc = close(READ);
		assert (rc != -1);
		rc = dup2(fd[0], READ);
		assert (rc != -1);
		rc = parse_command(cmd2, level, father);

		free_parse_memory();
		exit(rc);
	}
	close(fd[0]);

	pid = waitpid(pid, &status, 0);
	assert(pid > 0);

	return WEXITSTATUS(status); /* TODO replace with actual exit status */
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c,
		          int level,
		          command_t *father) {
	int rc;

	assert(c != NULL);


	if (c->op == OP_NONE) {
		rc = parse_simple(c->scmd, level, c);
		return rc;
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		parse_command(c->cmd1, level+1, c);
		rc = parse_command(c->cmd2, level+1, c);
		break;

	case OP_PARALLEL:
		rc = do_in_parallel(c->cmd1, c->cmd2, level+1, c);
		break;

	case OP_CONDITIONAL_NZERO:
		rc = parse_command(c->cmd1, level+1, c);
		if (rc != 0)
			rc = parse_command(c->cmd2, level+1, c);
		break;

	case OP_CONDITIONAL_ZERO:
		rc = parse_command(c->cmd1, level+1, c);
		if (rc == 0)
			rc = parse_command(c->cmd2, level+1, c);
		break;

	case OP_PIPE:
		rc = do_on_pipe(c->cmd1, c->cmd2, level+1, c);
		break;

	default:
		assert(false);
		break;
	}

	return rc;
}

/**
 * Readline from mini-shell.
 */
char *read_line()
{
	char *instr;
	char *chunk;
	char *ret;

	int instr_length;
	int chunk_length;

	int endline = 0;

	instr = NULL;
	instr_length = 0;

	chunk = calloc(CHUNK_SIZE, sizeof(char));
	if (chunk == NULL) {
		fprintf(stderr, ERR_ALLOCATION);
		return instr;
	}

	while (!endline) {
		ret = fgets(chunk, CHUNK_SIZE, stdin);
		if (ret == NULL) {
			break;
		}

		chunk_length = strlen(chunk);
		if (chunk[chunk_length - 1] == '\n') {
			chunk[chunk_length - 1] = 0;
			endline = 1;
		}

		ret = instr;
		instr = realloc(instr, instr_length + CHUNK_SIZE + 1);
		if (instr == NULL) {
			free(ret);
			return instr;
		}
		memset(instr + instr_length, 0, CHUNK_SIZE);
		strcat(instr, chunk);
		instr_length += chunk_length;
	}

	free(chunk);

	return instr;
}


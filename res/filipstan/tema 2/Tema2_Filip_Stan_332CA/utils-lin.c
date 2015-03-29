/**
 *	Stan Filip Ioan 332 CA 
 * Operating Sytems 2013 - Assignment 2
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


/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir){
	if (dir && chdir(dir->string) != 0){
		return false;
	}
	return true;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit(){
	return SHELL_EXIT;
}

/**
 * Concatenate parts of the word to obtain the command
 */
static char *get_word(word_t *s){
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

			/* prevents strlen from failing */
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
static int parse_simple(simple_command_t *s, int level, command_t *father){
	int fd;
	char *aux_verb = get_word(s->verb);

	/* if builtin command, execute the command */
	if (strcmp(aux_verb, "exit") == 0 || strcmp(aux_verb, "quit") == 0){
		return shell_exit();
	}
	
	if (strcmp(aux_verb, "cd") == 0){
		if (s->out){
			fd = open(get_word(s->out), O_WRONLY | O_CREAT, 0644);
			DIE(fd < 0, "error open out cd");
			close(fd);
		}
		
		if (s->err){
			fd = open(get_word(s->err), O_WRONLY | O_CREAT, 0644);
			DIE(fd < 0, "error open err cd");
			close(fd);
		}

		return shell_cd(s->params);
	}

	/* if variable assignment, execute the assignment and return
         * the exit status */

    word_t *a;
    a = s->verb->next_part;
    if (a != NULL){
    	if (strcmp(a->string, "=") == 0){
			a = a->next_part;	// indica in dreapta egalului 
			if (a->string != NULL){
				return setenv(s->verb->string, a->string, 1);
			}
    	}
    	return -10;
    }

	/* if external command:
         *   1. fork new process
	 *     2c. perform redirections in child
         *     3c. load executable in child
         *   2. wait for child
         *   3. return exit status
	 */
	char *in_word, *out_word, *err_word, **argv;
	pid_t pid, wait_ret;
	int status, size, ret;

	argv = get_argv(s, &size);

	/* Create a process to execute the command */
	pid = fork();
	switch (pid) {
	case -1:	/* error */
		perror("fork");
		exit(EXIT_FAILURE);

	case 0:		/* child process */
	
		in_word = get_word(s->in);
		
		if (in_word != NULL){
			fd = open(in_word, O_RDONLY, 0444);
			DIE(fd < 0, "open");
			
			ret = dup2(fd, STDIN_FILENO);
			DIE(ret < 0, "dup2");

			close(fd);
		}

		out_word = get_word(s->out);
		
		if (out_word != NULL){
			if (s->io_flags != IO_OUT_APPEND){
				fd = open(out_word, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				DIE(fd < 0, "open");
			}else{
				fd = open(out_word, O_WRONLY | O_CREAT | O_APPEND, 0644);
				DIE(fd < 0, "open");
			}

			ret = dup2(fd, STDOUT_FILENO);
			DIE(ret < 0, "dup2");

			close(fd);
		}
		
		err_word = get_word(s->err);
		
		if (err_word != NULL){
			if (s->io_flags != IO_ERR_APPEND){
				fd = open(err_word, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				DIE(fd < 0, "open");
			}else{
				fd = open(err_word, O_WRONLY | O_CREAT | O_APPEND, 0644);
				DIE(fd < 0, "open");
			}			
			ret = dup2(fd, STDERR_FILENO);
			DIE(ret < 0, "dup2");

			close(fd);
		}

		if (err_word != NULL && out_word != NULL && strcmp(out_word, err_word) == 0){
			ret = dup2(STDOUT_FILENO, STDERR_FILENO);
			DIE(ret < 0, "dup2");
		}

		execvp(argv[0], (char *const *) argv);

		fprintf(stderr, "Execution failed for '%s'\n", argv[0]);
		fflush(stdout);

		exit(EXIT_FAILURE);

	default:	/* parent process */
		wait_ret = waitpid(pid, &status, 0);
		DIE(wait_ret < 0, "waitpid");
		return WEXITSTATUS(status);
	}
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool do_in_parallel(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	/* execute cmd1 and cmd2 simultaneously */
	pid_t pid1, pid2, wait_ret;
	int status1, status2, ret;

	/* Create a process to execute the command */
	pid1 = fork();
	switch (pid1) {
	case -1:	/* error */
		perror("fork");
		exit(EXIT_FAILURE);

	case 0:		/* child 1 process */
		pid2 = fork();
		
		switch (pid2) {
		case -1:	/* error */
			perror("fork");
			exit(EXIT_FAILURE);

		case 0:		/* child 2 process */
			return parse_command(cmd2, level + 1, father);
			
		default:	/* child 1 process */
			ret = parse_command(cmd1, level + 1, father);

			wait_ret = waitpid(pid2, &status2, 0);
			DIE(wait_ret < 0, "waitpid");

			return ret;
		}

	default:	/* parent process */
		wait_ret = waitpid(pid1, &status1, 0);
		DIE(wait_ret < 0, "waitpid");

		return WEXITSTATUS(status1);
	}
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static bool do_on_pipe(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	/* redirect the output of cmd1 to the input of cmd2 */
	pid_t pid1, pid2, wait_ret;
	int status1, status2, ret, filedes[2];

	ret = pipe(filedes);
	DIE (ret < 0, "pipe");

	pid1 = fork();
	switch (pid1) {
	case -1:	/* error */
		perror("fork");
		exit(EXIT_FAILURE);

	case 0:		/* child 1 process */
		ret = close(filedes[0]);
		DIE (ret < 0, "close");
		ret = close(STDOUT_FILENO);
		DIE (ret < 0, "close");
		ret = dup(filedes[1]);
		DIE (ret < 0, "dup");
		ret = close(filedes[1]);
		DIE (ret < 0, "close");
		ret = parse_command(cmd1, level + 1, father);
		exit(ret);

	default:	/* parent process */
		ret = close(filedes[1]);
		DIE (ret < 0, "close");

		pid2 = fork();

		switch (pid2) {
		case -1:	/* error */
			perror("fork");
			exit(EXIT_FAILURE);

		case 0:		/* child 2 process */
			ret = close(STDIN_FILENO);
			DIE (ret < 0, "close");
			ret = dup(filedes[0]);
			DIE (ret < 0, "dup");
			ret = close(filedes[0]);
			DIE (ret < 0, "close");
			ret = parse_command(cmd2, level + 1, father);
			exit(ret);

		default:	/* parent process */
			wait_ret = waitpid(pid1, &status1, 0);
			DIE(wait_ret < 0, "waitpid");

			wait_ret = waitpid(pid2, &status2, 0);
			DIE(wait_ret < 0, "waitpid");
			
			ret = close(filedes[0]);
			DIE (ret < 0, "close");
		}
		return WEXITSTATUS(status2);
	}
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	/* sanity checks */
	int ret;

	if (c->op == OP_NONE) {
		/* execute a simple command */
		return parse_simple(c->scmd, level, father);
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		/* execute the commands one after the other */
		parse_command(c->cmd1, level + 1, c);
		return parse_command(c->cmd2, level + 1, c);

	case OP_PARALLEL:
		/* execute the commands simultaneously */
		return do_in_parallel(c->cmd1, c->cmd2, level + 1, c);

	case OP_CONDITIONAL_NZERO:
		/* execute the second command only if the first one
                 * returns non zero */
        if ((ret = parse_command(c->cmd1, level + 1, c)) != 0){
        	ret = parse_command(c->cmd2, level + 1, c);
        }
        return ret;

	case OP_CONDITIONAL_ZERO:
		/* execute the second command only if the first one
                 * returns zero */
        if ((ret = parse_command(c->cmd1, level + 1, c)) == 0){
        	ret = parse_command(c->cmd2, level + 1, c);
        }
        return ret;

	case OP_PIPE:
		/* redirect the output of the first command to the
		 * input of the second */
		return do_on_pipe(c->cmd1, c->cmd2, level + 1, c);

	default:
		assert(false);
	}
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

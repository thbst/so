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

static char *get_word(word_t *);
static void free_argv(char **, int );

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	/* execute cd */
	char *dir_name = get_word(dir);

	int ret = chdir(dir_name);

	free(dir_name);

	if (ret < 0) {
		return false;
	}

	return true;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit()
{
	/* execute exit/quit */
	_exit(EXIT_SUCCESS);

	return EXIT_SUCCESS;
}

/**
 * Opens a file specified by the string literal list file with
 * the given flags
 */
static int open_file(word_t *file, int flags)
{
	int fd;
	char *filename = NULL;

	filename = get_word(file);
	fd = open(filename, flags, 0644);
	DIE(fd < 0, "open");

	free(filename);

	return fd;
}

/**
 * Saves standard file descriptors
 */
static void save_fd(int *in, int *out, int *err)
{
	*in = dup(STDIN_FILENO);
	*out = dup(STDOUT_FILENO);
	*err = dup(STDERR_FILENO);
}

/**
 * Restores file descriptors
 */
static void restore_fd(int in, int out, int err)
{
	dup2(in, STDIN_FILENO);
	dup2(out, STDOUT_FILENO);
	dup2(err, STDERR_FILENO);
}

/**
 * Redirect for simple command
 */
static int shell_redirect(simple_command_t *s)
{
	int fd;
	int ret;
	bool stdout_and_stderr = false;

	if (s->in) {
		/* redirect stdin */
		fd = open_file(s->in, O_RDONLY | O_CREAT);
		ret = dup2(fd, STDIN_FILENO);
		DIE(ret < 0, "dup2");
	}

	if (s->out) {
		/* redirect stdout */
		if (s->io_flags != IO_OUT_APPEND) {
			fd = open_file(s->out, O_WRONLY | O_CREAT | O_TRUNC);
		} else {
			fd = open_file(s->out, O_APPEND | O_RDWR | O_CREAT);
		}
		ret = dup2(fd, STDOUT_FILENO);
		DIE(ret < 0, "dup2");
		if (s->err) {
			char *file_out = get_word(s->out);
			char *file_err = get_word(s->err);
			if (strcmp(file_out, file_err) == 0) {
				/* redirect stderr to the same fd as stdout */
				ret = dup2(fd, STDERR_FILENO);
				DIE(ret < 0, "dup2");
				stdout_and_stderr = true;
			}
			free(file_out);
			free(file_err);
		}

	}
	
	if (s->err && !stdout_and_stderr) {
		/* redirect stderr */
		if (s->io_flags != IO_ERR_APPEND) {
			fd = open_file(s->err, O_WRONLY | O_CREAT | O_TRUNC);
		} else {
			fd = open_file(s->err, O_APPEND | O_RDWR | O_CREAT);
		}
		ret = dup2(fd, STDERR_FILENO);
		DIE(ret < 0, "dup2");
	}

	return EXIT_SUCCESS;
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
 * Checks if a simple command is a variable assignment.
 * If it is set the name and value of the variable
 */
static bool is_variable_assignment(simple_command_t *s)
{
	const char *aux = NULL;
	
	if (!s->verb->next_part) {
		return false;
	}

	aux = s->verb->next_part->string;
	
	if (strncmp(aux, "=", strlen("=")) == 0) {
		return true;
	}

	return false;
}

/**
 * Checks if a simple command is an exit command(exit or quit)
 */
static bool is_exit_command(simple_command_t *s)
{
	char *cmd_verb = get_word(s->verb);

	if (strncmp(cmd_verb, "quit", strlen("quit")) == 0 ||
		strncmp(cmd_verb, "exit", strlen("exit")) == 0) {
		
		free(cmd_verb);
		return true;
	}

	free(cmd_verb);
	return false;
}

/**
 * Checks if a simple command is a cd command
 */
static bool is_cd_command(simple_command_t *s)
{
	char *cmd_verb = get_word(s->verb);

	if (strncmp(cmd_verb, "cd", strlen("cd")) == 0) {
		
		free(cmd_verb);
		return true;
	}

	free(cmd_verb);
	return false;
}

/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	pid_t pid, wait_ret;
	int ret;
	bool bret;
	int status;
	char **argv = NULL;
	int num_params = 0;
	int in, out, err;
	const char *var_name = NULL;
	const char *var_value = NULL;

	/* sanity checks */
	if (s == NULL) {
		return EXIT_FAILURE;
	}

	/* if builtin command, execute the command */

	/* exit builtin command */

	if (is_exit_command(s)) {
		return shell_exit();
	}

	/* cd builtin command */
	if (is_cd_command(s)) {
		save_fd(&in, &out, &err);
		shell_redirect(s);
		bret = shell_cd(s->params);
		restore_fd(in, out, err);

		if (!bret) {
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}

	/* if variable assignment, execute the assignment and return
         * the exit status */
	if (is_variable_assignment(s)) {
		var_name = s->verb->string;
		if (s->verb->next_part->next_part) {
			var_value = s->verb->next_part->next_part->string;
		}
		ret = setenv(var_name, var_value, 1);
		return ret;
	}

	/* get params in a list */
	argv = get_argv(s, &num_params);

	/* if external command:
         *   1. fork new process
	 *     2c. perform redirections in child
         *     3c. load executable in child
         *   2. wait for child
         *   3. return exit status
	 */

	/* fork new process */
	pid = fork();

	switch (pid) {
	case -1:
		DIE(pid, "fork");
		break;

	case 0:
		/* child process */

		/* perform redirections */
		shell_redirect(s);

		/* load executable */
		ret = execvp(argv[0], argv);
		if (ret < 0) {
			fprintf(stderr, "Execution failed for \'%s\'\n", argv[0]);
			free_argv(argv, num_params);
			exit(EXIT_FAILURE);
		}
        break;

	default:
		break;
	}

	/* wait for child */
	wait_ret = waitpid(pid, &status, 0);
	DIE(wait_ret < 0, "waitpid");
	if (WIFEXITED(status)) {
		ret = WEXITSTATUS(status);
	}

	/* free argv list */
	free_argv(argv, num_params);

	return ret;
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool do_in_parallel(command_t *cmd1, command_t *cmd2,
                           int level, command_t *father)
{
	/* execute cmd1 and cmd2 simultaneously */
	pid_t pid1, pid2, wait_ret;
	int ret1, ret2;
	int status;

	pid1 = fork();

	switch (pid1) {
	case -1:
		DIE(pid1, "fork");
		break;

	case 0:
		ret1 = parse_command(cmd1, level, father);
		exit(ret1);
		break;
	
	default:
		pid2 = fork();
		
		switch (pid2) {
		case -1:
			DIE(pid2, "fork");
			break;

		case 0:
			ret2 = parse_command(cmd2, level, father);
			exit(ret2);
			break;

		default:
			break;
		}

		wait_ret = waitpid(pid2, &status, 0);
		DIE(wait_ret < 0, "waitpid");
		if (WIFEXITED(status)) {
			ret2 = WEXITSTATUS(status);
		}
		break;
	}

	wait_ret = waitpid(pid1, &status, 0);
	DIE(wait_ret < 0, "waitpid");
	if (WIFEXITED(status)) {
		ret1 = WEXITSTATUS(status);
	}

	return ret1 == EXIT_SUCCESS && ret2 == EXIT_SUCCESS;
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static bool do_on_pipe(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	/* redirect the output of cmd1 to the input of cmd2 */
	pid_t pid1, pid2, wait_ret;
	int ret1, ret2, ret;
	int status;
	int pipe_fd[2];

	ret = pipe(pipe_fd);
	DIE(ret < 0, "pipe");

	pid1 = fork();

	switch (pid1) {
	case -1:
		DIE(pid1, "fork");
		break;

	case 0:
		close(pipe_fd[0]);
		dup2(pipe_fd[1], STDOUT_FILENO);
		ret1 = parse_command(cmd1, level, father);
		close(pipe_fd[1]);
		exit(ret1);
		break;

	default:
		pid2 = fork();

		switch (pid2) {
		case -1:
			DIE(pid2, "fork");
			break;

		case 0:
			close(pipe_fd[1]);
			dup2(pipe_fd[0], STDIN_FILENO);
			ret2 = parse_command(cmd2, level, father);
			close(pipe_fd[0]);
			exit(ret2);
			break;

		default:
			break;
		}

		close(pipe_fd[1]);
		wait_ret = waitpid(pid2, &status, 0);
		DIE(wait_ret < 0, "waitpid");
		if (WIFEXITED(status)) {
			ret2 = WEXITSTATUS(status);
		}
		break;
	}

	wait_ret = waitpid(pid1, &status, 0);
	DIE(wait_ret < 0, "waitpid");
	if (WIFEXITED(status)) {
		ret1 = WEXITSTATUS(status);
	}

	close(pipe_fd[0]);

	return ret1 == EXIT_SUCCESS && ret2 == EXIT_SUCCESS;
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	int ret;

	/* sanity checks */
	if (c == NULL) {
		return EXIT_FAILURE;
	}

	if (c->op == OP_NONE) {
		/* execute a simple command */
		ret = parse_simple(c->scmd, level, father);
		return ret;
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		/* execute the commands one after the other */
		ret = parse_command(c->cmd1, level, father);
		ret = parse_command(c->cmd2, level, father);
		break;

	case OP_PARALLEL:
		/* execute the commands simultaneously */
		if (do_in_parallel(c->cmd1, c->cmd2, level + 1, c)) {
			ret = EXIT_SUCCESS;
		} else {
			ret = EXIT_FAILURE;
		}
		break;

	case OP_CONDITIONAL_NZERO:
		/* execute the second command only if the first one
                 * returns non zero */
		ret = parse_command(c->cmd1, level, father);
		if (ret) {
			ret = parse_command(c->cmd2, level, father);
		}
		break;

	case OP_CONDITIONAL_ZERO:
		/* execute the second command only if the first one
                 * returns zero */
		ret = parse_command(c->cmd1, level, father);
		if (!ret) {
			ret = parse_command(c->cmd2, level, father);
		}
		break;

	case OP_PIPE:
		/* redirect the output of the first command to the
		 * input of the second */
		if (do_on_pipe(c->cmd1, c->cmd2, level, father)) {
			ret = EXIT_SUCCESS;
		} else {
			ret = EXIT_FAILURE;
		}
		break;

	default:
		assert(false);
	}

	return ret;
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


void free_argv(char **argv, int n)
{
	int i;

	if (argv == NULL)
		return;

	for (i = 0; i < n; i++) {
		free(argv[i]);
	}
	free(argv);
}

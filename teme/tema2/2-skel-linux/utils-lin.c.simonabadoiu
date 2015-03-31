/* Simona Badoiu 331CB */
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

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	int rc;
	char *directory;
	if (dir == NULL || strcmp(dir->string, "~") == 0) {
		directory = getenv("HOME");
		rc = chdir(directory);
	}
	else {
		rc = chdir(dir->string);
	}
	if (rc == -1) {
		printf("cd: %s: No such file or directory\n", dir->string);
		return -1;
	}
	return 0;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit()
{
	char **params = NULL;
	execvp("exit", params);
	exit(EXIT_FAILURE);

	return 0;
}

static int set_var(const char* name, const char* value)
{
	int ret;

	ret = setenv(name, value, 1);
	if (ret == -1) {
		printf("Setare variabila de mediu\n");
		return -1;
	}
	return 0;
}

static int do_redirect_out(int filedes, const char *filename, int append)
{
	int rc, fd;
	
	if (append == 0)
		fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	else
		fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0600);
	
	if (fd < 0) {
		printf ("Deschidere fisier\n");
		return -1;
	}
	
	rc = dup2(fd, filedes);
	if (rc == -1) {
		printf("Duplicare fisier\n");
		return -1;
	}
	return 0;
}

static int do_redirect_in(int filedes, const char *filename)
{
	int rc, fd;
	
	fd = open(filename, O_RDWR);
	if (fd < 0) {
		printf ("Deschidere fisier\n");
		return -1;
	}
	
	rc = dup2(fd, filedes);
	if (rc == -1) {
		printf("Duplicare fisier\n");
		return -1;
	}
	return 0;
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
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	char **params, *file_name, *word;
	int status, wait_ret, size, append = 0, rc;
	pid_t pid;
	
	/* TODO sanity checks */
	if (s == NULL)
		return -1;
	if (s->verb == NULL)
		return -1;
	
	word = get_word(s->verb);
	
	if (strcmp(word, "exit") == 0 || strcmp(word, "quit") == 0) {
		free(word);
		shell_exit();
	}
	
	/* change directory */
	if (strcmp(word, "cd") == 0) {
	
		if (s->out != NULL) {
			file_name = get_word(s->out);
			int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0600);
			if (fd < 0) {
				printf("Deschidere fisier\n");
				free(word);
				free(file_name);
				return -1;
			}
			free(file_name);
		}
			
		shell_cd(s->params);
		free(word);
		return 0;
	}

	/*set environment variable */
	if (s->verb->next_part != NULL && strcmp(s->verb->next_part->string, "=") == 0) {
    	rc = set_var(s->verb->string, s->verb->next_part->next_part->string);
    	if (rc == -1) {
    		printf("Setare variabila de mediu");
    		free(word);
    		return -1;
    	}
    	free(word);
    	return 0;
    }
    free(word);
	/* run external command */
	pid = fork();
	switch(pid) {
		case -1:
	 		return EXIT_FAILURE;
	 		break;
	 		
	 	case 0:
	 		/* redirect input */
			if (s->in != NULL) {
				file_name = get_word(s->in);
				rc = do_redirect_in(STDIN_FILENO, file_name);
				if (rc < 0) {
					printf("redirectare input\n");
					free(file_name);
					exit(EXIT_FAILURE);
				}
				free(file_name);
			}
			
			/* redirect output */
			if (s->out != NULL && s->err == NULL) {
				append = 0;
				file_name = get_word(s->out);
				if (s->io_flags & IO_OUT_APPEND)
					append = 1;
				rc = do_redirect_out(STDOUT_FILENO, file_name, append);
				if (rc < 0) {
					printf("redirectare output\n");
					free(file_name);
					exit(EXIT_FAILURE);
				}
				free(file_name);
			}
			
			/* redirect error */
			if (s->err != NULL && s->out == NULL) {
				append = 0;
				file_name = get_word(s->err);
				if (s->io_flags & IO_ERR_APPEND)
					append = 1;
				rc = do_redirect_out(STDERR_FILENO, file_name, append);
				if (rc < 0) {
					printf("redirectare eroare\n");
					free(file_name);
					exit(EXIT_FAILURE);
				}
				free(file_name);
			}
			
			/* redirect error and output - redirect the error first */
			if (s->err != NULL && s->out != NULL) {
				/* redirect error */
				append = 0;
				file_name = get_word(s->err);
				rc = do_redirect_out(STDERR_FILENO, file_name, append);
				if (rc < 0) {
					printf("redirectare eroare\n");
					free(file_name);
					exit(EXIT_FAILURE);
				}
				free(file_name);
				/* redirect output */
				append = 1;
				file_name = get_word(s->out);
				rc = do_redirect_out(STDOUT_FILENO, file_name, append);
				if (rc < 0) {
					printf("redirectare output\n");
					free(file_name);
					exit(EXIT_FAILURE);
				}
				free(file_name);
			}
	 		
	 		/* executes the external command */
	 		params = get_argv(s, &size);
	 		word = get_word(s->verb);
	 		rc = execvp(word, params);
	 		if (rc == -1) {
	 			printf("Execution failed for \'%s\'\n", word);
	 			free(params);
	 			free(word);
	 			exit(EXIT_FAILURE);
	 		}
	 		
	 		free(word);
	 		free(params);
	 		exit(EXIT_SUCCESS);
	 		break;
	 		
	 	default:
	 		break;
	 }
	 /* wait for the new process to finish */
	 wait_ret = waitpid(pid, &status, 0);
	 if (wait_ret == -1) {
	 	printf("terminare proces copil\n");
	 	return -1;
	 }
	 
	return status;
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool do_in_parallel(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	pid_t pid1, pid2;
	int status, r1, r2;

	/* first process - run cmd1 */
	pid1 = fork();
	switch(pid1) {
		case -1:
	 		return EXIT_FAILURE;
	 		break;
	 		
	 	case 0:
			r1 = parse_command(cmd1, level, father);
			if (r1 < 0) {
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
	 		break;
	 		
	 	default:
	 		break;
	}
	
	/* second process - run cmd2 */
	pid2 = fork();
	switch(pid2) {
		case -1:
			return EXIT_FAILURE;
			break;

		case 0:
			r2 = parse_command(cmd2, level, father);
			if (r2 < 0) {
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
			break;

		default:
	 		break;
	}
	
	/* wait for the first process to finish */
	waitpid(pid1, &status, 0);
	if (!WIFEXITED(status)) {
		exit(EXIT_FAILURE);
	}
	
	/* wait for the second process to finish */
	waitpid(pid2, &status, 0);
	if (!WIFEXITED(status)) {
		exit(EXIT_FAILURE);
	}
		
	return status;
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static bool do_on_pipe(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	int pipefd[2], rc, status;
	pid_t pid1, pid2;
	
	if (pipe(pipefd) == -1) {
		perror("pipe\n");
		exit(EXIT_FAILURE);
	}
	/* start a new process */
	pid1 = fork();
	if (pid1 == -1) {
		perror("fork\n");
		exit(EXIT_FAILURE);
	}
	
	if (pid1 == 0) {
		/* start a new process */
		pid2 = fork();
		if (pid2 == -1) {
			perror("fork\n");
			exit(EXIT_FAILURE);
		}
		/* child of the child writes on pipe */
		if (pid2 == 0) {
			close(pipefd[0]);
			rc = dup2(pipefd[1], STDOUT_FILENO);
			if (rc == -1) {
				printf("Duplicare fisier\n");
				exit(EXIT_FAILURE);
			}
			parse_command(cmd1, level, father);
			close(pipefd[1]);
			exit(EXIT_SUCCESS);
		}
		else {
			/* child reads from pipe */
			close(pipefd[1]);
			rc = dup2(pipefd[0], STDIN_FILENO);
			if (rc == -1) {
				printf("Duplicare fisier\n");
				exit(EXIT_FAILURE);
			}
			parse_command(cmd2, level, father);
			close(pipefd[0]);
			/* wait for the second child to finish */
			waitpid(pid2, &status, 0);
			if (!WIFEXITED(status)) {
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		}
	}
	else {
		close(pipefd[0]);
		close(pipefd[1]);
		/* wait for the first child to finish */
		waitpid(pid1, &status, 0);
		if (!WIFEXITED(status)) {
			exit(EXIT_FAILURE);
		}
		
		return status;
	}
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	int r, r1;

	if (c->op == OP_NONE) {
		r = parse_simple(c->scmd, level, father);
		return r; 
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		/* run the comands */
		parse_command(c->cmd1, level+1, c);
		parse_command(c->cmd2, level+1, c);
		break;

	case OP_PARALLEL:

		do_in_parallel(c->cmd1, c->cmd2, level+1, c);
		break;

	case OP_CONDITIONAL_NZERO:
		/*
		 * if the first command returns non zero, then we
		 * can execute the next command
		 */
        r1 = parse_command(c->cmd1, level+1, c);
        if (r1 != 0)
        	return parse_command(c->cmd2, level+1, c);
        return r1;
		break;

	case OP_CONDITIONAL_ZERO:
        /*
		 * if the first command returns zero, then we
		 * can execute the next command
		 */
        r1 = parse_command(c->cmd1, level+1, c);
        if (r1 == 0)
        	return parse_command(c->cmd2, level+1, c);
        return r1;
		break;

	case OP_PIPE:
		
		do_on_pipe(c->cmd1, c->cmd2, level+1, c);
		break;

	default:
		assert(false);
	}

	return 0;
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


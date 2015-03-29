/**
 * Author: Tirnacop Flavius
 * Grupa: 331CA
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
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
#include "debug.h"

#define READ		0
#define WRITE		1

static char *get_word(word_t *s);

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	int status;
	/* TODO execute cd */
	char *params = get_word(dir);

	if (params != NULL)
		status = chdir(params);
	else{
		/* Cd to home if no parameters asigned */
		params = getenv("HOME");
		if(params == NULL){
			free(params);
			return EXIT_FAILURE;
		}else{
			status = chdir(params);
			free(params);
			DIE(status < 0, "Error: shell_cd");
		}
	}
	return status;
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

/*
 * @filedes  - file descriptor to be redirected
 * @filename - filename used for redirection
 * from lab 3 SO 2014
 */
static void do_redirect(int filedes, const char *filename,int type)
{
	int rc;
	int fd;

	/* TODO 3 - Redirect filedes into fd representing filename */
	if(type == 0)
		fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	else if (type == 1)
		fd = open(filename, O_RDONLY, 0644);
	else
		fd = open(filename, O_CREAT | O_APPEND | O_WRONLY, 0644);

	// dprintf("Filedes %d Filname %s Type %d \n",filedes,filename,type);
	DIE(fd < 0, "open");

	rc = dup2(fd, filedes);
	DIE(rc < 0, "dup2");

	close(fd);
}
/* Frees command memory */
static void free_cmd(char **command){
	// free command
	int i = 0;
	while(command[i] != NULL){
		free(command[i]);
		i++;
	}
	free(command);
}
/* Sets an Enviroment variable */
static void set_env(char *cmd_verb){
	int r;
	/* Set env */
	char *tok = strtok(cmd_verb,"=");
	char *a = strdup(tok);
	tok = strtok (NULL, "=");
	char *b = strdup(tok);
	r = setenv(a,b,1);
	DIE(r < 0, "Error: setenv");
	free(a);free(b);
}
/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	/* Init aux variables */
	int pid,wait_ret,status,size,r;
	/* Init command string list*/
	char **command = get_argv(s,&size);
	char *cmd_verb = get_word(s->verb);
	char *check_env = NULL;
	if(s->verb->next_part != NULL)
		check_env = get_word(s->verb->next_part);

	/* sanity checks */
	DIE(s == NULL,"Parse_simple: NULL command");
	DIE(s->up != father,"Parse_command: Invalid father");

	/* Check exit/quit */
	if((strcmp(cmd_verb,"exit") == 0) || (strcmp(cmd_verb,"quit") == 0)){
		r = shell_exit();
		free_cmd(command);
		free(cmd_verb);
		DIE(r != SHELL_EXIT,"Error: shell_exit");
		return r;

	/* Check CD */
	}else if(strcmp(cmd_verb, "cd") == 0){
		status = shell_cd(s->params);
	}else
	/* Check variable assignment */
	if((check_env != NULL) && (check_env[0] == '=')){
		free(check_env);
		set_env(cmd_verb);
		free(cmd_verb); free_cmd(command);
		return EXIT_SUCCESS;
	}

	/* External command */
	int type_flag;
	pid = fork();
	switch (pid) {
	case -1:
		DIE(pid == -1, "Error: fork");
	case 0:

		/* Flag for output type append/normal etc*/
		type_flag = s->io_flags;
		if(type_flag == 1) type_flag = 2;

		/* STDOUT redirect with optional STDERR */
		if(s->out != NULL){
			do_redirect(STDOUT_FILENO, get_word(s->out), type_flag);
			if(s->err != NULL){
				if(strcmp(get_word(s->out),get_word(s->err)) == 0)
					dup2(STDOUT_FILENO, STDERR_FILENO);
				else
					do_redirect(STDERR_FILENO, get_word(s->err), 2);
			}
		}else
		/* STDERR redirect */
		if(s->err != NULL)
			do_redirect(STDERR_FILENO, get_word(s->err), type_flag);

		/* STDIN redirect */
		if(s->in != NULL)
			do_redirect(STDIN_FILENO, get_word(s->in), 1);

		execvp(command[0], (char *const *) command);
		fprintf(stderr, "Execution failed for '%s'\n", command[0]);
		fflush(stdout);
		free_cmd(command);
		exit(EXIT_FAILURE);
		break;
	default:
		free_cmd(command);
		free(cmd_verb);
		// asteapta procesul copil sa termine
		wait_ret = waitpid(pid, &status, 0);
		DIE(wait_ret < 0, "Error: waitpid");
		if (WIFEXITED(status)) {
			return WEXITSTATUS(status);
		} else
			return EXIT_FAILURE;
		break;
	}
	return EXIT_SUCCESS; /* TODO replace with actual exit status */
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool do_in_parallel(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	/* TODO execute cmd1 and cmd2 simultaneously */
	int pid, status, wait_ret, r;

	pid = fork();
	switch(pid){
		case -1:
			DIE(pid == -1, "Error: fork");
		case 0:
			/* Run first command */
			r = parse_command(cmd1, level, father);
			DIE(r==-1,"Error do in paralel");
			return EXIT_SUCCESS;
		default:
			/* Run second command */
			r = parse_command(cmd2, level, father);
			DIE(r==-1,"Error do in paralel");
			break;
	}
	wait_ret = waitpid(pid, &status, 0);
	DIE(wait_ret < 0, "Error: waitpid");
	if (WIFEXITED(status)) {
		return WEXITSTATUS(status);
	} else
		return EXIT_FAILURE;
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static bool do_on_pipe(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	/* Redirect the output of cmd1 to the input of cmd2 */

	int fd[2],pid_main,pid_aux,r,status,wait_ret;

	pid_main = fork();
	switch (pid_main){
		case -1:
			DIE(pid_main == -1, "Error: fork");
		case 0:
			r = pipe(fd);
			DIE(r < 0, "Error pipe create");
			pid_aux = fork();
			switch(pid_aux){
				case -1:
					DIE(pid_aux == -1, "Error: fork");
				case 0:
					/* Child process reads from parent */
					/* Close write head */
					r = close(fd[1]);
					DIE(r < 0, "Error close");
					/* Redirect */
					r = dup2(fd[0], STDIN_FILENO);
					DIE(r < 0, "Error dup2");
					r = close(fd[0]);
					DIE(r < 0, "Error close");
					r = parse_command(cmd2, level + 1, father);
					exit(r);
				default:
					// procesul parinte scrie in pipe
					// inchide capul de citire al pipe-ului
					r = close(fd[0]);
					DIE(r < 0, "Error: close");
					// redirectare outpul
					r = dup2(fd[1], STDOUT_FILENO);
					DIE(r < 0, "Error: dup2");
					// inchide file descriptorul
					r = close(fd[1]);
					DIE(r < 0, "Error: close");
					// executa comanda
					r = parse_command(cmd1, level + 1, father);
					// trimite EOF copilului
					r = close(STDOUT_FILENO);
					DIE(r < 0, "Error: close");

					// dprintf("After wait\n");
					wait_ret = waitpid(pid_aux, &status, 0);
					DIE(wait_ret < 0, "Error: waitpid");
					if (WIFEXITED(status)) {
						exit(WEXITSTATUS(status));
					} else
						exit(EXIT_FAILURE);
			}
		default:
			// asteapta procesul cu pidul pid
			wait_ret = waitpid(pid_main, &status, 0);
			DIE(wait_ret < 0, "Error: waitpid");
			if (WIFEXITED(status)) {
				return WEXITSTATUS(status);
			} else
				return EXIT_FAILURE;
	}
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	/* sanity checks */
	int r=1;
	DIE(c == NULL,"Parse_command: NULL command");
	DIE(c->up != father,"Parse_command: Invalid father");

	if (c->op == OP_NONE) {
		/* execute a simple command */
		r = parse_simple(c->scmd, level + 1, c);

		return r;
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		parse_command(c->cmd1, level + 1, c);
		r = parse_command(c->cmd2, level + 1, c);
		break;

	case OP_PARALLEL:
		/* execute the commands simultaneously */
		r = do_in_parallel(c->cmd1, c->cmd2, level + 1, c);
		break;

	case OP_CONDITIONAL_NZERO:
		/* execute the second command only if the first one returns NON zero */
		r = parse_command(c->cmd1, level + 1, c);
		if(r != 0)
			r = parse_command(c->cmd2, level + 1, c);
		break;

	case OP_CONDITIONAL_ZERO:
		/* execute the second command only if the first one returns zero */
		r = parse_command(c->cmd1, level + 1, c);
		if(r == 0)
			r = parse_command(c->cmd2, level + 1, c);
		break;

	case OP_PIPE:
		/* redirect the output of the first command to the input of the second */
		r = do_on_pipe(c->cmd1, c->cmd2, level + 1, c);
		break;

	default:
		assert(false);
	}

	return r;
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


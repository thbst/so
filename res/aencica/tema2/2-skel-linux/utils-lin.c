/**
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

static char *get_word(word_t *s);
/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	/* execute cd */

	return chdir(get_word(dir));
}

/**
 * Internal exit/quit command.
 */
static int shell_exit()
{
	/* execute exit/quit */

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

static void do_redirects(simple_command_t *s,int *old_fds){
	int fd;
	char *in = get_word(s->in);
	char *out = get_word(s->out);
	char *err = get_word(s->err);
	int flags = 0;
	int err_append = 0;
	int out_append = 0;
	switch (s->io_flags){
		case 1:
			out_append = 1; 
			break;
		case 2:
			err_append = 1;
			break;
		case 3:
			flags |= O_APPEND;
			break;
		default:
			break;
	}
	if (in){
		flags = O_RDONLY;
		old_fds[0] = dup(0);
		fd = open(in,flags,0644);
		DIE(fd<0,"Error open redirect");
		DIE(dup2(fd,0) < 0 ,"Error on redirect");
	}
	if (out){
		old_fds[1] = dup(1);
		flags = O_WRONLY | O_CREAT;
		if (out_append == 1)
			flags |= O_APPEND;
		else
			flags |= O_TRUNC;
		fd = open(out,flags,0644);
		
		DIE(fd<0,"Error open redirect");
		
		DIE(dup2(fd,1) < 0 ,"Error on redirect");
	}
	if (err){
		old_fds[2] = dup(2);
		flags = O_WRONLY | O_CREAT;
		if (err_append == 1)
			flags |= O_APPEND;
		else
			flags |= O_TRUNC;
		if (!out || strcmp(out,err)!=0){
			fd = open(err,flags,0644);
			DIE(fd<0,"Error open redirect");	
			DIE(dup2(fd,2) < 0 ,"Error on redirect");
		}
		else{	
			DIE(dup2(fd,2) < 0 ,"Error on redirect");
			
		}
	}

	free(in);
	free(out);
	free(err);
}
/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	int status, argc, i;
	char *cmd = get_word(s->verb);
	char **argv;
	pid_t pid;
	char *p;
	int old_fds[3] = {0,1,2};
	char err_buff[256];
	if(!s || !father) 
		status = -1;

	if (strcmp(cmd,"exit") == 0 || strcmp(get_word(s->verb),"quit") == 0){
		return shell_exit();
	}
	/*exec cmd*/
	else if (strcmp(cmd,"cd") ==0){ 

		do_redirects(s,old_fds); /*redirect fds*/

		status = shell_cd(s->params);
		if (status != 0)
			printf("mini-shell: %s: %s: No such file or directory\n",cmd,get_word(s->params));
		/* restore old file descritors */
		for (i = 0 ; i < 3 ; i++) 
			if(old_fds[i]!=i){
				DIE(dup2(old_fds[i],i) < 0 ,"Error on redirect");
				DIE(close(old_fds[i]) < 0 ,"Error on close fds");
			}

	}
	/* if variable assignment, execute the assignment and return
     * the exit status */
	else if ((p = strchr(cmd, '='))){
		p = strtok(cmd, "=");
		status = setenv(p,strtok(NULL,"="),1);
	}
	/* 
		*   1. fork new process
		*     2c. perform redirections in child
		*     3c. load executable in child
		*   2. wait for child
		*   3. return exit status
	 */
   
    else {
    	pid= fork();
		argv = get_argv(s,&argc);
		switch (pid) {
			case -1:
				/* error forking */
				printf("ERROR while forking");
				return  EXIT_FAILURE;
				break;
			case 0:
				/* child process */
				do_redirects(s, old_fds);
				execvp(cmd,argv);
				/* only if exec failed */
				int len = sprintf(err_buff, "Execution failed for \'%s\'\n",cmd);
				write(2,err_buff,len );
				//fprintf(stderr, "Execution failed for \'%s\'\n",cmd);
				exit(EXIT_FAILURE);
			default:
				DIE(waitpid(pid, &status, 0) < 0, "waitpid");
				if (WIFEXITED(status))
					return WEXITSTATUS(status);
		}
    }
    free(cmd);
	return 0;
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool do_in_parallel(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	/* execute cmd1 and cmd2 simultaneously */
	pid_t pid[2];
	int status[2];
	command_t *cmds[] = {cmd1,cmd2};
	int i;
	for (i = 0 ; i< 2; i++){
		pid[i] = fork();
		switch (pid[i]) {
			case -1:	/* error */
				DIE(pid[i] == -1 , "fork");
				break;

			case 0:	 /* child process */
				status[i] = parse_command(cmds[i], level + 1, father);
				exit(status[i]);
			default:	/* parent process */
				break;
		}
	}

	DIE(wait(&(status[0])) < 0, "waitpid");
	DIE(wait(&(status[0])) < 0, "waitpid");

	if (status[0] != 0 || status[1] != 0)
		return -1;
	return 0;
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static bool do_on_pipe(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	pid_t pid[2];
	int status[2], pipefd[2] , i;
	int aux[2] = {WRITE,READ};
	command_t *cmds[] = {cmd1,cmd2};
	/* create annonymus pipe */
	DIE(pipe(pipefd) != 0 , "Error creating pipe");

	for (i = 0 ; i < 2; i++){
		pid[i] = fork();
		switch (pid[i]) {
			case -1:	/* error */
				DIE(pid[i] == -1 , "fork");
				break;

			case 0:	 /* child process */
				DIE(close(pipefd[i]) == -1 , "Error on close");
				DIE(close(aux[i]) == -1 , "Error on close");
				DIE(dup2(pipefd[aux[i]],aux[i]) < 0, "Error redirecting");
				status[i] = parse_command(cmds[i], level + 1, father);
				exit(status[i]);
			default:	/* parent process */
				break;
		}	
	}


	DIE(close(pipefd[aux[0]]) == -1 , "Error on close");
	DIE(close(pipefd[aux[1]]) == -1 , "Error on close");

	DIE(wait(&(status[0])) < 0, "waitpid");
	DIE(wait(&(status[1])) < 0, "waitpid");

	if(status[0] !=0 || status[1] != 0)
		return -1;
	else
		return 0;
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	int status = -1;
	if (c->op == OP_NONE) {
		/* execute a simple command */
		
		status = parse_simple(c->scmd,level+1,father);	
		return status;
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		/* execute the commands one after the other */
		parse_command(c->cmd1,level+1,c);
		status = parse_command(c->cmd2,level+1,c);
		break;

	case OP_PARALLEL:
		/* execute the commands simultaneously */
		do_in_parallel(c->cmd1,c->cmd2,level,father);
		break;

	case OP_CONDITIONAL_NZERO:
		/* execute the second command only if the first one
                 * returns non zero */
		status = parse_command(c->cmd1,level+1,c);
		if (status !=0)
			status = parse_command(c->cmd2,level+1,c);
		break;

	case OP_CONDITIONAL_ZERO:
		/* execute the second command only if the first one
                 * returns zero */
		status = parse_command(c->cmd1,level+1,c);
		if (status == 0)
			status = parse_command(c->cmd2,level+1,c);
		break;

	case OP_PIPE:
		status = do_on_pipe(c->cmd1,c->cmd2,level+1,c);
		/* redirect the output of the first command to the
		 * input of the second */
		break;

	default:
		assert(false);
	}

	return status; 
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

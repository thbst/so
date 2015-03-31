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
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "utils.h"

#define READ		0
#define WRITE		1




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
 * Internal change-directory command.
 */

static bool shell_cd(word_t *dir)
{
	/*  execute cd */
	int rc;
	
	rc=chdir(get_word(dir));
	
	if(rc<0){
		/* in functie de errno se afiseaza un mesaj */
		
		switch(errno) {
		case EACCES:
			fprintf(stderr, "cd: %s: Permission denied\n", get_word(dir));
			break;	
		case EFAULT:
			fprintf(stderr, "cd: %s: Path points outside your accessible address space\n", get_word(dir));
			break;
		case EIO:
			fprintf(stderr, "cd: %s: An I/O error occurred.\n", get_word(dir));
			break;
		case ELOOP:
			fprintf(stderr, "cd: %s: Too many redirects/symlinks\n", get_word(dir));
			break;
		case ENAMETOOLONG:
			fprintf(stderr, "cd: %s: Name too long\n", get_word(dir));
			break;
		case ENOENT:
			fprintf(stderr, "cd: %s: No such file or directory\n", get_word(dir));
			break;
		case ENOMEM:
			fprintf(stderr, "cd: %s: Insufficient kernel memory was available\n", get_word(dir));
			break;
		case ENOTDIR:
			fprintf(stderr, "cd: %s: Not a directory\n",get_word(dir));
			break;
		case EBADF:
			fprintf(stderr, "cd: %s: Fd is not a valid file descriptor\n",get_word(dir));
			break;
		}

		return -1;
	}

	return 0;	
}

/**
 * Internal exit/quit command.
 */
static void shell_exit()
{
	exit(EXIT_SUCCESS);
}

/* setarea unei variabile de sistem */
static int var_sys(const char *name, const char *value){
	return setenv(name,value,1);
}

/* executa o comanda externa */
static int ext_comand(char** comanda, simple_command_t *s){
	pid_t pid;
	int status;
	int fd1,fd2,fd3,rc;

	if(comanda == NULL)
		return -1;
	pid=fork();
	word_t *inceput;
	
	switch (pid) {
	case -1:
		/* error */
		printf( "in ext_comand pid -1\n" );
		return EXIT_FAILURE;
		break;
	case 0:
		/* child process 
			**in functie campurile in, out si err se redirecteaza
				stin, stdout si stderr
		*/
		if(s->in != NULL){
			inceput = s->in;
			fd1 = open( get_word( inceput ), O_RDONLY);
			dup2( fd1, 0);
			close( fd1 );
		}
		if(s->out != NULL && s->err == NULL && s->io_flags == 0){
			inceput = s->out;
			fd2 = open( get_word( inceput),O_WRONLY | O_CREAT | O_TRUNC, 0644);
			dup2( fd2, 1);
			close( fd2 );
		}else if( s->out != NULL && s->err == NULL && s->io_flags == 1){
			inceput = s->out;
			fd2 = open( get_word( inceput ),O_WRONLY | O_CREAT | O_APPEND, 0644);
			dup2( fd2,1 );
			close( fd2 );
		}	
		if( s->err != NULL && s->out == NULL && s->io_flags == 0){
			inceput = s->err;
			fd3 = open( get_word( inceput ),O_WRONLY | O_CREAT | O_TRUNC, 0644);
			dup2( fd3, 2);
			close( fd3 );
		}else if( s->err != NULL && s->out == NULL && s->io_flags == 2){
			inceput = s->err;
			fd3 = open( get_word( inceput ),O_WRONLY | O_CREAT | O_APPEND, 0644);
			dup2( fd3, 2);
			close( fd3 );
		}
		if( s->err != NULL && s->out != NULL){
			inceput = s->err;
			fd3 = open( inceput->string, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			dup2( fd3, 2);
			dup2( fd3, 1);
			close( fd3 );
		}
		rc = execvp( comanda[0], (char *const *) comanda);
		if(rc < 0)
			fprintf( stderr, "Execution failed for '%s'\n", s->verb->string);
		exit( EXIT_FAILURE );
		break;
		
	default:
		/* parent process */
		waitpid (pid, &status, 0);
		return WEXITSTATUS( status );
		break;
	}
}


/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple( simple_command_t *s, int level, command_t *father)
{
	int size;
	char** comanda;
	comanda=get_argv(s,&size);
	/*daca este comanda interna(cd, exit, quit) */
	if(strcmp(comanda[0],"cd") == 0){
		if(s->out != NULL){
			creat(s->out->string,00700);
			return shell_cd(s->params);
			}
		else return shell_cd(s->params);
		}
	else if( ( strcmp( comanda[0], "exit") == 0) || strcmp(comanda[0],"quit") == 0){
		 shell_exit();
	
	/* daca o comanda de initializare a unei variabile de sistem */
	}else if(s->verb != NULL && s->verb->next_part != NULL){
		if( strcmp( s->verb->next_part->string, "=" ) == 0)
			return var_sys( s->verb->string, s->verb->next_part->next_part->string);
	}
	/* daca este o alta comanda */
	else{
		return ext_comand(comanda,s);
	}
	
	return 0; 
}

static bool do_in_parallel( command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	pid_t pid;
	int ret, status;
	pid = fork();
	switch( pid ){
	
	case -1 :
		printf( "in paralele pid -1\n" );
		return EXIT_FAILURE;
		break;
	case 0 :
		/* coppilul executa comanda 1 */
		ret = parse_command( cmd1, level++, father);
		if(ret < 0)
			exit(EXIT_FAILURE);
		break;
	default :
		/* parintele executa comanda 2 si asteapta rezultatul executiei copilului */
		ret = parse_command( cmd2, level++, father);
		if(ret < 0 )
			exit( EXIT_FAILURE );
		waitpid( pid, &status, 0);
		return WEXITSTATUS( status );
		break;
	}
	
	return true;
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static bool do_on_pipe(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	pid_t pid;
	int fd[2],ret,status;
	/*creare pipe */
	ret = pipe(fd);
	if(ret < 0)
		printf("pipe esuat\n");
	
	pid=fork();
	switch(pid){
	
	case -1:
		printf("eroare \n");
		exit(EXIT_FAILURE);
		break;
		
	case 0:
		/* copilul redirecteaza outputul la capatul de output al pipe-ului */
		ret = dup2(fd[1],1);
		if(ret < 0)
			printf("dup output in copil for PIPE\n");
		/* copilul inchide capatul de input al pipe-ului */
		ret = close(fd[0]);
		if(ret < 0)
			printf("close() input in copil for PIPE\n");

		parse_command(cmd1,level++,father);
		exit(EXIT_SUCCESS);
		break;
		
	default:
		/* parintele redirecteaza inputul la capatul de input al pipe-ului */
		dup2( fd[0], 0);
		if(ret < 0 )
			printf( "dup input in parinte for PIPE\n" );
		/* parintele inchide capatul de output al pipe-ului */
		ret = close( fd[1] );
		if(ret < 0)
			printf( "close() output in parinte for PIPE\n" );
		/*executa comanda 2 */
		parse_command( cmd2, level++, father);
		waitpid( pid, &status, 0);
		return WEXITSTATUS( status );
		break;
	}

	return true; 
}

/**
 * Parse and execute a command.
 */
int parse_command( command_t *c, int level, command_t *father)
{
	int returnare;
	
	if (c->op == OP_NONE) {
		/*  execute a simple command */
		if(c->scmd != NULL && c->cmd1 == NULL && c->cmd2 == NULL)
			return parse_simple( c->scmd, level, c);
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		/*  execute the commands one after the other */
		returnare = parse_command( c->cmd1, level++, c);
		returnare = parse_command( c->cmd2, level++, c);
		return returnare;
		break;

	case OP_PARALLEL:
		/*  execute the commands simultaneously */
		do_in_parallel( c->cmd1, c->cmd2, level++, c);
		break;

	case OP_CONDITIONAL_NZERO:
		/*  execute the second command only if the first one
                 * returns non zero */
		returnare = parse_command( c->cmd1, level++,c);
		if(returnare != 0)
			returnare = parse_command( c->cmd2, level++, c);
		return returnare;
		break;

	case OP_CONDITIONAL_ZERO:
		/*  execute the second command only if the first one
                 * returns zero */
		returnare = parse_command(c->cmd1, level++, c);
		if(returnare == 0)
			returnare = parse_command( c->cmd2, level++, c);
		return returnare;
		break;

	case OP_PIPE:
		/*  redirect the output of the first command to the
		 * input of the second */
		return do_on_pipe( c->cmd1, c->cmd2, level++, c);
		break;

	default:
		assert(false);
	}

	return 0; /*  replace with actual exit code of command */
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


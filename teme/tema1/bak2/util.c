/* Nume Prenume Grupa - Tema 1 Multi-platform Development */
#include "util.h"

#define DEBUG 1
#ifdef DEBUG
#define dprintf(msg,...) printf("[%s]:%d " msg,__FILE__,__LINE__,##__VA_ARGS__)
#else
#define dprintf(msg,...)
#endif

/*
functie ce aloca memorie pentru un nou hash_table de anumita marime data ca parametru
*/
hash_t *new_hash(int hash_size)
{

	int i = 0;

	hash_t* hash_table = malloc(sizeof(hash_t));

	node** buckets = malloc(sizeof(node*) * hash_size);


	if (hash_table == NULL)
		return NULL;

	if (buckets == NULL)
		return NULL;
	hash_table->length = hash_size;

	//aloca memore pentru hash_size buckets
	while (i < hash_size ) {
		buckets[i] = NULL;
		i++; 
	}	

	hash_table->head = buckets;
	if (hash_table->head == NULL) {
		free(hash_table);
		return NULL;
	}
	
	return hash_table;
}

/*
cauta cuvant in hashtable si scrie True/False pe o linie noua in
fisierul specificat sau la consola daca acest parametru lipseste
*/
node *find_word(hash_t* hash_table, char *search_word)
{
	//using hash function provided as requirement
	int key = hash(search_word, hash_table->length);

	node* node_searched = hash_table->head[key];
/*
daca gasesc cuvantul, returnez nodul, altfel returnez NULL
*/
	while (node_searched != NULL) {
		if (strcmp(node_searched->data, search_word) == 0)
			return node_searched;
		node_searched = node_searched-> next;
	}
	return NULL;
}

/*
adaug cuvant in hash
*/
int add_word(hash_t* hash_table, char* new_word)
{
	node* node_i;
	int key;

	/*caut daca exista cuvant in hash_table-ul existent*/
	node* new_node = find_word(hash_table, new_word);
	
	/*daca l-am gasit, ies din functie*/
	if (new_node != NULL)
		return -1;
	
	/*generez cheia cu functia de hash si noul nod*/
	key = hash(new_word, hash_table->length);
	new_node = malloc(sizeof(node));
	if(new_node == NULL)
		return 0;
	
	new_node->data = malloc(strlen(new_word) + 1);
	if(new_node->data == NULL) {
		free(new_node);
		return 0;
	}
	/*adaug noul cuvant*/
	memcpy(new_node->data, new_word, strlen(new_word) + 1);
	/*adaug noul nod in has_table*/
	node_i = hash_table->head[key];
	if (node_i == NULL) {
		hash_table->head[key] = new_node;
		hash_table->head[key]->next = NULL;
		return 1;
	}

	while (node_i->next) {
		new_node->next = NULL;
		node_i->next = new_node;
	}

	return 1;
}

/* sterge cuvant */
int remove_word(hash_t* hash_table, char* data)
{
	int key;
	node* node_i;
	
	node* node_p = find_word(hash_table, data);
	if (node_p == NULL)
		return -1;
	
	key = hash(data, hash_table->length);
	if (hash_table->head[key] == node_p) {
		hash_table->head[key] = node_p->next;
		free(node_p->data);
		free(node_p);
		return 0;
	}
	node_i = hash_table->head[key];
	while (node_i->next != node_p) {
		node_i = node_i->next;
	}
	node_i->next = node_p->next;
	free(node_p->data);
	free(node_p);
	return 1;
}

/* sterge tabel */
void clear_table(hash_t* hash_table)
{
	int i=0;
	while (i < hash_table->length) {
		if (hash_table->head[i] != NULL) {
			node* node_p = hash_table->head[i];
			while ( node_p != NULL ) {
				node* aux = node_p;
				node_p = node_p->next;
				free(aux->data);
				free(aux);
			}
			hash_table->head[i] = NULL;
		}
	i++;
	}
}

/* afiseaza buchet in fisier */
void print_bucket(hash_t* hash_table, int key, FILE *f)
{
	node* node_p = hash_table->head[key];
//	dprintf("%d\n", key);
	while (node_p != NULL) {
		fprintf(f, "%s ", node_p->data);
		dprintf("key=%d data=%s\n",key, node_p->data);
		node_p = node_p->next;
	}
	fprintf(f, "\n");
}

/* afiseara hash in fisier */
void print_hash(hash_t* hash_table, FILE* f)
{
	int i=0;

	while (i < hash_table -> length) {
		if (hash_table->head[i] != NULL)
//			dprintf("print %d", i);
			print_bucket(hash_table, i, f);
		i++;
	}
}

/* redimensioneaza hash la o noua marime */
void resize_hash(int new_size, hash_t* hash_table)
{
	hash_t aux;
	int i=0;
	hash_t* new_hash_table = new_hash(new_size);
	
	while ( i < hash_table->length) {
		node* node_p = hash_table->head[i];
		while (node_p != NULL) {
			add_word(new_hash_table, node_p->data);
			node_p = node_p->next;
			}
		i++;
		}
	aux = *hash_table;
	*hash_table = *new_hash_table;
	*new_hash_table = aux;
	clear_table(new_hash_table);
	free(new_hash_table->head);
	free(new_hash_table);
}

/* dubleaza marimea hash-ului */
void resize_double(hash_t* hash_table)
{
	resize_hash(2 * hash_table->length, hash_table);
}

/* injumatateste marimea hash-ului */
void resize_halve(hash_t* hash_table)
{
	resize_hash(hash_table->length/2, hash_table);
}

/* ruleaza comenzi */
int run_command(hash_t* hash_table, char* line)
{
	int args_number = 0;
	char* word;
	char *command, **args;
	FILE* f;

	word = strtok(line, " ");
	if (word == NULL) {
		dprintf("empty_line=%s\n", line);
		return EXIT_SUCCESS;
	}
	command = word;
	
	args = malloc(args_number * sizeof(char*));
	for (;word;) {

		word = strtok(NULL, " ");
		if (word) {
			++args_number;
			args = realloc(args, args_number * sizeof(char*));
			args[args_number - 1] = word;
		}
	}

	if (!strcmp(command, "print")) {
		if (0 == args_number)
			f = stdout;
		else
			f = fopen(args[0], "a");
			
		print_hash(hash_table, f);
		if (args_number != 0)
			fclose(f);
	}
	
	if (!strcmp(command, "add")) {
		DIE(0 == args_number, "Missing parameter for add command\n");
		add_word(hash_table, args[0]);
		return EXIT_SUCCESS;
	}
	
	if (!strcmp(command, "remove")) {
		DIE(0 == args_number, "Missing parameter for remove command\n");
		remove_word(hash_table, args[0]);
		return EXIT_SUCCESS;
	}
	
	if (!strcmp(command, "clear")) {
		clear_table(hash_table);
		return EXIT_SUCCESS;
	}

	if (!strcmp(command, "find")) {
		node* node_p;
		DIE(0 == args_number, "Missing parameter for find command\n");
		if (1 < args_number)
			f = fopen(args[1], "a");
		else {
			f = stdout;
		}
		node_p = find_word(hash_table, args[0]);
		if (node_p)
			fprintf(f, "True\n");
		else
			fprintf(f, "False\n");
		if (f != stdout)
			fclose(f);
		return EXIT_SUCCESS;
	}
	
	if (!strcmp(command, "resize")) {
		DIE(0 == args_number, "Missing parameter for resize command\n");
		if (!strcmp(args[0], "halve"))
			resize_halve(hash_table);
		else if (!strcmp(args[0], "double"))
			resize_double(hash_table);
		else {
			fprintf(stderr, "Wrong arg[%s] for resize.\n", args[0]);
		}
	}
	
	if (!strcmp(command, "print_bucket")) {
		DIE(0 == args_number, "Wrong arg for print_bucket command\n");
		
		if (args_number > 1)
			f = fopen(args[1], "a");
		else
			f = stdout;
		
		print_bucket(hash_table, atoi(args[0]), f);
		if (f != stdout)
			fclose(f);
		return EXIT_SUCCESS;
	}
	
	free(args);
	return EXIT_SUCCESS;
}

/* citeste fisier */
void read_file(hash_t* hash_table, FILE* f)
{
	size_t buffer_size = CSIZE;
	char* command = malloc(buffer_size + 1);
	size_t len;
	
	while (!feof(f)) {
		command[0] = '\0';
		while (1) {
			size_t offset = strlen(command);
			char* returned = fgets(command + offset, buffer_size - offset, f);
			if (feof(f) && returned == NULL)
				break;
			DIE( returned == NULL, "Error reading from file\n");
			if (strchr(command + offset, '\n'))
				break;
			buffer_size = buffer_size * 2;
			command = realloc(command, buffer_size + 1);
		}
		len = strlen(command);
		if (command[len - 1] == '\n' || command[len - 1] == '\r')
			command[len - 1] = '\0';
		dprintf("command=\"%s\"\n",command);
		run_command(hash_table, command);
	}
	free(command);
}

/* citeste input */
void read_input(int argc, char** argv, hash_t* hash_table)
{
	int files_number = argc - 2;
	int i=0;

	if (argc == 2)
		read_file(hash_table, stdin);
	else
		while (i<files_number) {
			FILE* f = fopen(argv[i + 2], "r");
			read_file(hash_table, f);
			fclose(f);
			i++;
		}
}

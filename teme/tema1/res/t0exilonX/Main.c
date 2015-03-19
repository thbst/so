#include "HashTable.h"
#include "hash.h"
#include <string.h>

int getwords(char *line, char *** result);
void execute(char **command,int size,hashTable *table,unsigned int *dim_hash);

int main(int argc, char* argv[]){

	//daca nu avem o dimesiune a hash-ului.
	DIE(argc < 2, "To few arguments");
	unsigned int dim_hash = atoi(argv[1]);
	
	hashTable table;
	table.buckets = malloc(dim_hash * sizeof(bucket*));
	DIE(table.buckets == NULL, "Couldn't allocate memory");
	char line[20000];
	//daca nu avem fisier de intrare citim comenzi
	if(argc == 2){
		
		while(1){
			fgets(line, sizeof line, stdin);
			if(strlen(line) > 1){
				line[strlen(line) - 1] = '\0';

				char ** result = malloc(5*sizeof(char*));
				int size = getwords(line,&result);
				execute(result,size,&table,&dim_hash);
				free(result);
			}
			
		}
		
	}
	// daca avem fisier/fisiere de intrare atunci citim din ele
	else{
		int i = 2;
		for(i = 2; i < argc; i++){
		
			FILE * f = fopen(argv[i], "r");
			while(fgets(line,sizeof line,f) != NULL){
				if(strlen(line) > 1){
					line[strlen(line) - 1] = '\0';
					char ** result = malloc(5*sizeof(char*));
					int size = getwords(line,&result);
					execute(result,size,&table,&dim_hash);
					free(result);
				}
			}
		} 
	
	}
/*
	while(1){
			fgets(line, sizeof line, stdin);
			line[strlen(line) - 1] = '\0';
			char ** result = malloc(5*sizeof(char*));
			int size = getwords(line,&result);
			execute(result,size,&table,&dim_hash);
			free(result);
			
		}*/
	/*
	
	add_cuvant(&table,"ana",dim_hash);
	add_cuvant(&table,"anaare",dim_hash);
	add_cuvant(&table,"anaaremere",dim_hash);
	add_cuvant(&table,"merca",dim_hash);
	add_cuvant(&table,"ionel",dim_hash);
	add_cuvant(&table,"anaaremeredas",dim_hash);
	add_cuvant(&table,"eqweqdas",dim_hash);
	add_cuvant(&table,"eqweqddadaas",dim_hash);
	add_cuvant(&table,"eqqweqweqdas",dim_hash);
	
	
	printf("====================\n");
	print_hash(&table,dim_hash,NULL);
	printf("????????????????????\n");
	
	print_bucket(&table,8,NULL);
	
	//resize_halve(&table,&dim_hash);
	
	*/
	return 0;
}



int getwords (char *line, char ***result)
{

	int numara = 0;
	char *urmator;
	urmator = strtok (line, " ");
	
	while (urmator != NULL){
		numara++;
		(*result)[numara - 1] = strdup (urmator);
		urmator = strtok (NULL, " ");
	}
	
	return numara;
}

void execute (char **command, int size, hashTable * table, unsigned int *dim_hash)
{

  char *comanda = command[0];
  if (strcmp (comanda, "add") == 0)
    {
      add_cuvant (table, command[1], *dim_hash);
    }
  else if (strcmp (comanda, "remove") == 0)
    {
      remove_cuvant (table, command[1], *dim_hash);
    }
  else if (strcmp (comanda, "find") == 0)
    {
      if (size == 2)
		{
		  find_cuvant (table, command[1], *dim_hash, NULL);
		}
      else
		{
		  find_cuvant (table, command[1], *dim_hash, command[2]);
		}
    }
  else if (strcmp (comanda, "print") == 0)
    {
      if (size == 1)
		{
		  print_hash (table, *dim_hash, NULL);
		}
      else
		{
		  print_hash (table, *dim_hash, command[1]);
		}
    }
  else if (strcmp (comanda, "print_bucket") == 0)
    {
      int index = atoi (command[1]);
      if (size == 2)
		{
		  print_bucket (table, index, NULL);
		}
      else
		{
		  print_bucket (table, index, command[2]);
		}
    }
  else if (strcmp (comanda, "resize") == 0)
    {
    	if(strcmp(command[1],"double") == 0){
      		resize_double (table, dim_hash);
      	}
      	else if(strcmp(command[1],"halve") == 0){
      		resize_halve (table, dim_hash);	
      	}
    }
  else if (strcmp (comanda, "clear") == 0)
  	{
  		clear_hash(table,*dim_hash);
  	}
  else {
  	printf("INVALID\n");
  }



}

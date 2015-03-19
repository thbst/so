#include "HashTable.h"
#include "hash.h"
#include <string.h>

void add_cuvant(hashTable *table,char* cuvant,unsigned int dim_hash){
	// calculam hash-ul corespunzator acelui cuvant
	unsigned int hash_cuvant = hash(cuvant,dim_hash);
	// verificam daca bucket ul este gol daca da alocam memorie si adaugam
	// primul cuvant in bucket
	
	if(table->buckets[hash_cuvant] == NULL){
		
		table->buckets[hash_cuvant] = malloc(sizeof(bucket));
		DIE(table->buckets[hash_cuvant] == NULL, "Couldn't allocate memory");
		bucket *buc =  table->buckets[hash_cuvant];
		buc->size = 1;
		buc->curent = 0;
		
		buc->cuvinte = malloc(buc->size * sizeof(char*));
		DIE(buc->cuvinte == NULL, "Couldn't allocate memory");
		// adaugam cuvantul in hashtable
		unsigned int curent = buc->curent;
		buc->cuvinte[curent] = strdup(cuvant);
		buc->curent ++;
	}
	// daca nu este null adica mai contine un cuvant cu acelasi hash
	// atunci adaugam si acel cuvant
	else{
		bucket *buc =  table->buckets[hash_cuvant];
		int ok = 1,i = 0;
		// verificam daca cuvantul a fost deja adaugat
		for(i = 0; i < buc->curent; i++){
			if(strcmp(buc->cuvinte[i],cuvant) == 0){
				ok = 0;
			}
		}
		
		// daca nu a fost adaugat il adaugam 
		if(ok == 1){
			//daca pozitia curenta este ultima pozitie din vector
			// atunci dublam dimensiunea vectorului
			if(buc->curent == buc->size){
					
				buc->size *= 2;
				char ** tmp;
				// si reallocam memorie pentru dimensiunea dublata
				tmp = realloc(buc->cuvinte,buc->size*(sizeof(char*)));
				DIE(tmp == NULL, "Couldn't allocate memory");
				buc->cuvinte = tmp;
			}
			unsigned int curent = buc->curent;
			buc->cuvinte[curent] = strdup(cuvant);
			buc->curent++;
		}
	}
}


void remove_cuvant(hashTable *table,char* cuvant,unsigned int dim_hash){
	// calculam hash-ul pentru valoarea cautata
	unsigned int hash_cuvant = hash(cuvant,dim_hash);
	
	bucket * bucket = table->buckets[hash_cuvant];
	
	char * cuvant_curent = bucket->cuvinte[0];
	//cat timp mai sunt cuvinte le parcurgem
	int ok = 0;
	int i = 0;
	//cautam cuvantul 
	while(cuvant_curent != NULL){
		if(strcmp(cuvant_curent, cuvant) == 0){
			ok = 1;
			break;
		}
		i++;
		cuvant_curent = bucket->cuvinte[i];
	}
	//daca s-a gasit permutam restul cuvintelor la stanga
	// si facem ultimul element NULL
	if(ok == 1){
		while(cuvant_curent != NULL){
			bucket->cuvinte[i] = bucket->cuvinte[i+1];
			i++;
			cuvant_curent = bucket->cuvinte[i];
		}
		// eliberam ultima pozitie
		free(bucket->cuvinte[i-1]);
		bucket->cuvinte[i-1] = NULL;
		bucket->curent --;
	}
}

int clear_hash(hashTable *table,unsigned int dim_hash){
	int i = 0;
	// parcurgem toata tabela
	for(i = 0; i < dim_hash; i++){
		bucket * buck = table->buckets[i];
		// daca bucketul nu este null atunci dezalocam
		// si lista de cuvinte
		if(buck != NULL){
			int j;
			for(j = 0; j < buck->curent; j++){
				remove_cuvant(table,table->buckets[i]->cuvinte[j],dim_hash);
			}
			free(table->buckets[i]->cuvinte);
		}
		free(table->buckets[i]);
		table->buckets[i] = NULL;
	}
	
	return 0;
}


void find_cuvant(hashTable *table,char* cuvant,unsigned int dim_hash,char* numefis){
	
	unsigned int hash_cuvant = hash(cuvant,dim_hash);
	bucket * bucket = table->buckets[hash_cuvant];
	int ok = 0;
	if(bucket != NULL){
		int i = 0;
		for(i = 0; i < bucket->curent; i++){
			if(strcmp(bucket->cuvinte[i],cuvant) == 0){
				ok = 1;
				break;
			}
		}
	}
	if(numefis == NULL){
		if(ok == 0)
			printf("False\n");
		else
			printf("True\n");	
	}else{
		FILE *f = fopen(numefis,"a");
		if(ok == 0)
			fprintf(f,"False\n");
		else
			fprintf(f,"True\n");
		fclose(f);
	}
		
	
}

void print_bucket(hashTable *table,unsigned int index_bucket,char * numefis){
	bucket* bucket = table->buckets[index_bucket];
	int i = 0;
	if(bucket != NULL){
		if(numefis == NULL){
			for(i = 0; i < bucket->curent-1; i++){
				printf("%s ",bucket->cuvinte[i]);
			}
			if(bucket->curent != 0){
				printf("%s", bucket->cuvinte[i]);
				printf("\n");
			}
		}else{
			FILE * f = fopen(numefis,"a");
			for(i = 0; i < bucket->curent-1; i++){
				fprintf(f,"%s ",bucket->cuvinte[i]);
			}
			if(bucket->curent != 0){
				fprintf(f,"%s", bucket->cuvinte[i]);
				fprintf(f,"\n");
			}
			fclose(f);
		}
	}
	
	
}

void print_hash(hashTable *table,unsigned int dim_hash, char* numefis){
	int i;
	for(i = 0; i < dim_hash; i++){
		bucket * bucket = table->buckets[i];
		if(bucket != NULL){
			print_bucket(table,i,numefis);
		}
	}
	
}

void resize_any(hashTable *table, unsigned int *dim_hash
					,unsigned int new_dim,hashTable * new_table){
	
	new_table->buckets = malloc(new_dim * sizeof(bucket *));
	DIE(new_table->buckets == NULL, "Couldn't allocate memory");
	int i = 0;
	
	for(i = 0; i < (*dim_hash); i++){
		bucket* bucket = table->buckets[i];
		int j = 0;
		
		if(bucket != NULL ){
			for(j = 0; j < bucket->curent; j++){
				
				char * cuvant = bucket->cuvinte[j];
				add_cuvant(new_table,cuvant,new_dim);
				
			}
		}
		
	}
	clear_hash(table,*dim_hash);
	
	//table = new_table;
	//table = &new_table;
	*dim_hash = new_dim;
	
}

void  resize_double(hashTable *table,unsigned int *dim_hash){
	unsigned int dim = *dim_hash;
	hashTable new_table;
	resize_any(table,&dim,2 * dim,&new_table);
	*dim_hash = dim;
	*table = new_table;
}

void resize_halve(hashTable *table,unsigned int *dim_hash){
	
	unsigned int new_hash = (*dim_hash) /2.0;
	hashTable new_table;
	resize_any(table,dim_hash,new_hash,&new_table);
	*table = new_table;
}


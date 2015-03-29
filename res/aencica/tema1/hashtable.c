/*
	Encica Alexandru
	Grupa 332CC
	mail: alexandru.encica@cti.pub.ro
*/

#include "utils.h"

HashTable *init (unsigned int size)
{
	HashTable *ht = NULL;
	unsigned int i;
	ht = malloc(sizeof(HashTable));
	DIE(!ht,"malloc failed");
	ht->size = size;
	ht->table = malloc(size * sizeof(Entry*));
	DIE(!ht->table,"malloc failed");
	for (i = 0 ; i < size; i++){
		ht->table[i] = NULL;
	}
	return ht;
}

void add(HashTable *ht, char *word)
{
	Entry *new_entry;
	unsigned int bucket = hash(word, ht->size);
	if (!ht->table[bucket]){
		ht->table[bucket] = malloc(sizeof(Entry));
		DIE(!ht->table[bucket],"malloc failed");
		ht->table[bucket]->word = malloc((1+strlen(word))*sizeof(char));
		DIE(!ht->table[bucket]->word,"malloc failed");
		strcpy(ht->table[bucket]->word,word);
		ht->table[bucket]->next = NULL;
	}
	else{
		if (!(new_entry = not_exists(ht,word,bucket)))
			return;
		new_entry->next = malloc(sizeof(Entry));
		DIE(!new_entry->next,"malloc failed");
		new_entry = new_entry->next;
		new_entry->word = malloc((1+strlen(word))*sizeof(char));
		DIE(!new_entry->word,"malloc failed");
		strcpy(new_entry->word,word);
		new_entry->next = NULL;
	}
}

void delete(HashTable *ht, char *word)
{
	unsigned int bucket = hash(word, ht->size);
	Entry *current_entry = ht->table[bucket];
	Entry *temp;
	if(!current_entry){
		return;
	}
	
	/*daca e primul element din bucket*/
	if(!strcmp(current_entry->word,word)){
		temp = ht->table[bucket];
		ht->table[bucket] = current_entry->next;
		free(temp->word);
		free(temp);
		return;
	}

	/*daca nu e primul element din bucket*/
	while (1){
		if (!current_entry->next){
			return;
		}
	 	if(!strcmp(current_entry->next->word,word)){
			temp = current_entry->next;
			current_entry->next = current_entry->next->next;
			free(temp->word);
			free(temp);
			return;
		}
		current_entry  = current_entry->next;
	}
}

int find(HashTable *ht, char *word)
{
	unsigned int bucket = hash(word, ht->size);
	if(ht->table[bucket] && !not_exists(ht,word,bucket))
		return 1;
	else
		return 0;
}

void clear(HashTable *ht)
{
	unsigned int i;
	for (i = 0 ; i < ht->size; i++){
		while(ht->table[i])
			delete(ht,ht->table[i]->word);
	}
}



Entry *not_exists(HashTable *ht, char *word,unsigned int bucket)
{
	Entry *entry = ht->table[bucket];

	while(1){
		if(!strcmp(entry->word,word))
			return NULL;
		if(!entry->next)
			return entry;
		entry=entry->next;
	}
}

void copy_elements(HashTable *ht, HashTable *htn,unsigned int bucket)
{
	Entry *current_entry = ht->table[bucket];
	Entry *temp;
	while(1){
		if(!current_entry){
			break;	
		}
		add(htn,current_entry->word);
		temp = current_entry;
		current_entry = current_entry->next;
		delete(ht, temp->word);
	}
}

HashTable *resize(HashTable *ht,unsigned int size)
{
	HashTable *htn = init(size);
	unsigned int i;
	for (i = 0; i < ht->size; i++){
		copy_elements(ht,htn,i);
	}
	free (ht->table);
	free (ht);
	return htn;
}

HashTable *resize_double(HashTable *ht)
{
	return resize(ht,2*ht->size);
}

HashTable *resize_halve(HashTable *ht)
{
	return resize(ht,ht->size/2);
}

void print(HashTable *ht, FILE *out_file)
{
	unsigned int i;
	for (i = 0 ; i < ht->size; i++){
		if(!ht->table[i])
			continue;
		print_bucket(ht,i,out_file);
		fprintf(out_file,"\n");
	}
}

void print_bucket(HashTable *ht,unsigned int bucket, FILE *out_file)
{
	Entry *e = ht->table[bucket];
	if (bucket>=ht->size)
		return;

	if (!e)
		return;

	while(1) {
		fprintf(out_file,"%s ",e->word);
		if (e->next==NULL)
			break;
		e = e->next;
	}
}
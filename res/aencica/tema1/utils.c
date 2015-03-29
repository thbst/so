/*
	Encica Alexandru
	Grupa 332CC
	Tema 1 SO
*/

#include "utils.h"

HashTable *parseLine(HashTable *ht, char *line){
	char *tok, *file_name;
	char *word;
	unsigned int index;
	FILE * out_file;

	tok = strtok(line,DELIMS);
	if (!strcmp(tok,"add")){
		word = strtok(NULL,DELIMS);
		add(ht,word);
	}
	if (!strcmp(tok,"remove")){
		word = strtok(NULL,DELIMS);
		delete(ht,word);
	}
	if (!strcmp(tok,"clear")){
		clear(ht);
	}
	if(!strcmp(tok,"find")){
		word = strtok(NULL,DELIMS);
		if((file_name = strtok(NULL, DELIMS)))
			out_file = fopen(file_name,"a");
		else
			out_file = stdout;

		if (find(ht,word))
			fprintf(out_file,"True\n");
		else
	 		fprintf(out_file, "False\n");
		if(out_file != stdout)
			fclose(out_file);
	}
	if (!strcmp(tok,"print_bucket")){
		index = atoi(strtok(NULL,DELIMS));
		if((file_name = strtok(NULL, DELIMS)))
			out_file = fopen(file_name,"a");
		else
			out_file = stdout;
		print_bucket(ht,index,out_file);
		fprintf(out_file, "\n");
		if(out_file != stdout)
			fclose(out_file);
	}
	if (!strcmp(tok,"print")){
		if((file_name = strtok(NULL, DELIMS)))
			out_file = fopen(file_name,"a");
		else
			out_file = stdout;
		print(ht,out_file);
		if(out_file != stdout)
			fclose(out_file);
	}
	if (!strcmp(tok,"resize")){
		tok = strtok(NULL, DELIMS);
		if(!strcmp(tok,"double"))
		 	ht = resize_double(ht);
		if(!strcmp(tok,"halve"))
			ht = resize_halve(ht);
	}
	return ht;
}

HashTable *parseFile(HashTable *ht, FILE *in_file){
	char line[BUFFSIZE];
	while(fgets(line,BUFFSIZE,in_file)){
		if (line[0] == '\n')
			continue;
		ht = parseLine(ht, line);
	}
	return ht;
}
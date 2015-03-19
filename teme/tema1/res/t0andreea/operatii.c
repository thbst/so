/* Badoiu Simona Andreea 331CB - Tema0 HashTable */
#include "operatii.h"

void add(char* cuvant, bucket **hashtable, unsigned int dim) {
	unsigned int index = hash(cuvant, dim);
	int /*len,*/ nr_elem, i;
	bucket *b = &(*hashtable)[index];
	nr_elem = ((*hashtable)[index]).n;
	/* daca nu avem niciun element in bucket, facem alocarile si adaugam
	fara alte verificari*/
	if (nr_elem == 0) {
		/* alocam memorie pentru un vector care sa contina un singur string */
		b->elemente = malloc(sizeof(char*));
		DIE(b->elemente == NULL, "Alocare nereusita!\n");
		/* copiem cuvantul pe pozitia corespunzatoare,
		strdup aloca si memorie, asa ca nu mai este nevoie
		de alocare */
		(b->elemente)[0] = strdup(cuvant);
		/*Verificare strdup */
		DIE((b->elemente)[0] == NULL, "Alocare nereusita!\n");
		b->n = 1;
	}
	/* daca deja exista elemente in bucket, verificam daca acest cuvant
	este duplicat. Daca nu este duplicat, realocam vectorul de string si
	adaugam noul cuvant in bucket */
	else {
		/* verificam daca acest cuvant exista deja */
		for (i = 0; i < nr_elem; i++) {
			if (strcmp((b->elemente)[i], cuvant) == 0) {
				return;
			}
		}
		/* realocam - pentru nr_elem_bucket + 1 */
		b->elemente = realloc(b->elemente, (b->n + 1)*sizeof(char*));
		DIE(b->elemente == NULL, "Alocare nereusita!\n");
		/* alocam spatiu pentru noul cuvant si il copiem */
		(b->elemente)[b->n] = strdup(cuvant);
		DIE((b->elemente)[b->n] == NULL, "Alocare nereusita!\n");
		b->n += 1;
	}
}

void rmv(char* cuvant, bucket **hashtable, unsigned int dim) {
	unsigned int index, nr_elem_bucket, i, j;
	bucket *b;
	char *aux;
	/* indexul bucket-ului din care trebuie sa stergem cuvantul */
	index = hash(cuvant, dim);
	b = &(*hashtable)[index];
	nr_elem_bucket = b->n;
	for (i = 0; i < nr_elem_bucket; i++) {
		/* cautam cuvantul in bucket-ul corespunzator */
		if (strcmp(cuvant, (b->elemente)[i]) == 0) {
			aux = (b->elemente)[i];
			/* daca nu ne aflam pe ultimul cuvant, mutam cu o pozitie
			la stanga toate elementele de dupa cuvantul cautat */
			if (i < (nr_elem_bucket - 1)) {
				for (j = i + 1; j < nr_elem_bucket; j++) {
					(b->elemente)[j-1] = (b->elemente)[j];
				}
				/* ultimul element nu trebuie sa mai trimita
				catre nicio adresa */
				(b->elemente)[nr_elem_bucket - 1] = NULL;
				/* eliberam memoria ocupata de elementul pe care l-am sters
				si micsoram numarul de elemente din bucket */
				free(aux);
				(b->n)--;
			}
			/* cand cuvantul cautat se afla pe ultima pozitie, eliberam memoria si
			micsoram numarul de elemente din bucket cu 1 */
			else {
				free(aux);
				(b->elemente)[i] = NULL;
				(b->n)--;
				/* daca bucketul continea doar un element, atunci eliberam si
				elemente deoarece inseamna ca nu a mai ramas nimic in bucket */
				if (nr_elem_bucket == 1) {
					free(b->elemente);
					b->elemente = NULL;
				}
			}
		return;
		}
	}
}

/* nu am folosit functia rmv deoarece ar fi consumat mai mult timp,
avand in vedere ca trebuia cautat mereu cuvantul care trebuia sters.
Varianta din clear sterge la rand fiecare cuvant gasit si elibereaza
memoria de fiecare data cand este nevoie */
void clear(bucket **hashtable, unsigned int dim) {
	unsigned int i, j, nr_elem_bucket;
	char **elemente;
	/* eliberam memoria ocupata de fiecare element, din fiecare bucket */
	for (i = 0; i < dim; i++) {
		elemente = (*hashtable)[i].elemente;
		nr_elem_bucket = (*hashtable)[i].n;
		for (j = 0; j < nr_elem_bucket; j++) {
			free(elemente[j]);
			elemente[j] = NULL;
		}
		/* daca bucketul a continut cel putin un element, adica s-a
		alocat memorie pentru el in add, atunci eliberam memoria
		alocata pentru vectorul de string*/
		if (elemente != NULL && nr_elem_bucket != 0) {
			free(elemente);
			elemente = NULL;
		}
		/* reinitializam numarul de elemente din bucket */
		(*hashtable)[i].n = 0;
	}	
}

void find(char* cuvant, bucket *hashtable, unsigned int dim, char* fisier) {
	/* write_to_file = 1/0 daca scriem in fisier/nu scriem in fisier */
	unsigned int index, nr_elem_bucket, i, write_to_file;
	char **elemente;
	FILE* fis_out;
	index = hash(cuvant, dim);
	elemente = hashtable[index].elemente;
	nr_elem_bucket = hashtable[index].n;
	/* deschidem fisierul, in cazul in care l-am primit ca parametru */
	if (fisier != NULL) {
		write_to_file = 1;
		fis_out = fopen(fisier, "a");
		DIE(fis_out == NULL, "Nu s-a putut deschide fisierul");
	}
	else {
		write_to_file = 0;
	}
	/* cauta cuvantul la indexul dat de functia hash */
	for (i = 0; i < nr_elem_bucket; i++) {
		/* daca gasim cuvantul, afisam True */
		if (strcmp(cuvant, elemente[i]) == 0) {
			if (write_to_file) {
				fprintf(fis_out, "True\n");
			}
			else {
				printf("True\n");
			}
		}
	}
	/* daca in cadrul parcurgerii nu am gasit elementul, afisam False */
	if (write_to_file) {
		fprintf(fis_out, "False\n");
	}
	else {
		printf("False\n");
	}
}

/* stim ca intotdeauna index_bucket este valid, asa ca nu am mai verificat */
void print_bucket(unsigned int index_bucket,
				bucket *hashtable,
				unsigned int dim, char* fisier) {
	char **elemente;
	unsigned int i, nr_elem_bucket;
	bucket b;
	FILE* fis_out;
	b = hashtable[index_bucket];
	nr_elem_bucket = b.n;
	elemente = b.elemente;
	if (nr_elem_bucket != 0) {
		/* daca s-a primit numele unui fisier, atunci vom scrie in el */
		if (fisier != NULL) {
			fis_out = fopen(fisier, "a");
			DIE(fis_out == NULL, "Nu s-a putut deschide fisierul!\n");
			/* afisam toate elementele din bucket, in fisierul primit ca parametru */
			for (i = 0; i < nr_elem_bucket; i++) {
				fprintf(fis_out, "%s ", elemente[i]);
			}
			fprintf(fis_out, "\n");
			fclose(fis_out);
		}
		/* altfel, scriem la stdout */
		else {
			for (i = 0; i < nr_elem_bucket; i++) {
				if (elemente[i] != NULL) {
					printf("%s ", elemente[i]);
				}
			}
			printf("\n");
		}
	}
}

/* printeaza toate bucketurile - am folosit functia print_bucket */
void print_all(bucket *hashtable, unsigned int dim, char* fisier) {
	unsigned int i;
	for(i = 0; i < dim; i++) {
		print_bucket(i, hashtable, dim, fisier);
	}
}

bucket* resize(bucket **hashtable, unsigned int dim, unsigned int dim_noua) {
	unsigned int i, j, nr_elem_bucket;
	char **elemente;
	char *cuvant;
	/* se aloca un nou hashtable si copiem pe pozitiile
	   corespunzatoare elementele din hashtable-ul vechi */
	bucket *hashtable_nou = calloc(dim_noua, sizeof(bucket));
	for (i = 0; i < dim; i++) {
		elemente = (*hashtable)[i].elemente;
		nr_elem_bucket = (*hashtable)[i].n;
		/*parcurgem fiecare cuvant si apelam add in noul hashtable */
		for(j = 0; j < nr_elem_bucket; j++) {
			cuvant = elemente[j];
			add(cuvant, &hashtable_nou, dim_noua);
		}
	}
	/* stergem toate elementele din vechiul hashtable */
	clear(hashtable, dim);
	/* eliberam memoria alocata pentru vechiul hashtable */
	free(*hashtable);
	return hashtable_nou;
}

/* returneaza rezultatul primit de la resize. Acesta reprezinta
un hashtable cu dimensiunea dubla fata de vechiul hashtable */
bucket* resize_double(bucket **hashtable, unsigned int *dim) {
	unsigned int old_dim = *dim;
	*dim = 2*old_dim;
	return resize(hashtable, old_dim, *dim);
	
}

/* returneaza rezultatul primit de la resize. Acesta reprezinta
un hashtable cu dimensiunea injumatatia fata de vechiul hashtable.
Daca dimensiunea este de forma 2k + 1, noul hashtable va avea
dimensiunea k */
bucket* resize_half(bucket **hashtable, unsigned int *dim) {
	unsigned int old_dim = *dim;
	if (old_dim % 2 == 0) {
		*dim = (unsigned int)(old_dim/2);
	}
	else {
		*dim = (unsigned int)((old_dim - 1)/2);
	}
	return resize(hashtable, old_dim, *dim);
}





























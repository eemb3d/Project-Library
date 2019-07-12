/** \file bib.c
	\brief contiene tutte le funzioni della libreria del progetto
*/

#include "bib.h"

/** Crea ed inizializza un nuovo autore

    \param a puntatore all'autore
 */
autore_t *new_autore(autore_t *a){
	if ((a = calloc(1, sizeof(autore_t))) == NULL)
		ALLOCAZIONE_MEM_FALLITA("La calloc() ha restituito null nella creazione dell'autore")
	return a;
}

/** crea ed inizializza una nuova scheda
    \returns new_elem la nuova scheda (allocata all'interno della funzione)
    \returns NULL se ci sono stati problemi (setta errno)
*/
scheda_t * new_scheda(void) {
	scheda_t *new_elem = NULL;
	if ((new_elem = calloc(1, sizeof(scheda_t))) == NULL )
		ALLOCAZIONE_MEM_FALLITA("La calloc() ha restituito null nella creazione della scheda")
	return new_elem;
}

/** libera la memoria occupata da una scheda

    \param ps puntatore al puntatore alla scheda (*ps viene settato a NULL dalla funzione)
 */
void free_scheda(scheda_t **ps) {
	autore_t* a_tmp = NULL,  *a_tmp1 = NULL;

	if (ps != NULL && *ps != NULL) {
		/*Se l'autore è diverso da NULL si verifica se ci sono più autori per deallocarli*/
		if((a_tmp1 = (*(ps))->autore) != NULL){
			/*Elimina i vari autori*/
			for (a_tmp = a_tmp1->next; a_tmp != NULL; a_tmp1 = a_tmp, a_tmp = a_tmp->next)
				free(a_tmp1);
			free(a_tmp1);
		}/*Elimina la scheda*/
		free(*ps); *ps = NULL;
	}
}

/** confronta due autori
    \param a1 puntatore al primo autore
    \param a2 puntatore al secondo autore

    \returns 0 se gli autori sono uguali (uguali e non nulli) e
    \returns 1 altrimenti
 */
int compar_autore(autore_t * a1, autore_t * a2){
	int valore = 0;
	for( ; (a1 && a2 && !(valore = strcmp(a1->cognome, a2->cognome)) &&
			!(valore = strcmp(a1->nome, a2->nome))); a1 = a1->next, a2 = a2->next);
	/*Ritorna zero se sono uguali*/
	return valore;
}

/** confronta due schede
    \param s1 puntatore alla prima scheda
    \param s2 puntatore alla seconda scheda

    \returns TRUE se le schede sono uguali (tutti i campi sono uguali) e
    \returns FALSE altrimenti
 */
int is_equal_scheda(scheda_t * s1, scheda_t * s2) {
	return ((s1 && s2 && !strcmp(s1->nota, s2->nota) &&	!strcmp(s1->collocazione, s2->collocazione) &&
		!strcmp(s1->titolo, s2->titolo) &&	!strcmp(s1->descrizione_fisica, s2->descrizione_fisica) &&
		!strcmp(s1->pub.editore, s2->pub.editore) && !strcmp(s1->pub.luogo, s2->pub.luogo) &&
		(s1->pub.anno == s2->pub.anno) && !compar_autore(s1->autore, s2->autore) && ((!s1->prestito.disponibile &&
		!s2->prestito.disponibile) || is_equal_data(s1->prestito.scadenza, s2->prestito.scadenza))))
		? TRUE : FALSE;
}

/** trasforma un'autore in una stringa
     \param a puntatore all'autore da trasformare

    \returns stringa_autore il puntatore all'autore (allocato all'interno della funzione)
    \returns  NULL se ci sono stati problemi (setta errno)
*/
char *crea_record_autore(autore_t * a){
	char *stringa_autore = NULL;
	autore_t* a_tmp = NULL;
	short int dim = 0;

	if((a_tmp = a)){
		DIMENSIONE_AUTORE(a_tmp, dim);
		if (!(stringa_autore = calloc(dim, sizeof(char))))
			SYSTEM_CALL_FALLITA("La calloc() ha restituito null")
		sprintf(stringa_autore, "autore: %s, %s;", a->cognome, a->nome);
		a_tmp = a;
		/*Concatena i successivi autori, se ci sono*/
		AGGIUNGI_ALTRI_AUTORI(stringa_autore, a_tmp);
	}
	return stringa_autore;
}

/** stampa la scheda in formato RECORD sullo stream specificato

    \param f stream di output
    \param s puntatore alla scheda
 */
void print_scheda(FILE* f, scheda_t * s) {
	char * stringa = NULL;
	if(f != NULL) {
		if(s != NULL && s->autore != NULL){
			if((stringa = crea_record_autore(s->autore))){
				(s->prestito.disponibile == FALSE) ?
						fprintf(f, "%s\ntitolo: %s;\neditore: %s;\nanno: %d;\nluogo_pubblicazione: %s;\nprestito: %d-%s%d-%d;\n"
								"collocazione: %s;\ndescrizione_fisica: %s;\nnota: %s;\n", stringa, PARAMETRI_RECORD_CON_PRESTITO(s)) :
						fprintf(f, "%s\ntitolo: %s;\neditore: %s;\nanno: %d;\nluogo_pubblicazione: %s;\n"
								"collocazione: %s;\ndescrizione_fisica: %s;\nnota: %s;\n", stringa, PARAMETRI_RECORD_SENZA_PRESTITO(s));
				if(stringa != NULL) free(stringa);
			}
		}else {
			ERRORE_ARGOMENTI("Errore nella scheda passata")
			fprintf(f, "%s\n\n%s\n", "Stampa scheda vuota:", PRINT_SCHEDA_VUOTA);
		}
	}else ERRORE_APERTURA_FILE("Errore nel file passato, risulta essere NULL")
}

/** trasforma un record in una scheda
    \param r record da trasformare (non viene modificato)
    \param l lunghezza massima del record (serve ad evitare l'overrun ...)

    \returns res la nuova scheda (allocata all'interno della funzione)
    \returns NULL se ci sono stati problemi (setta errno)
                  errno=EINVAL per stringa mal formattata
*/
scheda_t * record_to_scheda(char* r, int l) {
	/*r_tmp copia del record passato, s_tmp stringa di appoggio su cui lavorare con la strtok mentre p_s e p_r sono
	 *  i puntatori iniziali alle stringhe precedenti in modo da potere fare successivamente la free*/
	char *r_tmp = NULL, *s_tmp = NULL, *p_s = NULL, *p_r = NULL, *tmp = NULL;
	short int corretto = TRUE, tag = FALSE, nome = FALSE, cognome = FALSE, titolo = FALSE, editore = FALSE,
			luogo = FALSE, collocazione = TRUE, descr_fisica = FALSE, nota = TRUE, prestito = TRUE, anno = FALSE,
			presenti_autori_multipli = FALSE;
	scheda_t* res = NULL;
	autore_t *campo_autore = NULL;

	if ((corretto = (l > 0))) {
		p_r = r_tmp = calloc(l, sizeof(char));
		p_s = s_tmp = calloc(l, sizeof(char));
		if (corretto && (corretto = (r_tmp != NULL && s_tmp != NULL))){
			if(corretto && (corretto = ((res = new_scheda()) != NULL))){
				if(corretto && (corretto = ((campo_autore = res->autore = new_autore(res->autore)) != NULL))){
					strcpy(r_tmp, r);
					if(STR_RECORD_CORRETTO(r_tmp, tmp) == FALSE) corretto = FALSE;
					strcpy(s_tmp, strtok(r_tmp, ":"));
					/*Inizializiamo la scheda affinche sia disponibile il prestito*/
					res->prestito.disponibile = TRUE;
					while (corretto && (corretto = (tag = TAG_CORRETTO(s_tmp)))) {
						switch (tipo_campo(s_tmp)) {
							case AUTORE :
								strcpy(s_tmp, strtok(NULL, ","));
								if ((corretto = cognome = NOMINATIVO_CORRETTO(s_tmp))){
									if(presenti_autori_multipli){
										campo_autore->next = new_autore(campo_autore->next);
										campo_autore = campo_autore->next;
									}
									strcpy(campo_autore->cognome, s_tmp);
									strcpy(s_tmp, strtok(NULL, ";"));
									if ((corretto = nome = NOMINATIVO_CORRETTO(s_tmp))){
										strcpy(campo_autore->nome, s_tmp);
										presenti_autori_multipli = TRUE;
									}
								}break;
							case TITOLO :
								strcpy(s_tmp, strtok(NULL, ";"));
								if ((corretto = titolo = LLSTRING_CORRETTO(s_tmp)))
									strcpy(res->titolo, s_tmp);
								break;
							case EDITORE :
								strcpy(s_tmp, strtok(NULL, ";"));
								if ((corretto = editore = LSSTRING_CORRETTO(s_tmp)))
									strcpy(res->pub.editore, s_tmp);
								break;
							case LUOGO_PUBBLICAZIONE :
								strcpy(s_tmp, strtok(NULL, ";"));
								if ((corretto = luogo = LSSTRING_CORRETTO(s_tmp)))
									strcpy(res->pub.luogo, s_tmp);
								break;
							case ANNO :
								strcpy(s_tmp, strtok(NULL, ";"));
								corretto = anno = ANNO_CORRETTO(s_tmp, res->pub.anno);
								break;
							case COLLOCAZIONE :
								strcpy(s_tmp, strtok(NULL, ";"));
								if ((corretto = collocazione = LSSTRING_CORRETTO(s_tmp)))
									strcat(res->collocazione, s_tmp);
								break;
							case DESCRIZIONE_FISICA :
								strcpy(s_tmp, strtok(NULL, ";"));
								if ((corretto = descr_fisica = LSSTRING_CORRETTO(s_tmp)))
									strcpy(res->descrizione_fisica, s_tmp);
								break;
							case NOTA :
								strcpy(s_tmp, strtok(NULL, ";"));
								if ((corretto = nota = LLSTRING_CORRETTO(s_tmp)))
									strcpy(res->nota, s_tmp);
								break;
							case PRESTITO :
								strcpy(s_tmp, strtok(NULL, ";"));
								if((corretto = prestito = (STRINGA_DATA_CORRETTA(s_tmp, res->prestito.scadenza))))
									res->prestito.disponibile =  FALSE;
								break;
							default:  corretto = FALSE; break;
						}
						/*E' necessario fare un ulteriore verifica per considerare correttamente la
						 * terminazione del record e non restituire in questo modo un falso errore
						 * nella condizione del while*/
						if(!corretto || !(r_tmp = strtok(NULL, ":\n"))) break;
						strcpy(s_tmp, r_tmp);
					}
				}
			}
		}
		/*Elimino le stringhe di supporto*/
		if(p_r != NULL) free(p_r);
		if(p_s != NULL) free(p_s);
	}
	/*Nell'eventualità ci fossero degli errori setta errno e libera la memoria*/
	if (!corretto || !tag || !nome || !cognome || !titolo || !editore || !luogo || !anno ||
			!collocazione || !descr_fisica || !nota || !prestito) {
		if(res != NULL) free_scheda(&res);
		errno = EINVAL;
	}
	return res;
}

/** trasforma una scheda in un record
    \param s la scheda da trasformare

    \returns r il puntatore al record che rappresenta la scheda (allocato all'interno della funzione)
    \returns  NULL se ci sono stati problemi (setta errno)
*/
char * scheda_to_record(scheda_t* s) {
	char *r = NULL, *stringa_autore = NULL;
	unsigned int dim_max_record = DIM_MAX;
	autore_t* a_tmp = NULL;

	if(s != NULL){
		a_tmp = s->autore;
		/*Almeno una volta viene eseguita per aggiungere la dimensione dell'autore.*/
		DIMENSIONE_AUTORE(a_tmp, dim_max_record);
		/*Creo il record con la sua dimensione massima*/
		if(!(r = calloc(dim_max_record, sizeof(char))))
			SYSTEM_CALL_FALLITA("La calloc() ha restituito null")
		stringa_autore = crea_record_autore(s->autore);
		(s->prestito.disponibile == FALSE) ?
				sprintf(r, "%s titolo: %s; editore: %s; anno: %d; luogo_pubblicazione: %s; prestito: %d-%s%d-%d; collocazione: %s;"
						" descrizione_fisica: %s; nota: %s;\n", stringa_autore, PARAMETRI_RECORD_CON_PRESTITO(s)) :
				sprintf(r, "%s titolo: %s; editore: %s; anno: %d; luogo_pubblicazione: %s; collocazione: %s;"
						" descrizione_fisica: %s; nota: %s;\n", stringa_autore, PARAMETRI_RECORD_SENZA_PRESTITO(s));
		if(stringa_autore != NULL) free(stringa_autore);
	}else ERRORE_ARGOMENTI("Errore nella conversione della scheda in record")

	return r;
}

/** legge tutti i record da un file e li memorizza in un array -- i record malformati vengono
 * scartati ma se c'e' almeno un record convertibile corretamente la funzione ha successo

    \param fin stream da cui leggere i record
    \param psarray puntatore alla variabile che conterra' l'indirizzo dell'array di puntatori
    alle schede (allocato), significativo se n >0

    \returns -1 se si e' verificato un errore (setta errno)
    \returns n (n > 0) numero di record convertiti correttamente
     (ampiezza dell'array di schede) in questo caso *psarray contiene l'indirizzo dell'array allocato
 */
int load_records(FILE* fin, scheda_t** psarray[]) {
	unsigned int n = 0, lung = 0;
	char record[LUNG_MAX_RIGA_FILE];
	lista l_scheda = NULL;
	scheda_t *tmp = NULL;

	memset(record, '\0', sizeof(char));
	while(fgets(record, LUNG_MAX_RIGA_FILE, fin) != NULL){
		if ((lung = strlen(record)) > 0 && (tmp = record_to_scheda(record, ++lung)) != NULL){
			/*Utilizzo la lista affinchè memorizi le schede create correttamente, invece di creare un'array 
			  senza sapere quanto è lungo per poi ridimensionarlo*/			
			l_scheda = insert_in_coda(l_scheda, tmp);
			n++; 
		}memset(record, '\0', sizeof(char));
	}
	/*Salviamo le informazioni delle schede nell'array*/
	if(n != 0) *psarray = new_array_schede(l_scheda, n);
	else ERRORE_CONVERSIONE("Errore nella conversione dei record in scheda")

	freeList(l_scheda);

	return (n) ? n : -1;
}

/** scrive su file un array di schede sotto forma di record
    \param fout stream su cui scrivere i record
    \param sarray puntatore all'array di puntatori alle schede
    \param n numero schede nell'array

    \returns -1 se si e' verificato un errore (setta errno) -- nota l'errore nella scrittura di un
    singolo record viene riportato nel numero di record scritti, non provoca il ritorno di (-1)
    \returns n (n > 0) numero di record scritti correttamente */
int store_records(FILE* fout, scheda_t* sarray[], int n) {
	int num_record_corretti = 0, i = 0;
	char *record = NULL;

	if(fout != NULL) {
		for(; n > 0; n--, i++){
			if((record = scheda_to_record(sarray[i])) != NULL){
				fprintf(fout, "%s",record);
				if(record != NULL) free(record);
				num_record_corretti++;
			}
		}
		if(!num_record_corretti) ERRORE_CONVERSIONE("Errore nella conversione delle schede in record")
	}else ERRORE_APERTURA_FILE("Errore nel file passato, risulta essere NULL")
	n = num_record_corretti;
	return (n) ? n : -1;
}

/** ordina i record nell'array relativamente al campo specificato. Gli ordinamenti definiti per i vari campi sono i seguenti:
   AUTORE -- lessicografico (Cognome Nome) sulla lista degli autori
   TITOLO, EDITORE, LUOGO_PUBBLICAZIONE, COLLOCAZONE, DESCRIZIONE_FISICA, NOTA -- Lessicografico sulla stringa
   ANNO -- crescente
   PRESTITO -- crescente

   Nota: per ordine Lessicografico si intende quello normalmente usato nella strcmp()

   \param sarray puntatore array da ordinare (viene modificato durante l'ordinamento)
   \param n lunghezza array
   \param c campo rispetto al quale ordinare

   \returns -1 se si e' verificato un errore (setta errno) (in questo caso s e' invariato)
   \returns 0 se tutto e' andato bene
*/
int sort_schede(scheda_t* sarray[], int n, campo_t c) {
	short int corretto = TRUE;

	if (n) {
		switch (c) {
			case AUTORE :		 		qsort(sarray, n, sizeof(scheda_t *), autore_cmp); break;
			case TITOLO : 				qsort(sarray, n, sizeof(scheda_t *), titolo_cmp); break;
			case EDITORE : 				qsort(sarray, n, sizeof(scheda_t *), editore_cmp); break;
			case LUOGO_PUBBLICAZIONE : 	qsort(sarray, n, sizeof(scheda_t *), luogo_cmp); break;
			case ANNO : 				qsort(sarray, n, sizeof(scheda_t *), int_cmp); break;
			case COLLOCAZIONE : 		qsort(sarray, n, sizeof(scheda_t *), collocazione_cmp); break;
			case DESCRIZIONE_FISICA :	qsort(sarray, n, sizeof(scheda_t *), desc_fisica_cmp); break;
			case NOTA :					qsort(sarray, n, sizeof(scheda_t *), nota_cmp); break;
			case PRESTITO :				qsort(sarray, n, sizeof(scheda_t *), data_cmp); break;
			default: corretto = FALSE; break;
		}
	}
	return (corretto) ? 0 : -1;
}

/** Compara due date (prestiti)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo zero se sono uguali
*/
int data_cmp(const void * a, const void * b){
	scheda_t *x = *(scheda_t **)a, *y = *(scheda_t **)b;
	return ((int)difftime((time_t)&x->prestito.scadenza, (time_t)&y->prestito.scadenza));
}
/** Compara due interi (anno)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns la differenze dei due valori
*/
int int_cmp(const void * a, const void * b){
	scheda_t *x = *(scheda_t **)a, *y = *(scheda_t **)b;
	return (diff(x->pub.anno, y->pub.anno));
}
/** Compara due autori

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns 0 se sono uguali 1 altrimenti
*/
int autore_cmp(const void * a, const void * b){
	scheda_t * x = *(scheda_t **)a, * y = *(scheda_t **)b;
	return (compar_autore(x->autore, y->autore));
}

/** Compara due stringe (editore)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo, zero se sono uguali
*/
int editore_cmp(const void * a, const void * b){
	scheda_t *x = *(scheda_t **)a, *y = *(scheda_t **)b;
	return strcmp((char *)x->pub.editore, (char *)y->pub.editore);
}

/** Compara due stringe (luogo_pubblicazione)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo, zero se sono uguali
*/
int luogo_cmp(const void * a, const void * b){
	scheda_t *x = *(scheda_t **)a, *y = *(scheda_t **)b;
	return strcmp((char *)x->pub.luogo, (char *)y->pub.luogo);
}

/** Compara due stringe (collocazione)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo, zero se sono uguali
*/
int collocazione_cmp(const void *a, const void *b){
	scheda_t *x = *(scheda_t **)a, *y = *(scheda_t **)b;
    return strcmp((char *)x->collocazione, (char *)y->collocazione);
}

/** Compara due stringe (descrizione_fisica)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo, zero se sono uguali
*/
int desc_fisica_cmp(const void *a, const void *b){
	scheda_t *x = *(scheda_t **)a, *y = *(scheda_t **)b;
    return strcmp((char *)x->descrizione_fisica, (char *)y->descrizione_fisica);
}

/** Compara due stringe (nota)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo zero, se sono uguali
*/
int nota_cmp(const void *a, const void *b){
	scheda_t *x = *(scheda_t **)a, *y = *(scheda_t **)b;
    return strcmp((char *)x->nota, (char *)y->nota);
}

/** Compara due stringe (titolo)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo, zero se sono uguali
*/
int titolo_cmp(const void *a, const void *b){
	scheda_t *x = *(scheda_t **)a, *y = *(scheda_t **)b;
    return strcmp((char *)x->titolo, (char *)y->titolo);
}

/** Elimina eventuali spazi iniziali e finali

   \param stringa puntatore a carattere

   \returns stringa
*/
char *trim(char *stringa){
	int lung = 0, fine = 0, inizio = 0;
    if (stringa != NULL) {
    	fine = lung = strlen(stringa) - 1;
    	/*Elimino eventuali spazi iniziali*/
    	while (isspace(stringa[inizio])) inizio++;
    	/*Elimino eventuali spazi finali*/
    	while (fine && isspace(stringa[fine]) && inizio != fine) fine--;
    	if (inizio != fine){
    		/*Inserisco il terminatore di stringa*/
    		if (lung != fine) *(stringa+fine+1) = '\0';
    		/*"Shifto" le posizioni dei caratteri*/
    		if (inizio != 0) stringa = stringa + inizio;
    	}else *stringa = '\0';
    }
    return stringa;
}

/** Verifica la correttezza della data

   \param g intero che descrive il giorno
   \param m intero che descrive il mese
   \param a intero che descrive l'anno

   \returns corretto TRUE se corretto altrimenti FALSE
*/
int data(int g, int m, int a){
	int corretto = FALSE;
	switch ((mese)m) {
		case FEB:
			corretto = (g > 0 && (g < 29 || (g == 29 && ((a % 4 == 0 && a % 100 != 0) || (a % 400 == 0)))));
			break;
		case GEN: case MAR: case MAG: case LUG: case AGO: case OTT:	case DIC:
			corretto = (g > 0 && g < 32);
			break;
		case APR: case GIU: case SET: case NOV:
			corretto = (g > 0 && g < 31);
			break;
	}
	return corretto;
}

/** Verifica la correttezza della strina

   \param str stringa contenente i campi del campo_t con caratteri minuscole

   \returns un valore maggiore o uguale a zero se è andato tutto bene (posizione dei campi nel campo_t)
   \returns -1 se c'è un errore
*/
int tipo_campo(char * str){
	if(str != NULL){
		if(!strcmp(str, "autore")) return AUTORE;
		if(!strcmp(str, "titolo")) return TITOLO;
		if(!strcmp(str, "editore")) return EDITORE;
		if(!strcmp(str, "luogo_pubblicazione")) return LUOGO_PUBBLICAZIONE;
		if(!strcmp(str, "anno")) return ANNO;
		if(!strcmp(str, "collocazione")) return COLLOCAZIONE;
		if(!strcmp(str, "descrizione_fisica")) return DESCRIZIONE_FISICA;
		if(!strcmp(str, "nota")) return NOTA;
		if(!strcmp(str, "prestito")) return PRESTITO;
	}
	return -1;
}

/** lista di puntatori a schede to array di puntatori a schede

   \param l lista di schede
   \param n intero numero di elementi nella lista

   \returns array_schede
*/
scheda_t ** new_array_schede(lista l, int n){
	int indice = 0;
	scheda_t **array_schede = NULL;
	if((array_schede = calloc(n, sizeof(scheda_t *))) == NULL)
		SYSTEM_CALL_FALLITA("La calloc() ha restituito null")
	for(; indice < n; indice++, l = l->next)
		array_schede[indice] = l->scheda;

	return array_schede;
}

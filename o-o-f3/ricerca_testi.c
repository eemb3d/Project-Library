/** \file ricerca_testi.c
*/
#include "comsock.h"

/** Crea la lista Testi sottoforma di schede
 *  \param  msg puntatore alla struttura messaggio contenete la richiesta di MSG_QUERY o MSG_LOAN
 *			questa non viene modificata
 *
 *  \retval l    la lista dei delle schede che soddisfano la ricerca
 *  \retval NULL   in altri casi di errore
  */
lista ricerca_testi(message_t * msg){
	char *tmp = NULL, *tmp1 = NULL, *campo = NULL, *valore = NULL;
	int num_records_tmp = 0, anno = 0;
	int corretto = TRUE; /*Conferma la presenza di tutti i valori passati rispetta alla scheda*/
	char * arr_campi[NCAMPI]; /*Stringhe relative ai valori dei campi da cercare*/
	char data[20]; /*Data in formato stringa*/
	struct tm * tm_tmp; /*Struttura tempo temporanea*/
	time_t adesso, dopo; /*Tempo corrente e tempo in cui scade la prenotazione*/
	lista l = NULL; /*Lista dei testi trovati*/
	autore_t * aut_tmp = NULL;
	scheda_t * sched = NULL;

	memset(arr_campi, '\0', NCAMPI*sizeof(char *));
	/*Se il messaggio non è vuoto*/
	if(msg != NULL && msg->buffer != NULL){
		/*Controlla se calloc del messaggio ricevuto su cui bisognerà lavorare sia corretto*/
		if((tmp = tmp1 = calloc(msg->length, sizeof(char))) == NULL) ALLOCAZIONE_MEM_FALLITA("La calloc() ha restituito null")
		else{
			strcpy(tmp, msg->buffer); campo = strtok(tmp, ":");
			while(campo != NULL){/*In ogni campo relativo, salvo il valore passato dal client per la ricerca*/
				valore = strtok(NULL, ";");	campo = trim(campo); valore = trim(valore);
				switch (tipo_campo(campo)) {
					case AUTORE : arr_campi[AUTORE] = valore; break;
					case TITOLO : arr_campi[TITOLO] = valore; break;
					case EDITORE : arr_campi[EDITORE] = valore; break;
					case LUOGO_PUBBLICAZIONE : arr_campi[LUOGO_PUBBLICAZIONE] = valore; break;
					case ANNO : arr_campi[ANNO] = valore; break;
					case COLLOCAZIONE : arr_campi[COLLOCAZIONE] = valore; break;
					case DESCRIZIONE_FISICA :arr_campi[DESCRIZIONE_FISICA] = valore; break;
					case NOTA : arr_campi[NOTA] = valore; break;
				}campo = strtok(NULL, ":");
			}
			/*Per ogni record verifico la presenza di un record con i campi passati contenuti nell'array*/
			while(num_records_tmp != num_records){
				sched = array_schede[num_records_tmp++];
				corretto = TRUE;
				/*Analizzo i campi rispetto ai record della bibblioteca*/
				if(arr_campi[AUTORE] != NULL){
					aut_tmp = sched->autore;
					/*Per ogni autore presente nella scheda si verifica se l'opzione combacia con il nome oppure con il cognome*/
					while(aut_tmp != NULL && !strstr(aut_tmp->cognome, arr_campi[AUTORE]) && !strstr(aut_tmp->nome, arr_campi[AUTORE])){
						/*Se l'autore è null vuol dire che non abbiamo trovato corrispondenza, quindi è necessario controllare un'altra scheda*/
						if((aut_tmp = aut_tmp->next) == NULL){corretto = FALSE; break;}
					}
				}
				if(corretto && arr_campi[TITOLO] && strstr(sched->titolo, arr_campi[TITOLO]) == NULL) corretto = FALSE;
				if(corretto && arr_campi[EDITORE] && strstr(sched->pub.editore, arr_campi[EDITORE]) == NULL) corretto = FALSE;
				if(corretto && arr_campi[LUOGO_PUBBLICAZIONE] && strstr(sched->pub.luogo, arr_campi[LUOGO_PUBBLICAZIONE]) == NULL) corretto = FALSE;
				if(corretto && arr_campi[ANNO] && (!(anno = strtol(arr_campi[ANNO], NULL, 10)) || sched->pub.anno != anno)) corretto = FALSE;
				if(corretto && arr_campi[COLLOCAZIONE] && strstr(sched->collocazione, arr_campi[COLLOCAZIONE]) == NULL) corretto = FALSE;
				if(corretto && arr_campi[DESCRIZIONE_FISICA] && strstr(sched->descrizione_fisica, arr_campi[DESCRIZIONE_FISICA]) == NULL) corretto = FALSE;
				if(corretto && arr_campi[NOTA] && strstr(sched->nota, arr_campi[NOTA]) == NULL) corretto = FALSE;
				if(corretto){/*Verifico per ultimo in modo tale da non dover ripristinare la scheda allo stado iniziale in caso di fallimento*/
					if(msg->type == MSG_LOAN){/*Verifico se è disponibile*/
						if(sched->prestito.disponibile == TRUE){
							adesso = time(NULL);	/*Viene creato il tempo attuale*/
							tm_tmp = localtime(&adesso);
							tm_tmp->tm_mon += 2;	/*Viene avanzato di 2 mesi*/
							dopo = mktime(tm_tmp);  /*Viene creato il tempo aggiunto di due mesi*/
							strftime(data, 15, "%d-%m-%Y", localtime(&dopo));
							if(sscanf(data, "%d-%d-%d", &sched->prestito.scadenza.tm_mday, &sched->prestito.scadenza.tm_mon, &sched->prestito.scadenza.tm_year) == 3)
								sched->prestito.disponibile = FALSE;
							else corretto = FALSE;
							if(DEBUG_ATTIVO)
								printf("Giorno: %d, Mese: %d, Anno: %d.\n", sched->prestito.scadenza.tm_mday,
										sched->prestito.scadenza.tm_mon, sched->prestito.scadenza.tm_year);
						}else corretto = FALSE;
					}
				}/*Se tutte le opzioni combaciano la scheda viene inserita nella lista*/
				if(corretto) l = insert_in_coda(l, sched);
			}/*Elimino lo spazio*/
			if(tmp1 != NULL) free(tmp1);
		}
	}
	return l;
}

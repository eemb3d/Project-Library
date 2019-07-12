/** \file errore.h

	\brief Contiene le macro che gestiscono gli errori
*/

#ifndef ERRORE_H_
#define ERRORE_H_
/*############################################ STAMPA MSG ERRORI ########################################*/
/*Stampa il messaggio di errore senza uscire*/
/**Setta errno = ENOMEM e stampa messaggio nel momento in cui c'è stato un errore nell'allocare la memoria*/
#define ALLOCAZIONE_MEM_FALLITA(str) {errno = ENOMEM; perror(str);}
/**Setta errno = EIO e stampa messaggio nel momento in cui c'è stato un errore nel cercare di aprire un file*/
#define ERRORE_APERTURA_FILE(str) {errno = EIO; perror(str);}
/**Setta errno = EINVAL e stampa messaggio nel momento in cui c'è stato un errore di conversione*/
#define ERRORE_CONVERSIONE(str) {errno = EINVAL; perror(str);}
/**Setta errno = EINVAL e stampa messaggio nel momento in cui gli argomenti non sono corretti*/
#define ERRORE_ARGOMENTI(str) {errno = EINVAL; perror(str);}

/*Stampa il messaggio di errore con uscita dal programma*/
/**Setta errno = EINVAL, stampa messaggio in quanto l'argomento non è corretto ed esce*/
#define ARGOMENTO_ERRATO(stringa) {errno = EINVAL; perror(stringa); exit(EXIT_FAILURE);}
/**Setta errno = E2BIG, stampa messaggio in quanto l'argomento non è corretto ed esce*/
#define ARGOMENTO_TOO_LONG(stringa) {errno = E2BIG; perror(stringa); exit(EXIT_FAILURE);}
/**Stampa messaggio in quanto la chiamata a funzione non è corretta ed esce*/
#define SYSTEM_CALL_FALLITA(stringa) {perror(stringa); exit(EXIT_FAILURE);}


#endif /* ERRORE_H_ */

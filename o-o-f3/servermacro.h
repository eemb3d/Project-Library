/** \file servermacro.h
 *	\brief Contiene tutti le macro relative alla terza parte del progetto
*/
#ifndef SERVERMACRO_H_
#define SERVERMACRO_H_

/*############################################### DEBUG ###############################################*/
/**Abilita la stampa di tutti i messaggi di debuging presenti nel codice compreso le funzioni di libreria*/
#define DEBUG_ATTIVO FALSE
/**Stampa un messaggio errore (il parametro passato) insieme alla localizzazione di dove è appena avvenuto l'errore*/
#define PRINT_ERRORE(str) \
	fprintf (stderr, "\n-- ERRORE -- \"%s\" -> \"%s\" -> \"%d\" -> \"%s\".\n", __FILE__, __FUNCTION__, __LINE__, str)

/*############################################### VARIE ###############################################*/
/**Nome contenente la lista delle biblioteche*/
#define NOME_BIB "./bib.conf"
/**Path relativo alla socket sia client che server*/
#define PATH_SOCKET "./tmp/%s.sck"
/**Path relativo del file log in cui scrivere il risultato della ricerca del server*/
#define NOME_LOG_FILE "%s.log"
/**Carattere di separazione per il compattamento del messaggio da spedire*/
#define CARATTERE_SEPARZIONE '@'
/**La lunghezza del file*/
#define MAX_LENGTH_FILE 255

/* SERVERMACRO_H_ */
#endif

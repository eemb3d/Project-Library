#ifndef _COMSOCK_H
#define _COMSOCK_H

/* -= TIPI =- */

/** lunghezza massima del buffer messaggio */
#define MAXBUF 2048
/** <H3>Messaggio</H3>
 * La struttura \c message_t rappresenta un messaggio 
 * - \c type rappresenta il tipo del messaggio
 * - \c length rappresenta la lunghezza in byte della parte significativa del campo \c buffer
 * - \c buffer del messaggio (puo' essere non significativo se length == 0, altrimenti sono significativo solo i primi \c length caratteri)
 *
 * <HR>
 */
typedef struct {
  /** tipo del messaggio */
    char type;           
  /** lunghezza messaggio in byte */
    unsigned int length; 
  /** buffer messaggio */
    char buffer[MAXBUF];        
} message_t; 

/** lunghezza buffer indirizzo AF_UNIX */
#define UNIX_PATH_MAX    108

/** massimo numero di tentativi di connessione specificabili nella openConnection */
#define MAXTRIAL 10
/** massimo numero secondi specificabili nella openConnection */
#define MAXSEC 10

/** numero di tentativi per connessione alle biblioteche */
#define NTRIAL 3
/** numero di secondi fra due connessioni consecutive */
#define NSEC 1

/** tipi dei messaggi scambiati fra server e client */
/** richiesta di informazioni su volumi con contenuti specifici */
#define MSG_QUERY        'Q' 
/** richiesta di prestito di volumi con contenuti specifici */
#define MSG_LOAN         'L' 
/** richiesta di connessione utente */
#define MSG_RECORD       'R' 
/** accettazione */
#define MSG_OK           'K' 
/** rifiuto */
#define MSG_NO           'N' 
/** errore */
#define MSG_ERROR        'E' 


/*#############################################################*/
/*################# In aggiunta ###############################*/
/*#############################################################*/
/*Include necessarie per la parte server*/
#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>

/*Header file della libreria bib*/
#include "./bib.h"
/*Macro relativo alla parte server*/
#include "./servermacro.h"

/** Thread worker, risponde al client con un messaggio
 * \param sk_client puntatore alla socket client
 *
 * Viene lanciato dal server affinche si occupi di rispondere al client e permettere
 * al server di ritornare in ascolto per ulteriori richieste
 */
void *worker(void * sk_client);

/** Crea la lista Testi
 *  \param  msg puntatore alla struttura messaggio contenete la richiesta di MSG_QUERY o MSG_LOAN
 *
 *  \retval l    la lista dei delle schede che soddisfano la ricerca
 *  \retval NULL   in altri casi di errore
  */
lista ricerca_testi(message_t * msg);

/**Puntatore al file di log su cui scrivere le query e le loan ricevute dal client. IL file è aperto nel server*/
FILE * file_log;
/**Struttura dati condivisa, contiene le schede caricati dal file contenente i libri in suo posseso
 * nella forma di record*/
scheda_t** array_schede;
/**Numero di record presenti nell'array schede inizializzata nel server al caricamento dei record dal file "*.bib" ed
 * è necessaria nella ricerca testi per iterare nell'array di schede nel numero esatto di schede*/
int num_records;
/**Indica l'eventuale modifica nell'array scheda, affinche si faccio la store dei record della biblioteca
 * solo quando sono stati veramente modificati*/
int prenotato;

/*#############################################################*/
/*#############################################################*/

/* -= FUNZIONI =- */

/** Crea una socket AF_UNIX
 *  \param  path pathname della socket
 *
 *  \retval s    il file descriptor della socket  (s>0)
 *  \retval -1   in altri casi di errore (setta errno)
 *               errno = E2BIG se il nome eccede UNIX_PATH_MAX
 *
 *  in caso di errore ripristina la situazione inziale: rimuove eventuali socket create e chiude eventuali file descriptor rimasti aperti
 */
int createServerChannel(char* path);

/** Chiude un canale lato server (rimuovendo la socket dal file system) 
 *   \param path path della socket
 *   \param s file descriptor della socket
 *
 *   \retval 0  se tutto ok, 
 *   \retval -1  se errore (setta errno)
 */
int closeServerChannel(char* path, int s);

/** accetta una connessione da parte di un client
 *  \param  s socket su cui ci mettiamo in attesa di accettare la connessione
 *
 *  \retval  c il descrittore della socket su cui siamo connessi
 *  \retval  -1 in casi di errore (setta errno)
 */
int acceptConnection(int s);

/** legge un messaggio dalla socket --- attenzione si richiede che il messaggio sia adeguatamente spacchettato e trasferito nella struttura msg
 *  \param  sc  file descriptor della socket
 *  \param msg  indirizzo della struttura che conterra' il messagio letto 
 *		(deve essere allocata all'esterno della funzione)
 *
 *  \retval lung  lunghezza del buffer letto, se OK 
 *  \retval  -1   in caso di errore (setta errno)
 *                 errno = ENOTCONN se il peer ha chiuso la connessione 
 *                   (non ci sono piu' scrittori sulla socket)
 *      
 */
int receiveMessage(int sc, message_t * msg);

/** scrive un messaggio sulla socket --- attenzione devono essere inviati SOLO i byte significativi del campo buffer (msg->length byte) --  si richiede che il messaggio venga scritto con un'unica write dopo averlo adeguatamente impacchettato
 *   \param  sc file descriptor della socket
 *   \param msg indirizzo della struttura che contiene il messaggio da scrivere 
 *   
 *   \retval  n    il numero di caratteri inviati (se scrittura OK)
 *   \retval -1   in caso di errore (setta errno)
 *                 errno = ENOTCONN se il peer ha chiuso la connessione 
 *                   (non ci sono piu' lettori sulla socket)
 */
int sendMessage(int sc, message_t *msg);

/** crea una connessione all socket del server. In caso di errore funzione ritenta ntrial volte la connessione (a distanza di k secondi l'una dall'altra) prima di ritornare errore.
 *   \param  path  nome del socket su cui il server accetta le connessioni
 *   \param  ntrial numeri di tentativi prima di restituire errore (ntrial <=MAXTRIAL)
 *   \param  k secondi l'uno dell'altro (k <=MAXSEC)
 *   
 *   \return fd il file descriptor della connessione
 *            se la connessione ha successo
 *   \retval -1 in caso di errore (setta errno)
 *               errno = E2BIG se il nome eccede UNIX_PATH_MAX
 *
 *  in caso di errore ripristina la situazione inziale: rimuove eventuali socket create e chiude eventuali file descriptor rimasti aperti
 */
int openConnection(char* path, int ntrial, int k);

/** Chiude una connessione
 *   \param s file descriptor della socket relativa alla connessione
 *
 *   \retval 0  se tutto ok, 
 *   \retval -1  se errore (setta errno)
 */
int closeConnection(int s);

#endif

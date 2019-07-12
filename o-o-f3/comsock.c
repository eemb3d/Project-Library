/** \file comsock.c
*/

#include "comsock.h"

/** Crea una socket AF_UNIX
 *  \param  path pathname della socket
 *
 *  \retval s    il file descriptor della socket  (s>0)
 *  \retval -1   in altri casi di errore (setta errno)ntrial
 *               errno = E2BIG se il nome eccede UNIX_PATH_MAX
 *
 *  in caso di errore ripristina la situazione inziale: rimuove eventuali socket
 *   create e chiude eventuali file descriptor rimasti aperti
 */
int createServerChannel(char* path) {
	int sfd = 0, l = 0, errore = FALSE;
	struct sockaddr_un sa;

	memset(&sa, '\0', sizeof(sa));
	/*Verifico se il parametro passato non è vuoto*/
	if (path != NULL && (l = strlen(path)) != 0) {
		strncpy(sa.sun_path, path, UNIX_PATH_MAX);
		sa.sun_family = AF_UNIX;
		if ( (errore = (l > UNIX_PATH_MAX))) errno = E2BIG;
		if ( !errore && (sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) errore = TRUE;
		if ( !errore && bind(sfd, (struct sockaddr *) &sa, sizeof(sa)) == -1) errore = TRUE;
		if ( !errore && listen(sfd, SOMAXCONN) == -1) errore = TRUE;
	}else {
		errno = EINVAL;
		if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: !(path != NULL && (l = strlen(path)) != 0)");
	}

	return (errore == TRUE) ? -1 : sfd ;
}

/** Chiude un canale lato server (rimuovendo la socket dal file system)
 *   \param path path della socket
 *   \param s file descriptor della socket
 *
 *   \retval 0  se tutto ok,
 *   \retval -1  se errore (setta errno)
 */
int closeServerChannel(char* path, int s) {
	int err = 0;
	/*Setto errno = EINVAL in quanto unlink non lo fa*/
	if(path != NULL && (err = unlink(path)) == -1 ){
		if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: unlink(path) == -1");
	}else errno = EINVAL;
	if ((err == 0) && (err = close(s)) == -1)
		if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: close(s) == -1");

	return err;
}

/** accetta una connessione da parte di un client
 *  \param  s socket su cui ci mettiamo in attesa di accettare la connessione
 *
 *  \retval  fdc il descrittore della socket su cui siamo connessi
 *  \retval  -1 in casi di errore (setta errno)
 */
int acceptConnection(int s) {
	int fdc = 0;

	if ((fdc = accept(s, NULL, 0)) == -1)
		if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: accept(s, NULL, 0) == -1");

	return fdc;
}

/** legge un messaggio dalla socket --- attenzione si richiede che il messaggio sia
 * 	adeguatamente spacchettato e trasferito nella struttura msg
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
int receiveMessage(int sc, message_t * msg) {
	int lung = 0;
	char stringa_tmp[MAXBUF];
	/*Inizializzo le variabili*/
	memset(stringa_tmp, '\0', sizeof(char));
	if((lung = read(sc, stringa_tmp, MAXBUF)) > 0){
		if(stringa_tmp[1] == CARATTERE_SEPARZIONE){
			strcpy(msg->buffer, stringa_tmp+2);
			msg->type = stringa_tmp[0];
			lung = msg->length = strlen(msg->buffer)+1;
		}else {/* Errore formattazione file */
			lung = -1; errno = EINVAL;
			if(DEBUG_ATTIVO) PRINT_ERRORE("Errore formattazione file: stringa_tmp[1] != @");
		}/* EOF */
	}else if (lung == 0){
		lung = -1; errno = ENOMSG;
	}
	return lung;
}

/** scrive un messaggio sulla socket --- attenzione devono essere inviati SOLO i byte
 * 	significativi del campo buffer (msg->length byte) --  si richiede che il messaggio venga
 *  scritto con un'unica write dopo averlo adeguatamente impacchettato
 *   \param  sc file descriptor della socket
 *   \param msg indirizzo della struttura che contiene il messaggio da scrivere
 *
 *   \retval  lung    il numero di caratteri inviati (se scrittura OK)
 *   \retval -1   in caso di errore (setta errno)
 *                 errno = ENOTCONN se il peer ha chiuso la connessione
 *                   (non ci sono piu' lettori sulla socket)
 */
int sendMessage(int sc, message_t *msg) {
	int lung = -1, l = 0;
	char stringa_send[MAXBUF];

	memset(stringa_send, '\0', sizeof(char));
	/*Non è possibile fare assunzioni ne sul tipo ne sulla lunghezza del messaggio da inviare*/
	if(msg->buffer != NULL && ((l = strlen(msg->buffer)+1) < MAXBUF-2)){
		sprintf(stringa_send, "%c%c%s", msg->type, CARATTERE_SEPARZIONE, msg->buffer);
		/*E' necessario indicare come terzo parametro il MAXBUF altrimenti in un solo
		 * messaggio, se la send è più veloce, mi scrive tutto insieme non permettendo alla
		 * receve di ricevere correwttamente i dati*/
		if((lung = write(sc, stringa_send, MAXBUF)) != (l+2)) errno = ENOTCONN;
	}else {lung = -1;}

	return lung;
}

/** crea una connessione all socket del server. In caso di errore funzione ritenta ntrial
 * 	 volte la connessione (a distanza di k secondi l'una dall'altra) prima di ritornare errore.
 *   \param  path  nome del socket su cui il server accetta le connessioni
 *   \param  ntrial numeri di tentativi prima di restituire errore (ntrial <=MAXTRIAL)
 *   \param  k secondi l'uno dell'altro (k <=MAXSEC)
 *
 *   \return fd_skt il file descriptor della connessione
 *            se la connessione ha successo
 *   \retval -1 in caso di errore (setta errno)
 *               errno = E2BIG se il nome eccede UNIX_PATH_MAX
 *
 *  in caso di errore ripristina la situazione inziale: rimuove eventuali socket create
 *   e chiude eventuali file descriptor rimasti aperti
 */
int openConnection(char* path, int ntrial, int k) {
	int fd_skt = -1, errato = FALSE, lung = 0;
	struct sockaddr_un sa;

	memset(&sa, 0, sizeof(struct sockaddr_un));

	if (path != NULL && (lung = strlen(path)) != 0 && ntrial > 0 && ntrial <= MAXTRIAL && k >= 0 && k <= MAXSEC) {
		if ((errato = (lung > UNIX_PATH_MAX))) {
			if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: strlen(path) > UNIX_PATH_MAX");
			errno = E2BIG;
		}
		if (!errato && (errato = ((fd_skt = socket(AF_UNIX, SOCK_STREAM, 0)) == -1))) {
			if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: socket(AF_UNIX, SOCK_STREAM, 0) == -1");
		}
		if (!errato){
			strncpy(sa.sun_path, path, UNIX_PATH_MAX);
			sa.sun_family = AF_UNIX;
			while (connect(fd_skt, (struct sockaddr *) &sa, sizeof(sa)) == -1 && ntrial-- != 0) {
				if (errno == ENOENT) sleep(k);
				else { fd_skt = -1; break; }
			}
		}
	}else errno = EINVAL; /* Invalid argument */

	return fd_skt;
}

/** Chiude una connessione
 *   \param s file descriptor della socket relativa alla connessione
 *
 *   \retval err  0 se tutto ok, -1  se errore (setta errno)
 */
int closeConnection(int s) {
	int err = 0;
	if( (err = shutdown(s, 2)) == -1 )
		if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: shutdown(s, 2) == -1");
	if( (err = close(s)) == -1 )
		if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: close(s) == -1");
	return err;
}

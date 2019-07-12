/** \file worker.c
*/

#include "comsock.h"
#include "threads.h"

/** Thread worker, risponde al client con un messaggio
 * \param sk_client puntatore alla socket client
 *
 * Viene lanciato dal server affinche si occupi di rispondere al client e permettere
 * al server di ritornare in ascolto per ulteriori richieste
 */
void* worker(void *sk_client) {
	message_t msg_rcv, msg_snd; /*Struttura del messaggio ricevuto ed inviato*/
	lista l = NULL, l_tmp = NULL; /*Lista contenente eventualmente i testi trova nella ricerca*/
	/*Stringa contenete la descrizione della scheda in record per poterla inviare al client*/
	char * stringa_record = NULL;
	int num_libri_trovati = 0;/*Numero di libri trovati*/
	int sk_c = (int)sk_client; /*Socket del clietn*/
	sigset_t set; /*Struttura per settare il segnale*/

	/*azzera la maschera puntata da set per non ricevere nessun segnale*/
	if(sigfillset(&set) == -1) SYSTEM_CALL_FALLITA("sigfillset fallita")
	/*Attuale è la nostra maschera dei segnali, cioè nessun segnale ascoltiamo*/
	if(pthread_sigmask(SIG_SETMASK, &set, NULL) == -1) SYSTEM_CALL_FALLITA("pthread_sigmask fallita")

	/*Inizializzo struttura*/
	memset(&msg_rcv, '\0', sizeof(message_t));
	memset(&msg_snd, '\0', sizeof(message_t));

	/*Attendo di ricevere il messaggio dal client*/
	if (receiveMessage(sk_c, &msg_rcv) == -1) {
		if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: nella receiveMessage");
		perror("server: -- Messaggio non ricevuto.\n");
	}
	/*########################## creazione messaggio di risposta ##########################*/
	msg_snd.type = MSG_RECORD;
	if(msg_rcv.buffer != NULL && msg_rcv.length > 1){
		/*Ricerca testi in mutua esclusione*/
		l_tmp = mutex_ricerca_testi(&msg_rcv);
		/*Verifica il risultato della ricerca*/
		if(l_tmp != NULL) {/*Se la ricerca ha dato dei risultati ..*/
			while(l_tmp != NULL){/*Iteriamo fino a quando non abbiamo finito di inviare i dati del libro trovato*/
				if((stringa_record = scheda_to_record(l_tmp->scheda)) != NULL){
					strcpy(msg_snd.buffer, stringa_record);
					if((msg_snd.length = strlen(msg_snd.buffer)) > 0) msg_snd.length++;
					free(stringa_record);
					/* invio messaggio di risposta al client*/
					if(sendMessage(sk_c, &msg_snd) == -1){
						if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: sendMessage.\n");
						perror("sendMessage() FALLITA");
					}/*Scrivo il messaggio nel file dei log*/
					fprintf(file_log, "%s", msg_snd.buffer); fflush(file_log);
					memset(msg_snd.buffer, '\0', sizeof(char));
					num_libri_trovati++;
				}l_tmp = l_tmp->next;
			}freeList(l);/*Libero la memoria*/
			/*Verifico se è necessario fare la store della struttura condivisa dopo la sua modifica*/
			if(msg_rcv.type == MSG_LOAN && num_libri_trovati != 0 && prenotato == FALSE) prenotato = TRUE;
		}else{/*Invio messaggio in cui la ricerca non ha prodotto nulla*/
			strcpy(msg_snd.buffer, "Nessun testo trovato!\n");
			msg_snd.length = strlen(msg_snd.buffer) + 1 ;
			msg_snd.type = MSG_NO;
			if(sendMessage(sk_c, &msg_snd) == -1) perror("sendMessage() FALLITA");
		}
	}else{/*Invio messaggio in cui c'è stato un errore*/
		strcpy(msg_snd.buffer, "Messaggio inviato è vuoto o nullo!\n");
		msg_snd.length = strlen(msg_snd.buffer) + 1 ;
		msg_snd.type = MSG_ERROR;
		if(sendMessage(sk_c, &msg_snd) == -1) perror("sendMessage() FALLITA");
	}
	/*######################################################################################*/
	/* Scrittura sul file di LOG */
	if(msg_rcv.type == MSG_QUERY) fprintf(file_log, "QUERY %d\n", num_libri_trovati);
	else if(msg_rcv.type == MSG_LOAN) fprintf(file_log, "LOAN %d\n", num_libri_trovati);
	fflush(file_log);

	/*Sottraggo uno al numero di thread attivi in mutua esclusione*/
	sottrai_num_thread_attivi();

	/* chiude la connessione */
	if( sk_c > 0 && closeConnection(sk_c) == -1){
		if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: closeConnection chiusa sockte client.\n");
		SYSTEM_CALL_FALLITA("closeConnection nel worker fallita")
	}

	pthread_exit((void *) EXIT_SUCCESS);
}

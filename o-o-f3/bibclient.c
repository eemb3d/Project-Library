/** \file bibclient.c
	\brief Il client del nostro programma
*/

/*La libreria del nostro programma*/
#include "comsock.h"

/** Main **/
int main(int argc, char** argv) {
	int err = FALSE; /*Da la possibilità al client di pulire l'ambiente nel momento in cui si presenta un errore*/
	int skt_client, index_opzione = 0, lung_arg = 0, num_opzioni = 0, opzione_replicata = FALSE, richiesta_prenotazione = FALSE;
	/*Indentificano eventuali ripetizioni delle opzioni*/
	int autore = FALSE, titolo = FALSE, editore = FALSE, luogo = FALSE, collocazione = FALSE,
			descr_fisica = FALSE, nota = FALSE, prestito = FALSE, anno = FALSE;
	/*Rispettivamente stringa relativa all'opzione ed al valore dell'opzione stessa*/
	char * opt = NULL, * v_opt = NULL;
	/*Rispettivamente Array di caratteri necessari per la formattazione del messaggio da spedire al server, nome della bib, skt_name del client*/
	char msg_send[MAXBUF], nome_bib[MAX_LENGTH_FILE], client_skt_name[UNIX_PATH_MAX];
	struct stat info; /*Struttura info file bib*/
	struct sigaction s; /*Struttura per la gestione dei segnali*/
	FILE * file_bib = NULL; /*Puntatore al file bib.conf aperta in lettura, legge le biblioteche disponibili*/
	message_t msg_snd, msg_rcv; /*Struttura dei messaggi da spedire e ricevere dal server.*/

	/* memoria */
	mtrace();
	/*######################################## Inizializza variabili ########################################*/
	memset(msg_send, '\0', sizeof(char)); /*Inizializza le stringhe dei caratteri*/
	memset(nome_bib, '\0', sizeof(char));
	memset(client_skt_name, '\0', sizeof(char));
	memset(&info, 0, sizeof(struct stat)); /*Inizializzo struttura info file bib*/
	memset(&s, 0, sizeof(struct sigaction)); /*Inizializzo struttura sigaction*/
	memset(&msg_snd, '\0', sizeof(message_t)); /*Inizializza struttura messaggi*/
	memset(&msg_rcv, '\0', sizeof(message_t));
	msg_snd.type = MSG_QUERY; /*Setto di default il messasggio come richiesta*/
	num_opzioni = argc - 1;

	/*############################ Gestione dei segnali #################################*/
	/*Setto la maschera dei segnali*/
	if(sigfillset(&s.sa_mask) == -1) SYSTEM_CALL_FALLITA("sigfillset fallita")
	s.sa_flags = 0;

	/*################################### Verifico i parametri passati #######################################*/
	if(num_opzioni == 0) ARGOMENTO_ERRATO("Deve comparire almeno una opzione")
	/*Ciclo su tutti le opzioni passate da riga di comando */
	while (++index_opzione <= num_opzioni) {
		lung_arg = strlen(argv[index_opzione]);
		/*Verifico la correttezza dell'argomento prenotazione*/
		if( lung_arg == 2 && argv[index_opzione][0] == '-' && argv[index_opzione][1] == 'p'){
			if ( richiesta_prenotazione == FALSE) {
				richiesta_prenotazione = TRUE; msg_snd.type = MSG_LOAN;
			}else {opzione_replicata = TRUE; break;}
		}/*Verifico le opzioni di ricerca del libro*/
		else if( lung_arg > 2 && (opt = strtok(argv[index_opzione], "=")) != NULL && opt[0] == '-' && opt[1] == '-'){
			if( (v_opt = strtok(NULL, "\n")) != NULL && strlen(v_opt) > 0){
				switch (tipo_campo(opt+2)) {
					case AUTORE : if(!autore) autore = TRUE; else opzione_replicata = TRUE;	break;
					case TITOLO : if(!titolo) titolo = TRUE; else opzione_replicata = TRUE; break;
					case EDITORE :if(!editore) editore = TRUE; else opzione_replicata = TRUE;	break;
					case LUOGO_PUBBLICAZIONE : if(!luogo) luogo = TRUE; else opzione_replicata = TRUE; break;
					case ANNO : if(!anno) anno = TRUE; else opzione_replicata = TRUE;	break;
					case COLLOCAZIONE : if(!collocazione) collocazione = TRUE; else opzione_replicata = TRUE; break;
					case DESCRIZIONE_FISICA : if(!descr_fisica) descr_fisica = TRUE; else opzione_replicata = TRUE; break;
					case NOTA :	if(!nota) nota = TRUE; else opzione_replicata = TRUE; break;
					case PRESTITO :	if(!prestito) prestito = TRUE; else opzione_replicata = TRUE; break;
					default: if(DEBUG_ATTIVO) PRINT_ERRORE(opt+2); break;
				}
				if(!opzione_replicata && opt+2 != NULL && v_opt != NULL){
					/*Se è la prima iterazione non è necessario avanzare di uno*/
					sprintf(msg_send, "%s: %s; ", opt+2, v_opt);
					strcat(msg_snd.buffer, msg_send);
				}else err = TRUE;
			}else ARGOMENTO_ERRATO("Valore Opzione errata")
		}else ARGOMENTO_ERRATO("Argomenti errati")
	}
	/*Verifico se ci sono delle opzioni inseriti da riga di comando ripetute*/
	if(opzione_replicata)
		{err = TRUE; errno = 22; perror("Opzione replicata");}
	/*Verifico se le informazioni inseriti sono maggiori della dimensine del buffer*/
	if(err == FALSE && (err = ((msg_snd.length = strlen(msg_snd.buffer) + 1) > MAXBUF)))
		{err = TRUE; errno = 7; perror("UNIX_PATH_MAX: dimensione file superato");}

	/*############################ Controllo il file bib.conf ######################################*/
	if(err == FALSE && (err = (stat( NOME_BIB, &info ) == -1 ))) perror("stat() fallita");
	if(err == FALSE && (err = (!S_ISREG(info.st_mode)))) {errno = 22; perror("Errore nel controllo del file bib.conf");}
	if(err == FALSE && (err = (file_bib = fopen(NOME_BIB, "r+")) == NULL)) perror("fopen fallita nell'aprire il file bib.conf");

	/*###################### Cicla nelle biblioteche alla ricerca del libro ###########################*/
	if(err == FALSE){
		while(fgets(nome_bib, MAX_LENGTH_FILE, file_bib) != NULL){
			nome_bib[strlen(nome_bib)-1] = '\0';
			sprintf(client_skt_name, PATH_SOCKET, nome_bib);
			/*Apre la connessione con il server*/
			if ((err = ((skt_client = openConnection(client_skt_name, NTRIAL, NSEC)) == -1)))
				perror("Open connection Fallita");
			/* Invio il messaggio skt e' la connessione su cui mandare il msg */
			if (err == FALSE && (err = (sendMessage(skt_client, &msg_snd) == -1)))
				perror("client: sendMessage fallita");
			/*In caso di errore viene stampato il messaggio*/
			if (DEBUG_ATTIVO)
				fprintf(stdout,"client: -- Abbiamo spedito il messaggio \"%s\" con type \"%c\" al server \"%s\".\n",
						msg_snd.buffer, msg_snd.type, client_skt_name);
			/* leggo la risposta */
			if (err == FALSE){
				while((err = receiveMessage(skt_client, &msg_rcv)) != -1){
					if(errno == ENOMSG)	break;/*Se non ci sono più messaggi interrompre il ciclo*/
					/*Stampo a video il messaggio ricevuto.*/
					fprintf(stdout,"client: -- Risposta ricevuta dal server: %s", msg_rcv.buffer); fflush(stdout);
					memset(&msg_rcv, '\0', sizeof(message_t)); /*Setta a zero la struttura per la nuova ricezione del msg*/
				}fflush(stdout); /*E' necessario fare un ulteriore flush prima di interrompere */
			}memset(client_skt_name, '\0', sizeof(char)); /*Setta a zero la stringa di caratteri per la prossima bib*/
		}
	}
	/*###################### Chiusura pulita del Client #################################*/
	/*Chiusura File*/
	if(file_bib != NULL && fclose(file_bib) == EOF )  SYSTEM_CALL_FALLITA("fclose() fallita")
	if(DEBUG_ATTIVO) fprintf(stdout,"client: -- chiusa file_bib.\n");
	/* chiude la connessione */
	if( skt_client > 0 && closeConnection(skt_client) == -1) perror("fclose() fallita");
	if(DEBUG_ATTIVO) fprintf(stdout,"client: -- chiusa sockte client.\n");

	muntrace();

	exit(err);
}

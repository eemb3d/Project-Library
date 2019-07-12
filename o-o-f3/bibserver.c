/** \file bibserver.c
	\brief Il server del nostro programma
*/

#include "comsock.h"
#include "threads.h"

/** Variabile di terminazione del server, settata dal gestore del segnale del server
 * nel momento in cui quest'ultimo riceve un segnale di SIG_PIPE e SIG_INT*/
static volatile sig_atomic_t esci;

/** Uscita pulita dal server
 *  \param  signum intero riferito al segnale
 *
 *  Permette l'uscita dal ciclo while del server nel momento in cui arriva un segnale di SIG_PIPE e SIG_INT
  */
static void gestore_segnale_server (int signum);

/** Main **/
int main(int argc, char** argv) {
	int index_record = 0; /*Indice relativo alla'array delle schede*/
	int sk = 0, sk_c = 0;/* Socket di ascolto */
	char server_socket[UNIX_PATH_MAX]; /** nome server socket */
	char stringa_log[MAX_LENGTH_FILE]; /** Stringa contenente il nome del file log */
	struct stat info; /*Struttura info file*/
	struct sigaction s; /*Struttura gestione segnale*/
	pthread_t tid = 0; /*pid del thread creato*/
	FILE * file_record = NULL;/*Puntatore al file di record (biblioteca)*/
	pthread_attr_t atr; /*Necessaria per inizializzare il pthread in modo detached*/

	/* memoria */
	mtrace();
	/*Inizializzo le variabili*/
	memset(server_socket, '\0', sizeof(char));/*Inizializzo stringhe server_socket*/
	memset(stringa_log, '\0', sizeof(char));/*Inizializzo stringhe server_socket*/
	memset(&s, 0, sizeof(struct sigaction));/* Inizializzo struttura gestore segnale a 0 */
	memset(&info, 0, sizeof(struct stat));/*Inizializzo struttura info file*/

	/*########################## Gestione dei segnali e THREAD #############################*/
	sigemptyset(&s.sa_mask); /*Setto a zero la maschera dei segnali*/
	s.sa_flags = 0;
	s.sa_handler = SIG_IGN; /*Ignoro il segnale sig_pipe*/
	if(sigaction(SIGPIPE, &s, NULL) == -1) SYSTEM_CALL_FALLITA("sigaction(SIGPIPE) fallita")
	s.sa_handler = gestore_segnale_server; /*Registro il nuovo gestore per sig_int e sig_term*/
	if(sigaction(SIGINT, &s, NULL) == -1) SYSTEM_CALL_FALLITA("sigaction(SIGINT) fallita")
	if(sigaction(SIGTERM, &s, NULL) == -1) SYSTEM_CALL_FALLITA("sigaction(SIGTERM) fallita")
	/*Crea di default pthread DETACHED affinchè dopo la terminazione non è allocata, creo un thread distaccato*/
	if (pthread_attr_init(&atr) == -1)SYSTEM_CALL_FALLITA("pthread_attr_init fallita")
	if (pthread_attr_setdetachstate(&atr, PTHREAD_CREATE_DETACHED) == -1) SYSTEM_CALL_FALLITA("pthread_attr_setdetachstate fallita")
	/*###################################################################################*/

	/*######################### Verifica dei parametri passati ##########################*/
	if( argc != 3 ) ARGOMENTO_ERRATO("Argomenti errati")
	sprintf(server_socket, PATH_SOCKET, argv[1]);/* server socket name*/
	sprintf(stringa_log, NOME_LOG_FILE, argv[1]);/* Nome file log */
	if( (strlen(server_socket)) > UNIX_PATH_MAX ) ARGOMENTO_TOO_LONG("UNIX_PATH_MAX")
	if( (strlen(stringa_log)) > MAX_LENGTH_FILE ) ARGOMENTO_TOO_LONG("MAX_LENGTH_FILE - FILE_LOG")
	if( (strlen(argv[2]) ) > MAX_LENGTH_FILE ) ARGOMENTO_TOO_LONG("MAX_LENGTH_FILE - FILE_RECORD")
	if( access( argv[2], F_OK ) == -1 ) ARGOMENTO_ERRATO("File_record inesistente")
	if( stat( argv[2], &info ) == -1 ) SYSTEM_CALL_FALLITA("stat() fallita")
	if( !S_ISREG(info.st_mode) ) ARGOMENTO_ERRATO("Argomenti errati File non regolare")
	/*###################################################################################*/

	/*###################### creazione server socket e apertura files ########################*/
	/* Creazione socket principale*/
	if ((esci = ((sk = createServerChannel(server_socket)) == -1))){
		if(DEBUG_ATTIVO) PRINT_ERRORE(server_socket);
		perror("testserv: Creazione server socket");
	}/*Apertura del file log in solo scrittura, se non esiste la crea altrimenti sovrascrive.*/
	if((esci = ((file_log = fopen(stringa_log, "w")) == NULL))){
		if(DEBUG_ATTIVO) PRINT_ERRORE(stringa_log);
		perror("fopen fallita file_log"); esci = TRUE;
	}/*Apertura del file record in sola lettura*/
	if((esci = ((file_record = fopen(argv[2], "r")) == NULL))) {
		if(DEBUG_ATTIVO) PRINT_ERRORE(argv[2]);
		perror("fopen fallita fole_record");
	}/*Carico i record presenti sul file */
	if((esci = ((num_records = load_records(file_record, &array_schede)) == -1))) {
		if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: load_records");
		perror("load_records ha fallito"); esci = TRUE;
	}
	/*###################################################################################*/

	/*###################### Ciclo "infinito" del server ###################################*/
	while(esci == FALSE){
		/*Attendo su accept*/
		if ((sk_c = acceptConnection(sk)) == -1) {
			if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: acceptConnection");
			perror("server: -- acceptConnection"); break;
		}else{
			/*Aggiungo uno al numero di thread attivi in mutua esclusione*/
			aggiungi_num_thread_attivi();
			/*Creo il thread worker per il client appena connesso*/
			if(pthread_create(&tid, &atr, &worker, (void *)sk_c) != 0){
				if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: pthread_create");
				perror("pthread_create fallita"); break;
			}
		}
	}
	/*###################################################################################*/

	/*###################### Chiusura pulita del server #################################*/
	/*Se ci sono errori si esce senza nessun controllo*/
	if(prenotato){/*Salvataggio dei nuovi dati aggiornati in base alle prenotazioni*/
		if(file_record != NULL && fclose(file_record) == EOF ) SYSTEM_CALL_FALLITA("fclose() del file_record fallita")
		if((file_record = fopen(argv[2], "w")) == NULL) SYSTEM_CALL_FALLITA("fopen() fallita nel file_record")
		if (store_records(file_record, array_schede, num_records) == -1) perror("Errore nella store_records()");
	}/*Elimino la struttura dati array_schede*/
	if(array_schede != NULL) {

		while(index_record < num_records)
			free_scheda(&array_schede[index_record++]);
		free(array_schede);
	}
	/*Attendo che tutti i client terminano correttamente*/
	verifica_terminazione_threads();
	/*Libero la memoria*/
	pthread_attr_destroy(&atr);
	/*Chiusura File file_log*/
	if(file_log != NULL && fclose(file_log) == EOF ) {
		if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: server chiusura log_file.\n");
		SYSTEM_CALL_FALLITA("fclose() del file_log fallita")
	}/*Chiusura File file_record*/
	if(file_record != NULL && fclose(file_record) == EOF ){
		if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: server chiusura file_record.\n");
		SYSTEM_CALL_FALLITA("fclose() del file_record fallita")
	}/* chiude la server socket */
	if( sk > 0 && closeServerChannel(server_socket, sk) == -1) {
		if(DEBUG_ATTIVO) PRINT_ERRORE("Errore: server chiusura sockte client.\n");
		SYSTEM_CALL_FALLITA("closeServerChannel fallita")
	}
	/*###################################################################################*/

	/* memoria */
	muntrace();

	return EXIT_SUCCESS;
}

/** Gestore del segnale nel server
 *  \param  signum intero riferito al segnale
 *
 *  Permette l'uscita dal ciclo while del server nel momento in cui arriva un segnale di SIG_PIPE e SIG_INT
 *  liberando successivamente tutte le informazioni strutture e file aperti.
  */
static void gestore_segnale_server (int signum){
	if((esci = (signum == SIGINT)))	write(1, "\nABBIAMO RICEVUTO IL SEGNALE DI SIGINT\n", 40);
	else if((esci = (signum == SIGTERM))) write(1, "\nABBIAMO RICEVUTO IL SEGNALE DI SIGTERM\n", 41);
}

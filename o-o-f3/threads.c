/** \file threads.c
	\brief Contiene tutte le funzioni che fanno uso della muta-esclusione e delle attese-condizionate
*/
#include "comsock.h"
#include "threads.h"

/**Mutex relativa ai thread attivi*/
static pthread_mutex_t mtx_threads_attivi = PTHREAD_MUTEX_INITIALIZER;
/**Condizione relativa ai thread attivi*/
static pthread_cond_t cond_threads_attivi = PTHREAD_COND_INITIALIZER;
/**Mutex sulla struttura condivisa (array_schede) in fase di scrittura*/
static pthread_mutex_t mtx_array_schede = PTHREAD_MUTEX_INITIALIZER;
/**Condizione sull'array_schede in fase di prenotazione*/
static pthread_cond_t cond_prenotazione_scheda = PTHREAD_COND_INITIALIZER;

/** Attendo che tutti i client terminano correttamente*/
void verifica_terminazione_threads(void){
	pthread_mutex_lock(&mtx_threads_attivi);
	while (numero_threads != 0)
		pthread_cond_wait(&cond_threads_attivi, &mtx_threads_attivi);
	pthread_mutex_unlock(&mtx_threads_attivi);
}
/** Rimuovo di un fattore uno il numero di thread attivi*/
void sottrai_num_thread_attivi(void){
	pthread_mutex_lock(&mtx_threads_attivi);
	numero_threads--;
	pthread_cond_signal(&cond_threads_attivi);
	pthread_mutex_unlock(&mtx_threads_attivi);
}
/** Aggiungo di un fattore uno il numero di thread attivi*/
void aggiungi_num_thread_attivi(void){
	pthread_mutex_lock(&mtx_threads_attivi);
	numero_threads++;
	pthread_mutex_unlock(&mtx_threads_attivi);
}
/** Ritorna la lista dei Testi sottoforma di schede
*   \param msg_rcv puntatore al messaggio
*
*  \retval l    la lista delle schede che soddisfano la ricerca
*  \retval NULL   in altri casi
*/
lista mutex_ricerca_testi(message_t * msg_rcv){
	lista l = NULL; /*lista dei testi trovati all'interno della biblioteca*/
	pthread_mutex_lock(&mtx_array_schede);
	while(prenotazione == TRUE)	pthread_cond_wait(&cond_prenotazione_scheda, &mtx_array_schede);
	if(msg_rcv->type == MSG_LOAN){
		/*E' necessario settare qui la variabile in quanto si rischia che di versi worker accedono nella
		 * biblioteca e solo alla fine si rendono conto che non possono accedervi.*/
		prenotazione = TRUE;
		l = ricerca_testi(msg_rcv);/*Ricavo i testi che soddisfano le richieste del cliente*/
		prenotazione = FALSE;
		pthread_cond_signal(&cond_prenotazione_scheda);
	}else l = ricerca_testi(msg_rcv);
	pthread_mutex_unlock(&mtx_array_schede);
	return l;
}

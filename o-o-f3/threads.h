/** \file threads.h
	\brief Libreria contenente i prototipi di funzioni che adoperano le "mutex*" e le "cond*"
*/

#ifndef THREADS_H_
#define THREADS_H_

#include <pthread.h>

/**Il numero di thread attivi, necessario per capire se ci sono thred ancora attivi, nel qual caso è necessario
 * attendere la loro terminazione.*/
int numero_threads;
/**Condizione sulla scheda, afinche vi si accede in mutua esclusione e non si verifichi rate conditional
 * mentre operano diversi client tra cui uno in scrittura(fase di prenotazione)*/
int prenotazione;
/** Attendo che tutti i client terminano correttamente*/
void verifica_terminazione_threads(void);
/** Rimuovo di un fattore uno il numero di thread attivi*/
void sottrai_num_thread_attivi(void);
/** Aggiungo di un fattore uno il numero di thread attivi*/
void aggiungi_num_thread_attivi(void);
/** Ritorna la lista dei Testi sottoforma di schede
*   \param msg_rcv puntatore al messaggio
*
*  \retval l    la lista delle schede che soddisfano la ricerca
*  \retval NULL   in altri casi
*/
lista mutex_ricerca_testi(message_t * msg_rcv);

#endif /* THREADS_H_ */

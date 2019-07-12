/** \file lista.h
	\brief Contiene la struttura dati e tipi della lista
*/

#ifndef LISTA_H_
#define LISTA_H_

/** Struttura nodo relativo all'emento della lista */
typedef struct nodo{
	/**Puntatore alla struttura scheda*/
	scheda_t * scheda;
	/**Puntatore al successivo elemento della lista*/
	struct nodo *next;
}nodo;

/**Nuovo tipo della lista*/
typedef nodo *lista;

/** crea una lista vuota

   \retval NULL il puntatore alla lista vuota */
lista crea_elem();

/** iserisce n nella lista l creando un nuovo nodo
   \param l  		la lista
   \param scheda	elemento da inserire

   \retval l puntatore alla lista */
lista insert_in_coda(lista l, scheda_t *scheda);

/** dealloca la lista

    \param l la lista*/
lista freeList (lista l);

#endif /* LISTA_H_ */

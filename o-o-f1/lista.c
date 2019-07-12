#include "bib.h"
/** crea una lista vuota

   \retval elemento il puntatore alla lista vuota */
lista crea_elem(){
	lista elemento = NULL;
	if((elemento = calloc(1, (sizeof(nodo)))) == NULL)
		ALLOCAZIONE_MEM_FALLITA("La calloc() ha restituito null nella creazione del nuovo nodo")
	return elemento;
}

/** iserisce n nella lista l creando un nuovo nodo
   \param l  		la lista
   \param scheda	elemento da inserire

   \retval l puntatore alla lista */
lista insert_in_coda(lista l, scheda_t *scheda){
	lista l_tmp = NULL, l_inc = NULL, elem = NULL;
	/*Creo il nuovo elemento della lista, inizializzandolo con i parametri passati*/
	if((elem = crea_elem()) == NULL) SYSTEM_CALL_FALLITA("La calloc() ha restituito null");
	elem->scheda = scheda;

	if(l == NULL) return elem;
	/*Se la lista è diversa da null la si scorre fino a trovare la coda*/
	for(l_tmp = l, l_inc = l->next; l_inc != NULL; l_tmp = l_tmp->next, l_inc = l_inc->next)
		;
	/*Aggiungo l'elemento nuovo alla lista passata.*/
	l_tmp->next = elem;

	return l;
}

/** dealloca la lista

   \param l  la lista

   \retval NULL
*/
lista freeList (lista l){
	lista lista_tmp = NULL;
	if(l != NULL){
		for(lista_tmp = l->next; lista_tmp != NULL; l = lista_tmp, lista_tmp = lista_tmp->next)
			free(l);
		/*free dell'ultimo elemento della lista o del primo se abbiamo un solo elemento.*/
		free(l);
	}
	return NULL;
}

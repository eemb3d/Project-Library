#ifndef PROTOTIPI_H_
#define PROTOTIPI_H_

/** Prototipi di funzione*/

/** trasforma un'autore in una stringa
     \param a puntatore all'autore da trasformare

    \returns stringa_autore il puntatore all'autore (allocato all'interno della funzione)
    \returns  NULL se ci sono stati problemi (setta errno)
*/
char *crea_record_autore(autore_t *a);

/** Verifica la correttezza della data

   \param g intero che descrive il giorno
   \param m intero che descrive il mese
   \param a intero che descrive l'anno

   \returns corretto TRUE se corretto altrimenti FALSE
*/
int data(int g, int m, int a);

/** Elimina eventuali spazi iniziali e finali

   \param stringa puntatore a carattere

   \returns stringa
*/
char* trim(char *stringa);

/** Compara due date (prestiti)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo zero se sono uguali
*/
int data_cmp(const void * a, const void * b);

/** Compara due interi (anno)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns la differenze dei due valori
*/
int int_cmp(const void * a, const void * b);

/** Compara due autori

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns 0 se sono uguali 1 altrimenti
*/
int autore_cmp(const void * a, const void * b);

/** Compara due stringe (editore)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo, zero se sono uguali
*/
int editore_cmp(const void * a, const void * b);

/** Compara due stringe (luogo_pubblicazione)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo, zero se sono uguali
*/
int luogo_cmp(const void * a, const void * b);

/** Compara due stringe (collocazione)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo, zero se sono uguali
*/
int collocazione_cmp(const void *a, const void *b);

/** Compara due stringe (descrizione_fisica)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo, zero se sono uguali
*/
int desc_fisica_cmp(const void *a, const void *b);

/** Compara due stringe (nota)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo zero, se sono uguali
*/
int nota_cmp(const void *a, const void *b);

/** Compara due stringe (titolo)

   \param a puntatore generico (puntatore a scheda)
   \param b puntatore generico (puntatore a scheda)

   \returns un valore maggiore di zero se il primo è più grande, minore se è più piccolo, zero se sono uguali
*/
int titolo_cmp(const void *a, const void *b);

/** Verifica la correttezza della strina

   \param str stringa contenente i campi del campo_t con caratteri minuscole

   \returns un valore maggiore o uguale a zero se è andato tutto bene (posizione dei campi nel campo_t)
   \returns -1 se c'è un errore
*/
int tipo_campo(char * str);

/** confronta due autori
    \param a1 puntatore al primo autore
    \param a2 puntatore al secondo autore

    \returns 0 se gli autori sono uguali (uguali e non nulli) e
    \returns 1 altrimenti
 */
int compar_autore(autore_t * a1, autore_t * a2);

/** Crea ed inizializza un nuovo autore

    \param a puntatore all'autore
 */
autore_t *new_autore(autore_t *a);

/** lista di puntatori a schede to array di puntatori a schede

   \param l lista di schede
   \param n intero numero di elementi nella lista

   \returns array_schede
*/
scheda_t **new_array_schede(lista l, int n);


#endif /* PROTOTIPI_H_ */

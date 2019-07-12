#ifndef _BIB__H
#define _BIB__H
#include <time.h>

/** lunghezza massima dell'etichetta di un campo (autore, titolo, etc ...)*/
#define LTAG 32
/** lunghezza massima cognome */
#define LCOGN 256
/** lunghezza massima nome */
#define LNOME 256
/** lunghezza massima anno */
#define LANNO 4
/** lunghezza massima campo stringa corto (descrizione_fisica, prestito, editore, luogo_publicazione, collocazione)*/
#define LSSTRING 64
/** lunghezza massima campo stringa lungo (titolo, nota)*/
#define LLSTRING 1024
/** vero ...*/
#define TRUE 1
/** falso ... */
#define FALSE 0

/** numero campi previsti */
#define NCAMPI 9
/** descrizione campi previsti */
typedef enum campo {AUTORE, TITOLO, EDITORE, LUOGO_PUBBLICAZIONE, ANNO, COLLOCAZIONE, DESCRIZIONE_FISICA, NOTA, PRESTITO} campo_t;

/** elemento della lista autori */
typedef struct autore {
  /** cognome/i (almeno uno deve essere presente) */
  char cognome[LCOGN +1];
  /** nome/i (almeno uno deve essere presente) */ 
  char nome [LNOME + 1];
  /** puntatore al prossimo autore */ 
  struct autore * next; 
} autore_t;


/** descrizione prestito */
typedef struct prestito {
  /** true se e' disponibile, false altrimenti */
  short int disponibile; 
  /** data di scadenza prestito */
  struct tm scadenza;    
} prestito_t;

/** descrizione pubblicazione */
typedef struct pubblicazione {
  /** nome casa editrice */
  char editore[LSSTRING+1];  
  /** luogo pubblicazione */
  char luogo[LSSTRING+1];    
  /** anno pubblicazione */
  unsigned int anno;         
} pubblicazione_t;

/** scheda bibliografica */
typedef struct scheda {
  /** puntatore lista degli autori */
  autore_t* autore;         
  /** titolo */
  char titolo[LLSTRING +1]; 
  /** dati pubblicazione */
  pubblicazione_t pub;      
  /** dati prestito */
  prestito_t prestito;      
  /** note */
  char nota[LLSTRING+1];    
  /** collocazione nella biblioteca */
  char collocazione[LSSTRING+1];  
  /** informazioni fisiche */
  char descrizione_fisica[LSSTRING+1];   
} scheda_t;

/**Inclusione delle librerie standard*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mcheck.h>
#include <ctype.h>
/**Ulteriori librerie da noi sviluppate*/
#include "macro.h"
#include "lista.h"
#include "errore.h"
#include "prototipi.h"
/**Definizione del tipo mese, utilizzato per la verifica della data*/
typedef enum mese {GEN = 1, FEB, MAR, APR, MAG, GIU, LUG, AGO, SET, OTT, NOV, DIC} mese;
/*#############################################################*/

/** crea ed inizializza una nuova scheda 
    \returns res la nuova scheda (allocata all'interno della funzione)
    \returns NULL se ci sono stati problemi (setta errno) 
*/
scheda_t * new_scheda (void);

/** libera la memoria occupata da una scheda

    \param ps puntatore al puntatore alla scheda (*ps viene settato a NULL dalla funzione)
 */
void free_scheda (scheda_t ** ps);

/** confronta due schede
    \param s1 puntatore alla prima scheda
    \param s2 puntatore alla seconda scheda

    \returns TRUE se le schede sono uguali (tutti i campi sono uguali) e
    \returns FALSE altrimenti
 */
int is_equal_scheda (scheda_t * s1, scheda_t * s2);

/** stampa la scheda in formato RECORD (vedi sopra) sullo stream specificato

    \param f stream di output
    \param s puntatore alla scheda 
 */
void print_scheda (FILE* f, scheda_t * s);

/** trasforma un record in una scheda 
    \param r record da trasformare (non viene modificato)
    \param l lunghezza massima del record (serve ad evitare l'overrun ...)

    \returns res la nuova scheda (allocata all'interno della funzione)
    \returns NULL se ci sono stati problemi (setta errno) 
                  errno=EINVAL per stringa mal formattata
*/
scheda_t * record_to_scheda (char* r, int l);

/** trasforma una scheda in un record 
    \param s la scheda da trasformare
    
    \returns r il puntatore al record che rappresenta la scheda (allocato all'interno della funzione)
    \returns  NULL se ci sono stati problemi (setta errno)
*/
char * scheda_to_record(scheda_t* s);


/** legge tutti i record da un file e li memorizza in un array -- i record malformati vengono scartati ma se c'e' almeno un record convertibile corretamente la funzione ha successo 

    \param fin stream da cui leggere i record
    \param psarray puntatore alla variabile che conterra' l'indirizzo dell'array di puntatori alle schede (allocato), significativo se n >0 

    \returns -1 se si e' verificato un errore (setta errno) 
    \returns n (n > 0) numero di record convertiti correttamente (ampiezza dell'array di schede) in questo caso *psarray contiene l'indirizzo dell'array allocato
 */
int load_records(FILE* fin, scheda_t** psarray[]);

/** scrive su file un array di schede sotto forma di record
    \param fout stream su cui scrivere i record
    \param sarray puntatore all'array di puntatori alle schede 
    \param n numero schede nell'array

    \returns -1 se si e' verificato un errore (setta errno) -- nota l'errore nella scrittura di un singolo record viene riportato nel numero di record scritti, non provoca il ritorno di (-1)
    \returns n (n > 0) numero di record scritti correttamente */
int store_records(FILE* fout, scheda_t* sarray[], int n);

/** ordina i record nell'array relativamente al campo specificato. Gli ordinamenti definiti per i vari campi sono i seguenti:
   AUTORE -- lessicografico (Cognome Nome) sulla lista degli autori
   TITOLO, EDITORE, LUOGO_PUBBLICAZIONE, COLLOCAZONE, DESCRIZIONE_FISICA, NOTA -- Lessicografico sulla stringa
   ANNO -- crescente
   PRESTITO -- crescente 

   Nota: per ordine Lessicografico si intende quello normalmente usato nella strcmp()  

   \param sarray puntatore array da ordinare (viene modificato durante l'ordinamento)
   \param n lunghezza array
   \param c campo rispetto al quale ordinare

   \returns -1 se si e' verificato un errore (setta errno) (in questo caso s e' invariato) 
   \returns 0 se tutto e' andato bene
*/
int sort_schede(scheda_t* sarray[], int n, campo_t c);

#endif

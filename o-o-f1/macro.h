/** \file macro.h
 * 	\brief Contiene tutti le macro inerenti alla prima parte del progetto relativa alla libreria
*/

#ifndef MACRO_H_
#define MACRO_H_
/*############################################### VARIE ###############################################*/
/** Dimensione massima(escluso autore) di un record.*/
#define DIM_MAX	((2 * LTAG+LLSTRING+1) + (5 * LTAG+LSSTRING+1) + LTAG+LANNO)
/** La lunghezza della data*/
#define LUNG_PRESTITO 10+1
/**In base al numero degli autori si calcola la lunghezza massima dei caratteri necessari*/
#define DIMENSIONE_AUTORE(a, dim) for(; a != NULL; dim += LTAG+LCOGN+LNOME+1, a = a->next)
/** lunghezza max della stringa letta sul file */
#define LUNG_MAX_RIGA_FILE 500
/** Ritorna la differenza di due elementi*/
#define diff(x, y) (x-y)
/** Verifica se due date sono uguali*/
#define is_equal_data(s1, s2) (!(diff(s1.tm_mday, s2.tm_mday)) && !(diff(s1.tm_mon, s2.tm_mon)) && !(diff(s1.tm_year, s2.tm_year)))

/*############################################# Manipolazione delle stringhe ##################################*/
/**Elimina eventuali spazi iniziali e finali della stringa, verificando che la stessa termini correttamente con ';'*/
#define STR_RECORD_CORRETTO(str, tmp) \
		(str && (tmp = trim(str)) && (strlen(tmp) <= DIM_MAX) && (tmp = strrchr(tmp, ';')) && tmp[1] == '\0')
/**Controlla se la lunghezza e il tipo di campo se sono corretti*/
#define TAG_CORRETTO(tag) (tag && (tag = trim(tag)) && (strlen(tag) <= LTAG) && (tipo_campo(tag)!= -1))
/**Controlla le stringhe di lunghezza 256=LNOME=LCOGN caratteri cioe il campo "nome" e "cognome"*/
#define NOMINATIVO_CORRETTO(elem) (elem && (elem = trim(elem)) && (strlen(elem) <= LNOME))
/**Almeno un autore deve essere nella scheda*/
#define AUTORE_CORRETTO(autore) (autore && NOMINATIVO_CORRETTO(autore->nome) && NOMINATIVO_CORRETTO(autore->cognome))
/**Controlla le stringhe di lunghezza LLSTRING cioe il campo "titolo" e "nota"*/
#define LLSTRING_CORRETTO(elem) (elem && (elem = trim(elem)) && (strlen(elem) <= LLSTRING))
/**Controlla le stringhe di lunghezza LSSTRING cioe il campo "editore", "luogo", "collocazione" e "descrizione_fisica"*/
#define LSSTRING_CORRETTO(elem) (elem && (elem = trim(elem)) && (strlen(elem) <= LSSTRING))
/**Controlla la stringa anno*/
#define ANNO_CORRETTO(anno, var) (anno && (anno = trim(anno)) && (strlen(anno) == LANNO) && (var = strtol(anno, NULL, 10)))
/**Controlla la stringa prestito*/
#define STRINGA_DATA_CORRETTA(d, p) (d && (d = trim(d)) && (strlen(d) < LUNG_PRESTITO) &&\
		((sscanf(d, "%d-%d-%d", &p.tm_mday, &p.tm_mon, &p.tm_year) == 3) && data(p.tm_mday, p.tm_mon, p.tm_year)))
/**Aggiunge tutti gi autori all'interno della stringa passata*/
#define AGGIUNGI_ALTRI_AUTORI(str, a)\
for(a = a->next; (a && a->nome && a->cognome); sprintf(str, "%s autore: %s, %s;", str, a->cognome, a->nome), a = a->next)

/*############################################### STAMPA ######################################################*/
/**Stampa il prestito*/
#define PRINT_PRESTITO(p) p.tm_mday, ((p.tm_mon > 9) ? ("") : ("0")), (p.tm_mon), p.tm_year
/**Stampa il record con il prestito*/
#define PARAMETRI_RECORD_CON_PRESTITO(s) s->titolo, s->pub.editore, s->pub.anno, s->pub.luogo,\
	PRINT_PRESTITO(s->prestito.scadenza), s->collocazione, s->descrizione_fisica, s->nota
/**Stampa il record senza il prestito*/
#define PARAMETRI_RECORD_SENZA_PRESTITO(s) s->titolo, s->pub.editore, s->pub.anno, s->pub.luogo, \
	s->collocazione, s->descrizione_fisica, s->nota
/**Stampa scheda vuota*/
#define PRINT_SCHEDA_VUOTA "autore: ;\ntitolo: ;\neditore: ;\nanno: ;\nluogo_pubblicazione: ;\nprestito: ;\n\
collocazione: ;\ndescrizione_fisica: ;\nnota: ;\n"

#endif /* MACRO_H_ */

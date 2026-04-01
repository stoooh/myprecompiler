/*qui ci andranno le strutture dati e prototipi di funzioni. 

header è il contratto, main usa il contratto e funcs lo implementa e coordina

Input 
Il programma prevede tre parametri di input 
-i, --in (notazione doppio trattino) per specificare il file di input
-o --out (notazione doppio trattino) per specificare il file di output su cui salvare le statistiche. Se questo parametro non viene usato, l'output e' automaticamente ridirezionato sullo stdout
-v, --verbose (notazione doppio trattino) per visualizzare sullo stdout le statistiche di elaborazione

*/

#ifndef HEADER_H //#ifndef = "Se HEADER_H non è stato definito prima"
#define HEADER_H //#define = "Allora definiscilo adesso"#include <stdio.h>
#include <stdlib.h>  //per funzioni di allocazione dinamica, gestione file, ecc.
#include <string.h> //per funzioni di manipolazione stringhe, come strcmp, strcpy, ecc.
#include <stdio.h> //per funzioni di input/output, come printf, fprintf, ecc.

#define ERR_INVALID_NAME 1
#define ERR_INVALID_TYPE 2
#define ERR_UNUSED_VAR 3

// Struttura dati per rappresentare un errore
typedef struct {
    int tipo_errore;
    int numero_riga;
} Errore;

// Struttura dati per rappresentare una variabile
typedef struct {
    char *nome; //nome della variabile
    char *tipo; //tipo della variabile (es. int, float, ecc.)
    int riga_dichiarazione; //riga in cui è stata dichiarata
    int utilizzata; //flag con 1 o 0 se usata o no
    Errore *errori; // Array dinamico di errori
    int num_errori; // contatore di errori
} Variabile;

typedef struct {
    char *file_input; //nome file da analizzare
    char *file_output; //nome file di output
    int verbose; //flag per output dettagliato (0 o 1)
} Parametri;

// Struttura dati per memorizzare le statistiche
typedef struct {
    int variabili_totali;           // Numero totale di variabili controllate
    int errori_totali;              // Numero totale di errori rilevati
    int variabili_non_utilizzate;  // Numero di variabili non utilizzate
    int nomi_non_corretti;         // Numero di nomi di variabili non corretti
    int tipi_non_corretti;         // Numero di tipi di dato non corretti
} Statistiche;



// 1. Parsing dei parametri da linea di comando (-i, -o, -v)
// Ritorna una struct Parametri riempita con i valori passati
//argv legge gli argomenti passati da terminale (array di stringhe)
Parametri* parsing_parametri(int argc, char *argv[]);

// 2. Validazione del nome di una variabile
// Ritorna 1 se valido, 0 se non valido
int is_valid_name(char *nome);

// 3. Validazione del tipo di una variabile
// Ritorna 1 se valido, 0 se non valido
int is_valid_type(char *tipo);

// 4. Lettura e analisi del file di input
// Carica tutte le variabili trovate in un array
// Ritorna l'array di Variabile e il numero totale in *num_vars
Variabile* leggi_file(char *filename, int *num_vars, Statistiche *stats);

// 5. Calcolo delle statistiche finali
// Aggiorna la struct Statistiche con i risultati
void calculate_stats(Variabile *variabili, int num_vars, Statistiche *stats);

// 6. Stampa dei risultati (su file e/o stdout)
void print_result(Statistiche *stats, Variabile *variabili, int num_vars, Parametri *param);

// 7. Liberazione memoria
void free_memory(Variabile *variabili, int num_vars, Parametri *param);

#endif

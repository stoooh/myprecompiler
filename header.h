/*qui ci andranno le strutture dati e prototipi di funzioni.

header è il contratto, main usa il contratto e funcs lo implementa e coordina

Input
Il programma prevede tre parametri di input
-i, --in (notazione doppio trattino) per specificare il file di input
-o --out (notazione doppio trattino) per specificare il file di output su cui
salvare le statistiche. Se questo parametro non viene usato, l'output e'
automaticamente ridirezionato sullo stdout -v, --verbose (notazione doppio
trattino) per visualizzare sullo stdout le statistiche di elaborazione

*/

#ifndef HEADER_H    // #ifndef = "Se HEADER_H non è stato definito prima"
#define HEADER_H    // #define = "Allora definiscilo adesso"
#include <ctype.h>  // per isalpha e isalnum
#include <stdio.h>  //per funzioni di input/output, come printf, fprintf, ecc.
#include <stdlib.h> //per funzioni di allocazione dinamica, gestione file, ecc.
#include <string.h> //per funzioni di manipolazione stringhe, come strcmp, strcpy, ecc.

// Struttura dati per rappresentare un errore
typedef struct {
  int type_error; // tipo di errore (es. nome non valido, tipo non valido,
                  // variabile non utilizzata)
  int num_line;
} Error;

// Struttura dati per rappresentare una variabile
typedef struct {
  char *name;     // nome della variabile
  char *type;     // tipo della variabile (es. int, float, ecc.)
  int line;       // riga in cui è stata dichiarata
  int used;       // flag con 1 o 0 se usata o no
  Error *errors;  // Array dinamico di errori
  int num_errors; // contatore di errori
} Variabile;

typedef struct {
  char *file_input;  // nome file da analizzare
  char *file_output; // nome file di output
  int verbose;       // flag per output dettagliato (0 o 1)
} Parameters;

// Struttura di stato del parser lexicale persistente
typedef struct {
  int in_declaration;
  int in_assegnment;
  char type_found[30];
  int is_typedef;
} ParserState;

// Struttura dati per memorizzare le statistiche
typedef struct {
  int tot_vars;      // Numero totale di variabili controllate
  int tot_errors;    // Numero totale di errori rilevati
  int vars_not_used; // Numero di variabili non utilizzate
  int names_not_ok;  // Numero di nomi di variabili non corretti
  int types_not_ok;  // Numero di tipi di dato non corretti

  // Per Traccia: memorizzare la riga esatta degli errori
  int *error_lines;
  int error_lines_count;
  int error_lines_capacity;
} Statistics;

//-------------------------------------------//

// per rappresentare una lista di tipi validi, usiamo una linked list che
// sfrutta l'allocazione dinamica creo qui una losta con nodi e puntatori (nodo
// che prende il valore della stringa)
typedef struct NodeType {
  char *types;           // Il tipo memorizzato
  struct NodeType *next; // Punta al PROSSIMO nodo
} NodeType;

// creo qui la testa della lista e un contatore
typedef struct {
  NodeType *head; // Punta al primo nodo
  int num_types;  // Conta quanti tipi
} ListType;

// gestione della lista di tipi validi
ListType *create_ListType(); // Crea una lista vuota  (per ogni tipo nuovo
                             // chiamo questa funzione)
void add_type(ListType *list, char *type); // Aggiunge un tipo alla lista
int flag_type_exists(ListType *list,
                     char *type);  // Controlla se un tipo esiste già (flag 1 o
                                   // 0) tramite ricerca volendo binaria
void freeListType(ListType *list); // Libera la memoria con free

//--------------------------------------------//

// 1. Parsing dei parametri da linea di comando (-i, -o, -v)
// Ritorna una struct Parametri riempita con i valori passati
// argv legge gli argomenti passati da terminale (array di stringhe)
Parameters *parsing_parameters(int argc, char *argv[]);

// 4. Lettura e analisi del file di input
// Carica tutte le variabili trovate in un array
// Ritorna l'array di Variabile e il numero totale in *num_vars
Variabile *read_file(char *filename, int *num_vars, Statistics *stats,
                     ListType *custom_types);

// 2. Validazione del nome di una variabile
// Ritorna 1 se valido, 0 se non valido
int is_valid_name(char *name, ListType *custom_types);

// 3. Validazione del tipo di una variabile /typedef/
// Ritorna 1 se valido, 0 se non valido
int is_valid_type(char *type, ListType *custom_types);

// Rimozione dei commenti dalla riga letta
void rmv_comments(char *line, int *in_m_comment);

// Riconosce se stiamo entrando nel corpo del codice
int is_start_of_code(char *token, Variabile *array_vars, int num_vars);

// Analizza la singola riga di codice
void analyze_line(char *line, int current_line, Variabile **array_vars_ptr,
                  int *num_vars, int *capacity_array, Statistics *stats,
                  int *reading_declarations, ListType *custom_types,
                  ParserState *p_state);

// 5. Calcolo delle statistiche finali
// Aggiorna la struct Statistiche con i risultati
void calculate_stats(Variabile *vars, int num_vars, Statistics *stats);

// 6. Stampa dei risultati (su file e/o stdout)
void print_result(Statistics *stats, Variabile *vars, int num_vars,
                  Parameters *param);

// 7. Liberazione memoria
void free_memory(Variabile *vars, int num_vars, Parameters *param);

#endif
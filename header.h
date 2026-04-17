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

// Struttura dati per rappresentare una variabile
typedef struct {
  char *name;     // nome della variabile
  char *type;     // tipo della variabile (es. int, float, ecc.)
  int line;       // riga in cui è stata dichiarata
  int used;       // flag con 1 o 0 se usata o no
  int num_errors; // contatore di errori
} Variabile;

typedef struct {
  char *file_input;  // nome file da analizzare
  char *file_output; // nome file di output
  int verbose;       // flag per output dettagliato (0 o 1)
} Parameters;

// Struttura di stato dell'analizzatore
typedef struct {
  int in_declaration;  // flag per segnare l'inizio di una dichiarazione di una
                       // variabile int, char
  int in_assegnment;   // flag per segnare l'inizio di un assegnamento di una
                       // variabile (es. =)
  char type_found[30]; // tipo trovato e salvo qui le variabili temporaneamente
                       // (int a,b,c...)
  int is_typedef;      // flag per segnare l'inizio di un typedef
} Analyser;

// Struttura dati per memorizzare le statistiche per gli errori
typedef struct {
  int tot_vars;      // Numero totale di variabili controllate
  int tot_errors;    // Numero totale di errori rilevati
  int vars_not_used; // Numero di variabili non utilizzate
  int names_not_ok;  // Numero di nomi di variabili non corretti
  int types_not_ok;  // Numero di tipi di dato non corretti

  // memorizza la riga esatta degli errori
  int *error_lines;         // array dinamico di righe d'errore
  int error_lines_count;    // numero di righe con errori
  int error_lines_capacity; // capacita' iniziale che si raddoppia dinamicamente
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

// Parsing dei parametri da linea di comando (-i, -o, -v)
// Ritorna una struct Parametri riempita con i valori passati
// argv legge gli argomenti passati da terminale (array di stringhe)
Parameters *parsing_parameters(int argc, char *argv[]);

// Lettura e analisi del file di input
// Carica tutte le variabili trovate in un array
// Ritorna l'array di Variabile e il numero totale in *num_vars
Variabile *read_file(char *filename, int *num_vars, Statistics *stats,
                     ListType *custom_types);

// Validazione del nome di una variabile
// Ritorna 1 se valido, 0 se non valido
int is_valid_name(char *name, ListType *custom_types);

// Validazione del tipo di una variabile /typedef/
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
                  Analyser *analyser);

// Calcolo delle statistiche finali
// Aggiorna la struct Statistiche con i risultati
void calculate_stats(Variabile *vars, int num_vars, Statistics *stats);

// Stampa dei risultati (su file e/o stdout)
void print_result(Statistics *stats, Variabile *vars, int num_vars,
                  Parameters *param);

// Liberazione memoria
void free_memory(Variabile *vars, int num_vars, Parameters *param);

#endif
// in questa classe ci andranno le funzioni di analisi del file.

#include "header.h"

Parameters *parsing_parameters(int argc, char *argv[]) {

  Parameters *par = malloc(sizeof(Parameters)); // memoria dinamica

  // caso base
  if (par == NULL) {
    fprintf(stderr, "Errore di allocazione memoria!\n");
    return NULL;
  }

  par->file_input = NULL; // puntatore a stringa, inizialmente NULL se non viene
                          // passato da terminale
  par->file_output = NULL;
  par->verbose = 0; // flag per output dettagliato

  for (int i = 1; i < argc;
       i++) { // argc è la lunghezza totale degli argomenti passati da terminale

    // usiamo strcmp che confronta le stringhe (primo argomento col secondo), e
    // se e' uguale a 0 sono uguali
    if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--in") == 0) {
      if (i + 1 < argc) {
        // USO STRDUP PER ALLOCARE MEMORIA E COPIARE LA STRINGA, IN MODO DA NON
        // AVERE PROBLEMI SE argv VIENE MODIFICATO O LIBERATO
        par->file_input = strdup(
            argv[i + 1]); // settiamo il file input nella posizione dopo -i
        i++;              // saltiamo il prossimo argomento (file input)
      } else {
        fprintf(stderr, "Errore: parametro -i richiede un argomento!\n");
        free(par->file_input);
        free(par->file_output);
        free(par); // liberiamo la memoria prima di uscire
        return NULL;
      }
    } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--out") == 0) {
      if (i + 1 < argc) {
        par->file_output = strdup(argv[i + 1]); // stessa cosa per il file out
        i++;
      } else {
        fprintf(stderr, "Errore: parametro -o richiede un argomento!\n");
        free(par->file_input);
        free(par->file_output);
        free(par); // liberiamo la memoria prima di uscire
        return NULL;
      }
    }
    // confronto i caratteri con strcmp, se è -v o --verbose setto il flag
    // verbose a 1
    else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
      par->verbose = 1; // Flag per output dettagliato
    }
    // strncmp confronta solo i primi n caratteri, in questo caso 3
    else if (strncmp(argv[i], "-vi", 3) == 0 ||
             strncmp(argv[i], "-iv", 3) == 0) {
      par->verbose = 1; // Flag per output dettagliato

      // estraggo il file di input dal prossimo argomento
      if (i + 1 < argc) {
        par->file_input = strdup(argv[i + 1]);
        i++;
      } else {
        fprintf(stderr, "Errore: -vi o -iv richiede un file di input!\n");
        free(par->file_input);
        free(par->file_output);
        free(par);
        return NULL;
      }
    }
  }

  return par; // ritorniamo gli argomenti parsati e i file input/output e il
              // flag verbose
}

// GESTIONE DELLA LISTA DI TIPI VALIDI, USATA PER VALIDARE I TIPI NELLE
// DICHIARAZIONI, INCLUSI I TYPEDEF PERSONALIZZATI
//-----------------------------------------------------------//

ListType *create_ListType() // crea una lista vuota
{
  ListType *list = malloc(sizeof(ListType));
  if (list) {
    list->head = NULL; // inizializza la testa a NULL
    list->num_types = 0;
  }
  return list;
}

// list è un puntatore alla lista creata dalla funzione create_ListType, type è
// la stringa del tipo da aggiungere
void add_type(ListType *list,
              char *type) // aggiunge un tipo alla lista, creando un nuovo nodo
                          // e inserendolo in testa
{
  if (!list || !type)
    return; // se la lista o il tipo sono NULL, non facciamo nulla

  NodeType *new_node =
      malloc(sizeof(NodeType)); // allochiamo memoria per un nuovo nodo
  new_node->types =
      strdup(type); // copiamo la stringa del tipo nel nodo (strdup alloca
                    // memoria e copia la stringa, così evitiamo problemi se la
                    // stringa originale viene modificata o liberata)
  new_node->next = list->head; // il nuovo nodo punta alla testa attuale della
                               // lista (inserimento in testa)
  list->head = new_node;       // la testa della lista ora è il nuovo nodo
  list->num_types++;
}

int flag_type_exists(
    ListType *list,
    char *type) // flag per controllare se un tipo esiste già nella lista,
                // ritorna 1 se esiste, 0 altrimenti
{
  if (!list || !type)
    return 0; // se la lista o il tipo sono NULL, il tipo non esiste

  NodeType *current =
      list->head; // partiamo dalla testa della lista e iteriamo fino alla fine
  while (current != NULL) {
    if (strcmp(current->types, type) ==
        0) // se troviamo il tipo nella lista, ritorniamo 1
      return 1;
    current = current->next; // altrimenti, passiamo al nodo successivo
  }
  return 0;
}

void freeListType(ListType *list) // libera la memoria allocata per la lista
{
  if (!list)
    return; // se la lista non esiste, non facciamo nulla
  NodeType *current =
      list->head; // iteriamo su tutti i nodi della lista, liberando la memoria
                  // allocata per ogni nodo e per la stringa del tipo
  while (current != NULL) {
    NodeType *temp = current;
    current = current->next;
    free(temp->types);
    free(temp);
  }
  free(list);
}

//------------------------------------------------------------//

// passiamo a rmv_comments la riga da pulire e un puntatore a un flag che indica
// se siamo dentro un commento multiriga o no, così possiamo gestire
// correttamente i commenti che si estendono su più linee
void rmv_comments(char *line, int *in_m_comment) {

  for (int i = 0; line[i] != '\0'; i++) // iteriamo finché non arriviamo alla
                                        // fine della stringa (carattere null)
  {

    if (*in_m_comment == 0) { // fuori dal commento multiriga

      if (line[i] == '/' && line[i + 1] == '/') {
        line[i] = '\0'; // tronco la stringa qui, ignoro il resto
        break;          // esco dal ciclo for, la riga è finita
      } else if (line[i] == '/' &&
                 line[i + 1] == '*') { // Commento multiriga inizia qui
        *in_m_comment = 1;
        line[i] = ' ';
        line[i + 1] = ' ';
        i++; // salto l'asterisco
      }
    } else { // dentro il commento multiriga

      if (line[i] == '*' &&
          line[i + 1] == '/') // se trovo la fine del commento multiriga
      {
        *in_m_comment = 0; // chiudo il commento
        line[i] = ' ';
        line[i + 1] = ' ';
        i++;   // salto lo slash
      } else { // se ancora non ho trovato la fine del commento, continuo a
               // pulire la riga
        line[i] =
            ' '; // pulisco con uno spazio vuoto i caratteri dentro il commento
      }
    }
  }
}

// funzione per riconoscere se un nome di una variabile è valido

// MIGLIORARE PER I TIPI COMPOSTI TIPO LONG LONG INT, CHE SONO TIPI VALIDI MA
// NON SONO RICONOSCIUTI COME TIPI STANDARD, QUINDI POTREBBERO ESSERE
// RICONOSCIUTI COME NOMI VALIDI, QUINDI DOVREMMO FARE UN CONTROLLO PIU'
// APPROFONDITO PER RICONOSCERE QUESTI CASI

int is_valid_name(char *name, ListType *custom_types) {
  if (!name || strlen(name) == 0)
    return 0;

  // Se la parola è un tipo valido (es. "int", "float"), NON è un nome valido!
  if (is_valid_type(name, custom_types))
    return 0;

  if (!isalpha(name[0]) && name[0] != '_')
    return 0;

  for (int i = 1; name[i] != '\0'; i++) {
    if (!isalnum(name[i]) && name[i] != '_')
      return 0;
  }
  return 1;
}

// funzione per riconoscere se un tipo è valido, sia standard che personalizzato
// tramite typedef
int is_valid_type(char *type, ListType *custom_types) {
  if (!type)
    return 0; // se il tipo è NULL, non è valido

  const char *standard_types[] = {
      "int",      "float",  "double", "char", "void",  "short", "long",
      "unsigned", "signed", "struct", "enum", "union", "FILE"};
  int num_std = sizeof(standard_types) /
                sizeof(standard_types[0]); // numero di tipi nell'array per
                                           // iterere correttamente

  for (int i = 0; i < num_std; i++) {
    if (strcmp(type, standard_types[i]) ==
        0) // se il tipo è uno dei tipi standard, è valido
      return 1;
  }

  return flag_type_exists(
      custom_types, type); // se non è un tipo standard, controlliamo se è un
                           // tipo personalizzato definito tramite typedef,
                           // usando la funzione flag_type_exists che controlla
                           // nella lista dei tipi personalizzati
}

// riconosce se una parola segna l'inizio del codice effettivo
int is_start_of_code(char *token, Variabile *array_vars, int num_vars) {
  if (token == NULL)
    return 0; // se il token è NULL, non è l'inizio del codice

  const char *keywords[] = {"if",     "for",    "while", "do",   "switch",
                            "return", "printf", "scanf", "else", "main"};
  int num_keywords = sizeof(keywords) /
                     sizeof(keywords[0]); // Numero di parole chiave nell'array
                                          // per iterare correttamente

  for (int i = 0; i < num_keywords; i++) {
    if (strcmp(token, keywords[i]) ==
        0) // se il token è una parola chiave che indica l'inizio del codice,
           // ritorniamo 1
      return 1;
  }

  for (int i = 0; i < num_vars; i++) {
    if (strcmp(token, array_vars[i].name) ==
        0) // se il token è il nome di una variabile già dichiarata siamo
           // nell'inizio del codice eseguibile
      return 1;
  }

  return 0;
}

// funzione che aggiunge spazi intorno a = ; , per facilitare la tokenizzazione,
// dato che potremmo avere casi come int a=5; e vogliamo tokenizzare
// correttamente a e 5, quindi trasformiamo in int a = 5 ;
void cleaned_line(char *line) {

  if (line == NULL)
    return; // se la riga è NULL, non facciamo nulla

  char temp_buffer[1024] = ""; // buffer temporaneo per la riga modificata
  int j = 0;
  for (int i = 0; line[i] != '\0'; i++) {
    if (line[i] == '=' || line[i] == ',' || line[i] == ';') {
      temp_buffer[j++] = ' ';
      temp_buffer[j++] = line[i];
      temp_buffer[j++] = ' ';
    } else {
      temp_buffer[j++] = line[i];
    }
  }
  temp_buffer[j] = '\0';     // termina la stringa
  strcpy(line, temp_buffer); // copia le modifiche nella stringa originale
}

void add_error_line(Statistics *stats, int line) {
  if (stats->error_lines_count >= stats->error_lines_capacity) {
    stats->error_lines_capacity *= 2;
    int *temp =
        realloc(stats->error_lines, stats->error_lines_capacity * sizeof(int));
    if (temp)
      stats->error_lines = temp;
  }
  stats->error_lines[stats->error_lines_count++] = line;
}

// funzione che analizza riga per riga e ritorna le variabili trovate,
// aggiornando le statistiche e gestendo il blocco dichiarazioni e il blocco
// codice eseguibile
void analyze_line(char *line, int current_line, Variabile **array_vars_ptr,
                  int *num_vars, int *capacity_array, Statistics *stats,
                  int *reading_declarations, ListType *custom_types,
                  ParserState *p_state) {

  Variabile *array_vars =
      *array_vars_ptr; // otteniamo il puntatore all'array di variabili per
                       // moodificare la grandezza dell'array se necessario

  // RIMOZIONE COMMENTI E PULIZIA DELLA RIGA NEL BLOCCO DICHIARAZIONI
  //---------------------------------------------//
  cleaned_line(line);

  const char *delimiters =
      " \t\n\r(){}[]*+-/!&|<>\"'"; // delimitatori per tokenizzazione, tranne =
                                   // ; , che gestiamo a parte
  char *token = strtok(
      line,
      delimiters); // spezza la riga in token singoli tramite i delimitatori

  if (token == NULL)
    return; // se la riga è vuota o contiene solo delimitatori, esci
  if (token[0] == '#')
    return; // se la riga è una direttiva preprocessor, ignorala (non contiene
            // dichiarazioni di variabili)

  if (*reading_declarations == 1 &&
      is_start_of_code(
          token, array_vars,
          *num_vars)) { // se siamo ancora nel blocco dichiarazioni e troviamo
                        // un token che indica l'inizio del codice eseguibile,
                        // usciamo dal blocco dichiarazioni
    *reading_declarations = 0;
  }

  // FUORI BLOCCO DICHIARAZIONI, CERCHIAMO VARIABILI USATE
  //----------------------------------------------//
  if (*reading_declarations == 0) {
    while (token != NULL) {
      // Saltiamo i simboli isolati
      if (strcmp(token, "=") == 0 || strcmp(token, ";") == 0 ||
          strcmp(token, ",") == 0) {
        token = strtok(NULL, delimiters);
        continue;
      }

      // Cerchiamo se il token corrisponde a una variabile USATA
      for (int i = 0; i < *num_vars; i++) {
        if (strcmp(token, array_vars[i].name) ==
            0) // se troviamo una variabile già dichiarata, la segniamo come
               // usata
        {
          array_vars[i].used = 1; // flag
          break; // usciamo dal ciclo for, non serve continuare a cercare
        }
      }
      token = strtok(NULL, delimiters); // passiamo al prossimo token
    }
    return; // usciamo
  }

  //---------------------------------------------// BLOCCO DICHIARAZIONI,
  //CERCHIAMO VARIABILI DICHIARATE

#define in_declaration (p_state->in_declaration)
#define in_assegnment (p_state->in_assegnment)
#define is_typedef (p_state->is_typedef)
  char *type_found = p_state->type_found;

  while (token != NULL) {
    // gestione dei simboli isolati = ; ,
    if (strcmp(token, "=") == 0) {
      in_assegnment = 1;
      token = strtok(NULL, delimiters);
      continue;
    }
    if (strcmp(token, ";") == 0) {
      in_declaration = 0;
      in_assegnment = 0;
      is_typedef = 0;
      token = strtok(NULL, delimiters);
      continue;
    }
    if (strcmp(token, ",") == 0) {
      in_assegnment = 0;
      token = strtok(NULL, delimiters);
      continue;
    }
    // Analisi della parola
    if (strcmp(token, "typedef") == 0) // flag typedef
    {
      is_typedef = 1;
      in_declaration = 1;
    } else if (in_declaration == 0) // se typedef non è stato ancora dichiarato
    {
      if (is_valid_type(token, custom_types)) // controlliamo se valido
      {
        in_declaration = 1;        // settiamo che siamo in dichiarazione
        strcpy(type_found, token); // salviamo il tipo trovato per associarlo
                                   // alle variabili che dichiareremo dopo
      } else { // altrimenti non è un tipo valido, quindi è un errore,
               // incrementiamo le statistiche e saltiamo tutti i token fino al
               // prossimo ;
        stats->tot_errors++;
        stats->types_not_ok++;
        add_error_line(stats, current_line);
        // salta tutti i token fino al prossimo ;
        while (token != NULL && strcmp(token, ";") != 0) {
          token = strtok(NULL, delimiters);
        }
      }
    } else if (in_declaration == 1 &&
               in_assegnment ==
                   0) // se siamo in dichiarazione e non in assegnazione
    {
      if (is_typedef == 1) // se stiamo dichiarando un typedef
      {
        if (!is_valid_type(
                token,
                NULL)) // aggiungiamo il nuovo tipo alla lista dei tipi validi
          add_type(custom_types, token);
      } else {

        if (isdigit(token[0])) {
          // Se è solo un numero puro, ignoralo
          int is_pure_number = 1;
          for (int k = 1; token[k] != '\0'; k++) {
            if (!isdigit(token[k]) && token[k] != '.' && token[k] != 'f') {
              is_pure_number = 0;
              break;
            }
          }
          if (is_pure_number) {
            token = strtok(NULL, delimiters);
            continue;
          }
          // altrimenti cade nel controllo is_valid_name sotto, che darà errore
        }

        if (is_valid_name(token, custom_types)) {

          // aggiunto dato che ci dava main come variabile dichiarata, ma in
          // realtà è la funzione principale che segna l'inizio
          //  del codice eseguibile, quindi usiamo questo flag per uscire dal
          //  blocco dichiarazioni e non contare main come variabile
          if (strcmp(token, "main") == 0) {
            in_declaration =
                0; // Ignoriamo main, non è una variabile da salvare
            type_found[0] = '\0';
            break; // Interrompe il parsing di questa riga, così ignora
                   // argomenti come int argc che creano falsi positivi
          }

          stats->tot_vars++; // Incrementa il contatore totale di variabili
                             // trovate

          // Array dinamico per le variabili
          if (*num_vars >= *capacity_array) {
            *capacity_array *= 2;
            Variabile *temp_array =
                realloc(array_vars, (*capacity_array) * sizeof(Variabile));
            if (temp_array == NULL) {
              fprintf(stderr,
                      "Errore di allocazione memoria: realloc fallito!\n");
              return; // Esci evitando di impostare array_vars a NULL
            }
            array_vars = temp_array;
            *array_vars_ptr = array_vars;
          }

          array_vars[*num_vars].name = strdup(token);
          array_vars[*num_vars].type = strdup(type_found);
          array_vars[*num_vars].line = current_line;
          array_vars[*num_vars].used = 0;
          (*num_vars)++;
        } else {
          stats->tot_errors++;
          stats->names_not_ok++;
          add_error_line(stats, current_line);
        }
      }
    }
    token = strtok(NULL, delimiters);
  }

#undef in_declaration
#undef in_assegnment
#undef is_typedef
}

Variabile *read_file(char *filename, int *num_vars, Statistics *stats,
                     ListType *custom_types) {
  FILE *file = fopen(filename, "r"); // apriamo il file in lettura

  if (file == NULL) {
    fprintf(stderr, "Errore: impossibile aprire il file '%s'\n", filename);
    return NULL;
  }

  // Controlliamo se il file è vuoto
  fseek(file, 0, SEEK_END);
  if (ftell(file) == 0) {
    fprintf(stderr, "Errore: il file %s e' vuoto!\n", filename);
    fclose(file);
    return NULL;
  }
  rewind(file);

  // array dinamico per salvare le variabili (token) trovati
  int capacity_array = 20; // verrà espanso dinamicamente se necessario,
                           // partiamo con una capacità iniziale di 20 variabili
  Variabile *array_vars = malloc(capacity_array * sizeof(Variabile));

  *num_vars = 0;        // contatore di variabili trovate, inizialmente 0
  int current_line = 0; // per salvare dove si trova la variabile o l'errore

  char line[256];
  int in_m_comment =
      0; // flag per capire se siamo dentro un commento multilinea

  int reading_declarations =
      1; // siamo nel blocco dichiarazioni (assunzione che tutte le
         // dichiarazioni siano all'inizio del file, prima di qualsiasi codice
         // eseguibile)

  ParserState p_state = {
      0, 0, "", 0}; // Mette a zero lo stato del parser lexicale persistente per
                    // poter varcare i confini della riga

  while (fgets(line, sizeof(line), file) != NULL) {
    current_line++; // Incrementa il contatore di linee per l'errore

    rmv_comments(line, &in_m_comment); // puntatore a in_m_comment per
                                       // modificare il flag dentro la funzione

    // passiamo &array_vars (puntatore a puntatore) e &capacity_array
    analyze_line(line, current_line, &array_vars, num_vars, &capacity_array,
                 stats, &reading_declarations, custom_types,
                 &p_state); // analizza la riga e aggiorna l'array di variabili
                            // e le statistiche
  }

  // Errore di lettura file (corrotto o problemi I/O)
  if (ferror(file)) {
    fprintf(stderr,
            "Errore: si è verificato un errore durante la lettura del file!\n");
  }

  if (fclose(file) == EOF) {
    fprintf(stderr, "Errore durante la chiusura del file!\n");
  }
  return array_vars; // Ritorniamo il puntatore all'array di variabili trovate
}

void calculate_stats(Variabile *vars, int num_vars, Statistics *stats) {
  for (int i = 0; i < num_vars; i++) {
    if (vars[i].used == 0) {
      stats->vars_not_used++;
      stats->tot_errors++;
      add_error_line(stats, vars[i].line);
    }
  }
}

void print_result(Statistics *stats, Variabile *vars, int num_vars,
                  Parameters *param) {

  if (param->file_output != NULL) {
    FILE *file_out = fopen(param->file_output, "w");
    if (file_out) {
      fprintf(file_out, "=== RESOCONTO ANALISI ===\n");
      fprintf(file_out, "Variabili trovate: %d\n", stats->tot_vars);
      fprintf(file_out, "Errori totali: %d\n", stats->tot_errors);
      fprintf(file_out, " - Nomi non validi: %d\n", stats->names_not_ok);
      fprintf(file_out, " - Tipi non validi: %d\n", stats->types_not_ok);
      fprintf(file_out, " - Variabili non usate: %d\n", stats->vars_not_used);
      fprintf(file_out, "=========================\n");

      fprintf(file_out, "\n[TRACCIA] --- CRONOLOGIA ERRORI ---\n");
      for (int e = 0; e < stats->error_lines_count; e++) {
        fprintf(file_out, "Rilevato errore alla RIGA %d nel file.\n",
                stats->error_lines[e]);
      }
      fprintf(file_out, "\n[TRACCIA] --- VARIABILI NON USATE ---\n");
      for (int i = 0; i < num_vars; i++) {
        if (vars[i].used == 0)
          fprintf(file_out, "Nome: %s\n", vars[i].name);
      }

      // Se nel file vogliamo anche i dettagli, stampiamoli
      fprintf(file_out, "\n--- DETTAGLIO GENERALE VARIABILI ---\n");
      for (int i = 0; i < num_vars; i++) {
        fprintf(
            file_out,
            "Riga %d      | Tipo: %-10s      | Nome: %-15s      | Stato: %s\n",
            vars[i].line, vars[i].type, vars[i].name,
            vars[i].used ? "USATA" : "NON USATA [!]");
      }

      // Controllo errori di scrittura su file come richiesto da specifiche
      if (ferror(file_out)) {
        fprintf(stderr, "Errore: si è verificato un problema durante la "
                        "scrittura sul file di output.\n");
      }

      if (fclose(file_out) == EOF) {
        fprintf(stderr,
                "Errore: problema durante la chiusura del file di output!\n");
      } else {
        printf("Analisi completata! Risultati salvati in '%s'\n",
               param->file_output);
      }
    } else {
      fprintf(stderr, "Errore: impossibile creare il file di output.\n");
    }
  }

  if (param->verbose == 1 || param->file_output == NULL) {
    printf("\n=== RESOCONTO ANALISI ===\n");
    printf("Variabili trovate: %d\n", stats->tot_vars);
    printf("Errori totali: %d\n", stats->tot_errors);
    printf(" - Nomi non validi: %d\n", stats->names_not_ok);
    printf(" - Tipi non validi: %d\n", stats->types_not_ok);
    printf(" - Variabili non usate: %d\n", stats->vars_not_used);
    printf("=========================\n");

    printf("\n[TRACCIA] --- CRONOLOGIA ERRORI ---\n");
    for (int e = 0; e < stats->error_lines_count; e++) {
      printf("Rilevato errore alla RIGA %d nel file.\n", stats->error_lines[e]);
    }
    printf("\n[TRACCIA] --- VARIABILI NON USATE ---\n");
    for (int i = 0; i < num_vars; i++) {
      if (vars[i].used == 0)
        printf("Nome: %s\n", vars[i].name);
    }

    if (param->verbose == 1) {
      printf("\n--- DETTAGLIO GENERALE VARIABILI ---\n");
      for (int i = 0; i < num_vars; i++) {
        printf(
            "Riga %d      | Tipo: %-10s      | Nome: %-15s      | Stato: %s\n",
            vars[i].line, vars[i].type, vars[i].name,
            vars[i].used ? "USATA" : "NON USATA [!]");
      }
    }
  }
}

void free_memory(Variabile *vars, int num_vars, Parameters *param) {
  for (int i = 0; i < num_vars;
       i++) // liberiamo dalla memoria dinamica i nomi e i tipi delle variabili
  {
    if (vars[i].name)
      free(vars[i].name);
    if (vars[i].type)
      free(vars[i].type);
  }
  free(vars);

  if (param) // liberiamo dalla memoria dinamica i file in input e output
  {
    if (param->file_input)
      free(param->file_input);
    if (param->file_output)
      free(param->file_output);
    free(param);
  }
}

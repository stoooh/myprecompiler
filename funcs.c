// in questa classe ci andranno le funzioni di analisi del file.

#include "header.h"

// Parsing dei parametri da linea di comando (-i, -o, -v)
// Ritorna una struct Parametri riempita con i valori passati
// argv legge gli argomenti passati da terminale (array di stringhe)
Parameters *parsing_parameters(int argc, char *argv[]) {

  // memoria dinamica per gestire i comandi nel terminale
  Parameters *par = malloc(sizeof(Parameters));

  // caso base
  if (par == NULL) {
    fprintf(stderr, "Errore di allocazione memoria!\n");
    return NULL;
  }

  par->file_input = NULL; // puntatore a stringa del file input da analizzare
  par->file_output =
      NULL; // puntatore a stringa del file output dove salvare le statistiche
  par->verbose = 0; // flag per output dettagliato (0 o 1)

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

// crea una lista vuota
ListType *create_ListType() {
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

// flag per controllare se un tipo esiste già nella lista
int flag_type_exists(ListType *list, char *type) {
  if (!list || !type)
    return 0; // se la lista o il tipo sono NULL, il tipo non esiste

  NodeType *current =
      list->head; // partiamo dalla testa della lista e iteriamo fino alla fine
  while (current != NULL) { // finchè non arriviamo alla fine della lista
    if (strcmp(current->types, type) ==
        0) // se troviamo il tipo nella lista, ritorniamo 1
      return 1;
    current = current->next; // altrimenti, passiamo al nodo successivo
  }
  return 0; // se non troviamo il tipo nella lista, ritorniamo 0
}

// libera la memoria allocata per la lista
void freeListType(ListType *list) {
  if (!list)
    return; // se la lista non esiste, non facciamo nulla
  NodeType *current =
      list->head; // iteriamo su tutti i nodi della lista, liberando la memoria
                  // allocata per ogni nodo e per la stringa del tipo
  while (current != NULL) {
    NodeType *temp = current; // creo un nodo temporaneo per liberare la memoria
                              // del nodo corrente
    current = current->next;  // passo al nodo successivo
    free(temp->types);        // libero la memoria del tipo
    free(temp);               // libero la memoria del nodo
  }
  free(list); // libero la memoria della lista
}

// passiamo a rmv_comments la riga da pulire e un puntatore a un flag che indica
// se siamo dentro un commento multiriga o no, così possiamo gestire
// correttamente i commenti che si estendono su più linee
void rmv_comments(char *line, int *in_m_comment) {

  for (int i = 0; line[i] != '\0';
       i++) // iteriamo finché non arriviamo alla fine della stringa
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
int is_valid_name(char *name, ListType *custom_types) {

  if (!name || strlen(name) == 0) // se il nome è NULL o vuoto, non è valido
    return 0;

  if (is_valid_type(name,
                    custom_types)) // se la parola è un tipo valido (es. "int",
                                   // "float"), NON è un nome valido!
    return 0;

  if (!isalpha(name[0]) &&
      name[0] !=
          '_') // il primo carattere deve essere una lettera o un underscore
    return 0;

  for (int i = 1; name[i] != '\0';
       i++) { // resto dei caratteri deve essere alfanumerico o un underscore
    if (!isalnum(name[i]) && name[i] != '_')
      return 0;
  }
  return 1; // se tutti i controlli sono passati, il nome è valido
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

  return 0; // se non è una parola chiave né il nome di una variabile, non è
            // l'inizio del codice
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

// funzione per memorizzare i numeri di riga in cui ci sono errori (dinamica)
void add_error_line(Statistics *stats, int line) {
  if (stats->error_lines_count >= stats->error_lines_capacity) {
    stats->error_lines_capacity *=
        2; // se serve riallocare raddoppio la capacità
    int *temp =
        realloc(stats->error_lines,
                stats->error_lines_capacity *
                    sizeof(int)); // alloco memoria per il nuovo array di errori
    if (temp)
      stats->error_lines = temp;
  }
  stats->error_lines[stats->error_lines_count++] =
      line; // aggiungo il numero di riga se c'è un errore
            // incrementando il contatore di errori
}

// funzione principale che analizza riga per riga e ritorna le variabili
// trovate, aggiornando le statistiche e gestendo il blocco dichiarazioni e il
// blocco codice eseguibile
void analyze_line(char *line, int current_line, Variabile **array_vars_ptr,
                  int *num_vars, int *capacity_array, Statistics *stats,
                  int *reading_declarations, ListType *custom_types,
                  Analyser *analyser) {

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

  // BLOCCO DICHIARAZIONI, CERCHIAMO VARIABILI DICHIARATE
  //---------------------------------------------//

#define in_declaration                                                         \
  (analyser->in_declaration) // uso di un #define per evitare di scrivere
                             // analyser-> ogni volta
#define in_assegnment (analyser->in_assegnment) // idem
#define is_typedef (analyser->is_typedef)       // idem
  char *type_found = analyser->type_found;      // idem ma con puntatore

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
    //-----------------------------------------------//

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
                   0) // se siamo in dichiarazione e NON in assegnazione,
                      // es. int x,y,z (qui in_assegnment è 0)
    {
      if (is_typedef == 1) // se stiamo dichiarando un typedef
      {
        if (!is_valid_type(
                token,
                NULL)) // aggiungiamo il nuovo tipo alla lista dei tipi validi
          add_type(custom_types, token);
      } else {

        if (isdigit(token[0])) { // il primo carattere del token è un numero ->
                                 // controlliamo se è un numero puro
          int is_pure_number =
              1; // flag a 1 se il token è composto solo da numeri
          for (int k = 1; token[k] != '\0';
               k++) { // controlliamo i caratteri successivi
            if (!isdigit(token[k]) && token[k] != '.' &&
                token[k] !=
                    'f') {        // se non è un numero, punto o 'f' (es. 3.14f)
              is_pure_number = 0; // flag a 0
              break;
            }
          }
          if (is_pure_number) { // se è un numero puro, ignoralo e passa al
                                // prossimo token
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
            *capacity_array *= 2; // raddoppiamo la capacità
            Variabile *temp_array = realloc(
                array_vars, (*capacity_array) *
                                sizeof(Variabile)); // riallochiamo la memoria
            if (temp_array == NULL) { // se la memoria non è stata allocata
              fprintf(stderr,
                      "Errore di allocazione memoria: realloc fallito!\n");
              return; // Esci evitando di impostare array_vars a NULL
            }
            array_vars = temp_array; //"travasiamo" gli oggetti di array_vars in
                                     // temp_array (liberiamo memoria)
            *array_vars_ptr =
                array_vars; // usiamo array_vars puntato da array_vars_ptr per
                            // non perdere l'array allocato
          }

          array_vars[*num_vars].name = strdup(
              token); // copiamo il token (allocazione dinamica con strdup)
          array_vars[*num_vars].type = strdup(
              type_found); // copiamo il tipo (allocazione dinamica con strdup)
          array_vars[*num_vars].line = current_line; // salviamo la riga
          array_vars[*num_vars].used = 0; // inizializziamo il flag a 0
          (*num_vars)++; // incrementiamo il numero di variabili trovate

        } else { // altrimenti non è un nome valido

          stats->tot_errors++;   // incrementiamo il numero totale di errori
          stats->names_not_ok++; // incrementiamo il numero di nomi di variabili
                                 // non corretti
          add_error_line(stats, current_line); // aggiungiamo la riga agli
                                               // errori
        }
      }
    }
    token = strtok(NULL, delimiters); // passiamo al prossimo token
  }

#undef in_declaration // liberiamo i define
#undef in_assegnment
#undef is_typedef
}

// funzione che legge il file e restituisce un array di variabili
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
  rewind(file); // riporta il puntatore del file all'inizio

  // array dinamico per salvare le variabili (token) trovati
  int capacity_array = 20; // verrà espanso dinamicamente se necessario,
                           // partiamo con una capacità iniziale di 20 variabili
  Variabile *array_vars = malloc(capacity_array * sizeof(Variabile));

  *num_vars = 0;        // contatore di variabili trovate, inizialmente 0
  int current_line = 0; // per salvare dove si trova la variabile o l'errore

  char line[256]; // array di caratteri per memorizzare la riga letta dal file
  int in_m_comment =
      0; // flag per capire se siamo dentro un commento multilinea

  int reading_declarations =
      1; // siamo nel blocco dichiarazioni (assunzione che tutte
         // le dichiarazioni siano all'inizio del file,
         // prima di qualsiasi codice eseguibile)

  Analyser analyser = {0, 0, "",
                       0}; // Mette a zero lo stato del parser lexicale
                           // persistente per poter varcare i confini della riga

  // cicliamo per ogni riga del file IMPORTANTE**
  while (fgets(line, sizeof(line), file) != NULL) {
    current_line++; // Incrementa il contatore di linee per l'errore

    rmv_comments(line, &in_m_comment); // puntatore a in_m_comment per
                                       // modificare il flag dentro la funzione

    // passiamo &array_vars (puntatore a puntatore) e &capacity_array
    analyze_line(line, current_line, &array_vars, num_vars, &capacity_array,
                 stats, &reading_declarations, custom_types,
                 &analyser); // analizza la riga e aggiorna l'array di variabili
                             // e le statistiche
  }

  // Errore di lettura file (corrotto o problemi I/O)
  if (ferror(file)) { // controlla se c'è stato un errore durante la lettura
    fprintf(stderr,
            "Errore: si è verificato un errore durante la lettura del file!\n");
  }

  if (fclose(file) ==
      EOF) { // chiude il file e verifica che la chiusura sia andata a buon fine
    fprintf(stderr, "Errore durante la chiusura del file!\n");
  }

  return array_vars; // Ritorniamo il puntatore all'array di variabili trovate
}

// calcolo statistice di variabili non usate e totale errori
void calculate_stats(Variabile *vars, int num_vars, Statistics *stats) {
  for (int i = 0; i < num_vars; i++) {
    if (vars[i].used == 0) {  // se la variabile non è usata
      stats->vars_not_used++; // incrementa il contatore di variabili non usate
      stats->tot_errors++;    // incrementa il contatore di errori totali
      add_error_line(stats, vars[i].line); // aggiunge la riga agli errori
    }
  }
}

// funzione che stampa i risultati
void print_result(Statistics *stats, Variabile *vars, int num_vars,
                  Parameters *param) {

  if (param->file_output != NULL) {
    FILE *file_out =
        fopen(param->file_output, "w"); // apre il file in scrittura
    if (file_out) { // se il file è stato aperto con successo
      fprintf(file_out, "=== RESOCONTO ANALISI ===\n");
      fprintf(file_out, "Variabili trovate: %d\n", stats->tot_vars);
      fprintf(file_out, "Errori totali: %d\n", stats->tot_errors);
      fprintf(file_out, " - Nomi non validi: %d\n", stats->names_not_ok);
      fprintf(file_out, " - Tipi non validi: %d\n", stats->types_not_ok);
      fprintf(file_out, " - Variabili non usate: %d\n", stats->vars_not_used);
      fprintf(file_out, "=========================\n");

      fprintf(file_out, "\n--- CRONOLOGIA ERRORI ---\n");
      for (int e = 0; e < stats->error_lines_count;
           e++) { // scorre tutte le righe con errori
        fprintf(file_out, "Rilevato errore alla RIGA %d nel file.\n",
                stats->error_lines[e]);
      }
      fprintf(file_out, "\n--- VARIABILI NON USATE ---\n");
      for (int i = 0; i < num_vars; i++) { // scorre tutte le variabili
        if (vars[i].used == 0)
          fprintf(file_out, "Nome: %s\n", vars[i].name);
      }

      // Se nel file vogliamo anche i dettagli, stampiamoli
      fprintf(file_out, "\n--- DETTAGLIO GENERALE VARIABILI ---\n");
      for (int i = 0; i < num_vars; i++) { // scorre tutte le variabili
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

  if (param->verbose == 1 ||
      param->file_output ==
          NULL) { // se verbose è 1 o file_output è NULL, stampa su terminale
    printf("\n=== RESOCONTO ANALISI ===\n");
    printf("Variabili trovate: %d\n", stats->tot_vars);
    printf("Errori totali: %d\n", stats->tot_errors);
    printf(" - Nomi non validi: %d\n", stats->names_not_ok);
    printf(" - Tipi non validi: %d\n", stats->types_not_ok);
    printf(" - Variabili non usate: %d\n", stats->vars_not_used);
    printf("=========================\n");

    printf("\n--- CRONOLOGIA ERRORI ---\n");
    for (int e = 0; e < stats->error_lines_count;
         e++) { // scorre tutte le righe con errori
      printf("Rilevato errore alla RIGA %d nel file.\n", stats->error_lines[e]);
    }
    printf("\n--- VARIABILI NON USATE ---\n");
    for (int i = 0; i < num_vars; i++) { // scorre tutte le variabili
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

// funzione che libera la memoria
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

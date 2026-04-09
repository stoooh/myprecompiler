//in questa classe ci andranno le funzioni di analisi del file.

//cominciano con il parsing dei parametri da linea di comando

#include "header.h" 

Parameters* parsing_parameters(int argc, char *argv[]) {
    
    Parameters *par = malloc(sizeof(Parameters)); // memoria dinamica

    //caso base
    if (par == NULL) {
        fprintf(stderr, "Errore di allocazione memoria!\n");
        return NULL;
    }

    par->file_input = NULL;
    par->file_output = NULL;
    par->verbose = 0;    //flag per outut dettagliato
    
    for (int i = 1; i < argc; i++) {  //argc sono la lunghezza totale degli argomenti passati da terminale
        
        //usiamo strcmp che confronta le stringe (primo argomento col secondo), e se e' uguale a 0 OK
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--in") == 0) {
            if (i + 1 < argc) { 
                //USO STRDUP PER ALLOCARE MEMORIA E COPIARE LA STRINGA, IN MODO DA NON AVERE PROBLEMI SE argv VIENE MODIFICATO O LIBERATO
                par->file_input = strdup(argv[i + 1]);     //settiamo il file input tramite puntatore nella posizione dopo -i 
                i++; // saltiamo il prossimo argomento
            } else {
                fprintf(stderr, "Errore: parametro -i richiede un argomento!\n");
                free(par);
                return NULL;
            }
        } 
        else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--out") == 0) {
            if (i + 1 < argc) {
                par->file_output = strdup(argv[i + 1]);   //stessa cosa per il file out  
                i++; 
            } else {
                fprintf(stderr, "Errore: parametro -o richiede un argomento!\n");
                free(par);
                return NULL;
            }
        } 
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) { //confronto i caratteri con strcmp
            par->verbose = 1;
        }
    }

    return par;
}

//GESTIONE DELLA LISTA DI TIPI VALIDII, USATA PER VALIDARE I TIPI NELLE DICHIARAZIONI, INCLUSI I TYPEDEF PERSONALIZZATI
//-----------------------------------------------------------// */

ListType* create_ListType() {
    ListType *list = malloc(sizeof(ListType));
    if (list) {
        list->head = NULL;
        list->num_types = 0;
    }
    return list;
}

void add_type(ListType *list, char *type) {
    if (!list || !type) return;
    NodeType *new_node = malloc(sizeof(NodeType));
    new_node->types = strdup(type);
    new_node->next = list->head; 
    list->head = new_node;
    list->num_types++;
}

int flag_type_exists(ListType *list, char *type) {
    if (!list || !type) return 0;
    NodeType *current = list->head;
    while (current != NULL) {
        if (strcmp(current->types, type) == 0) return 1;
        current = current->next;
    }
    return 0;
}

void freeListType(ListType *list) {
    if (!list) return;
    NodeType *current = list->head;
    while (current != NULL) {
        NodeType *temp = current;
        current = current->next;
        free(temp->types);
        free(temp);
    }
    free(list);
}


//------------------------------------------------------------//

void rmv_comments(char *line, int *in_m_comment) {
    for (int i = 0; line[i] != '\0'; i++) {
            
        if (*in_m_comment == 0) { // fuori dal commento multiriga
                
            if (line[i] == '/' && line[i + 1] == '/') {
                line[i] = '\0'; // tronco la stringa qui, ignoro il resto
                break;          // esco dal ciclo for, la riga è finita
            }         
            else if (line[i] == '/' && line[i + 1] == '*') {
                *in_m_comment = 1;
                line[i] = ' ';
                line[i + 1] = ' ';
                i++; //salto l'asterisco
            }
        } else { // dentro il commento multiriga
                
            if (line[i] == '*' && line[i + 1] == '/') {
                *in_m_comment = 0; // chiudo il commento
                line[i] = ' ';   
                line[i + 1] = ' '; 
                i++; // salto lo slash
            } else {
                line[i] = ' '; // pulisco con uno spazio vuoto i caratteri dentro il commento
            }
        }
    }
}

//funzione per riconoscere se un nome di una variabile è valido
int is_valid_name(char *name) {
    if (!name || strlen(name) == 0) return 0;
    
    if (!isalpha(name[0]) && name[0] != '_') return 0;
    
    for (int i = 1; name[i] != '\0'; i++) {
        if (!isalnum(name[i]) && name[i] != '_') return 0;
    }
    return 1;
}
//funzione per riconoscere se un tipo è valido, sia standard che personalizzato tramite typedef
int is_valid_type(char *type, ListType *custom_types) {
    if (!type) return 0;
    
    const char *standard_types[] = {"int", "float", "double", "char", "void", "short", "long", "unsigned", "signed", "struct", "enum", "union", "size_t", "FILE"};
    int num_std = sizeof(standard_types) / sizeof(standard_types[0]); 
    
    for (int i = 0; i < num_std; i++) {
        if (strcmp(type, standard_types[i]) == 0) return 1;
    }
    
    return flag_type_exists(custom_types, type);
}

// riconosce se una parola segna l'inizio del codice effettivo
int is_start_of_code(char *token, Variabile *array_vars, int num_vars) {
    if (token == NULL) return 0;
    
    const char *keywords[] = {"if", "for", "while", "do", "switch", "return", "printf", "scanf", "else"};
    int num_keywords = sizeof(keywords) / sizeof(keywords[0]);
    
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(token, keywords[i]) == 0) return 1;
    }
    
    for (int i = 0; i < num_vars; i++) {
        if (strcmp(token, array_vars[i].name) == 0) return 1;
    }
    return 0;
}

//funzione che analizza riga per riga e ritorna le variabili trovate, aggiornando le statistiche e gestendo il blocco dichiarazioni e il blocco codice eseguibile
void analyze_line(char *line, int current_line, Variabile **array_vars_ptr, int *num_vars, int *capacity_array, Statistics *stats, int *reading_declarations, ListType *custom_types) {

    Variabile *array_vars = *array_vars_ptr; // otteniamo il puntatore all'array di variabili per moodificare la grandezza dell'array se necessario

    //aggiungiamo degli spazi intorno all'operatore di assegnazione = dato che potremmo avere casi come int a=5; e vogliamo tokenizzare correttamente a e 5
    
    //---------------------------------------------// PULIZIA DELLA RIGA PER TOKENIZZAZIONE, AGGIUNGENDO SPAZI INTORNO A =    
    char cleaned_line[512] = ""; // buffer temporaneo per la riga modificata
    int j = 0; 
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '=') {
            cleaned_line[j++] = ' '; // spazio prima di =
            cleaned_line[j++] = '='; // l'operatore =
            cleaned_line[j++] = ' '; // spazio dopo =
        } else {
            cleaned_line[j++] = line[i]; // copia il carattere originale
        }
    }
    cleaned_line[j] = '\0'; // termina la stringa
    //strcpy(line, cleaned_line); // copia la riga modificata indietro a line


    const char *delimiters = " \t\n(){}[]*+-/!&|<>\"'"; // delimitatori per tokenizzazione, tranne = ; , che gestiamo a parte
    char *token = strtok(cleaned_line, delimiters); // otteniamo il primo token dalla riga pulita

    if(token == NULL) return; // se la riga è vuota o contiene solo delimitatori, esci

    if(*reading_declarations == 1 && is_start_of_code(token, array_vars, *num_vars)) { // se siamo ancora nel blocco dichiarazioni e troviamo un token che indica l'inizio del codice eseguibile, usciamo dal blocco dichiarazioni
        *reading_declarations = 0;
    }


    //FUORI BLOCCO DICHIARAZIONI, CERCHIAMO VARIABILI USATE
    //----------------------------------------------//
    if(*reading_declarations == 0) {  //se siamo fuori dal blocco dichiarazioni controlliamo che ci siano variabili usate in questa riga, e se sì settiamo il flag used a 1
        while (token != NULL){
            int len = strlen(token);
            if (len > 0 && (token[len - 1] == ';' || token[len - 1] == ',' || token[len - 1] == '=')) { //se finisce con ; , = 
                token[len - 1] = '\0'; //lo rimuoviamo per analizzarlo
            }
            if (strlen(token) > 0) { //se non è un token vuoto dopo la rimozione del delimitatore, lo analizziamo per vedere se è una variabile usata
                for (int i = 0; i < *num_vars; i++) {
                    if (strcmp(token, array_vars[i].name) == 0) {
                        array_vars[i].used = 1; //se usato, settiamo il flag a 1
                        break;
                    }
                }
            }
            token = strtok(NULL, delimiters);
        }
        return;
    }


    //---------------------------------------------// BLOCCO DICHIARAZIONI, CERCHIAMO VARIABILI DICHIARATE

    
    int in_declaration = 0; // flag per capire se siamo in una dichiarazione di variabile
    int in_assegnment = 0; // flag per capire se siamo in un'assegnazione di variabile
    char type_found[30] = ""; //salviamo il tipo trovato
    int is_typedef = 0; // flag per capire se stiamo dichiarando un typedef
    while (token != NULL) { 
        int len = strlen(token);
        
        // Gestione dell'uguale
        if (strchr(token, '=') != NULL) {
            in_assegnment = 1;
            if (strcmp(token, "=") == 0) { 
                token = strtok(NULL, delimiters); 
                continue; 
            }
        }

        // Pulisce punteggiatura attaccata a fine parola
        char last_char = token[len - 1];
        int have_comma = (last_char == ','), have_semicolon = (last_char == ';');
        if (have_comma || have_semicolon) { token[len - 1] = '\0'; len--; }
        
        if (len == 0) { 
            if (have_semicolon) { in_declaration = 0; in_assegnment = 0; is_typedef = 0; }
            else if (have_comma) { in_assegnment = 0; }
            token = strtok(NULL, delimiters); 
            continue;
        }

        // Analisi della parola
        if (strcmp(token, "typedef") == 0) { 
            is_typedef = 1; 
            in_declaration = 1; 
        }
        else if (in_declaration == 0) { 
            if (is_valid_type(token, custom_types)) { 
                in_declaration = 1; 
                strcpy(type_found, token); 
            } 
            else { 
                stats->tot_errors++; 
                stats->types_not_ok++; 
                break; //aggiunto cosi ignora il resto della riga
            }
        } 
        else if (in_declaration == 1 && in_assegnment == 0) { 
            if (is_typedef == 1) {
                if (!is_valid_type(token, NULL)) add_type(custom_types, token);
            } 
            else {

                if (isdigit(token[0])) continue; //ignora i numeri ad esempio dimensioni di array

                if (is_valid_name(token)) {

                    //aggiunto dato che ci dava main come variabile dichiarata, ma in realtà è la funzione principale che segna l'inizio 
                    // del codice eseguibile, quindi usiamo questo flag per uscire dal blocco dichiarazioni e non contare main come variabile
                    if (strcmp(token, "main") == 0) {
                        *reading_declarations = 0; // Il main segna l'inizio del codice!
                        in_declaration = 0;
                        continue; // Saltiamo questa parola
                    }

                    stats->tot_vars++;  // Incrementa il contatore totale di variabili trovate
                    
                    // Array dinamico per le variabili
                    if (*num_vars >= *capacity_array) {
                        *capacity_array *= 2;
                        array_vars = realloc(array_vars, (*capacity_array) * sizeof(Variabile));
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
                }
            }
        }

        // Fine del token: chiudiamo eventuali stati
        if (have_semicolon) { 
            in_declaration = 0; 
            in_assegnment = 0; 
            is_typedef = 0; 
        } else if (have_comma) { 
            in_assegnment = 0; 
        }
        
        token = strtok(NULL, delimiters); 
    }
}


Variabile* read_file(char *filename, int *num_vars, Statistics *stats, ListType *custom_types) {
    FILE *file = fopen(filename, "r");//apriamo il file in lettura

    if (file == NULL) {
        fprintf(stderr, "Errore: impossibile aprire il file '%s'\n", filename);
        return NULL;
    }
    //array dinamico per salvare le variabili (token) trovati
    int capacity_array = 20; // verrà espanso dinamicamente se necessario, partiamo con una capacità iniziale di 20 variabili
    Variabile *array_vars = malloc(capacity_array * sizeof(Variabile));
    
    
    *num_vars = 0; //contatore di variabili trovate, inizialmente 0
    int current_line = 0; // per salvare dove si trova la variabile o l'errore

    char line[256];
    int in_m_comment = 0; // flag per capire se siamo dentro un commento multilinea

    int reading_declarations = 1; //siamo nel blocco dichiarazioni (assunzione che tutte le dichiarazioni siano all'inizio del file, prima di qualsiasi codice eseguibile)

    while (fgets(line, sizeof(line), file) != NULL) {
        current_line++; // Incrementa il contatore di linee per l'errore

        rmv_comments(line, &in_m_comment); //puntatore a in_m_comment per modificare il flag dentro la funzione
        
        //passiamo &array_vars (puntatore a puntatore) e &capacity_array
        analyze_line(line, current_line, &array_vars, num_vars, &capacity_array, stats, &reading_declarations, custom_types); //analizza la riga e aggiorna l'array di variabili e le statistiche

    }

    fclose(file);
    return array_vars; // Ritorniamo il puntatore all'array di variabili trovate
}


void calculate_stats(Variabile *vars, int num_vars, Statistics *stats) {
    for (int i = 0; i < num_vars; i++) {
        if (vars[i].used == 0) {
            stats->vars_not_used++;
            stats->tot_errors++; 
        }
    }
}

void print_result(Statistics *stats, Variabile *vars, int num_vars, Parameters *param) {
    FILE *out = stdout; 
    
    if (param->file_output != NULL) {
        out = fopen(param->file_output, "w");
        if (!out) {
            fprintf(stderr, "Errore: file output invalido. Stampo a video.\n");
            out = stdout;
        }
    }

    fprintf(out, "\n=== RESOCONTO ANALISI ===\n");
    fprintf(out, "Variabili trovate: %d\n", stats->tot_vars);
    fprintf(out, "Errori totali: %d\n", stats->tot_errors);
    fprintf(out, " - Nomi non validi: %d\n", stats->names_not_ok);
    fprintf(out, " - Tipi non validi: %d\n", stats->types_not_ok);
    fprintf(out, " - Variabili non utilizzate: %d\n", stats->vars_not_used);
    fprintf(out, "=========================\n\n");

    if (param->verbose == 1 || out != stdout) {
        fprintf(out, "--- DETTAGLIO VARIABILI ---\n");
        for (int i = 0; i < num_vars; i++) {
            fprintf(out, "Riga %d | Tipo: %-10s | Nome: %-15s | Stato: %s\n", 
                    vars[i].line, 
                    vars[i].type, 
                    vars[i].name, 
                    vars[i].used ? "USATA" : "NON USATA [!]");
        }
    }

    if (out != stdout) {
        fclose(out);
        printf("Analisi completata con successo! Salvata in '%s'\n", param->file_output);
    }
}

// ============================================================================
// 7. PULIZIA MEMORIA
// ============================================================================
void free_memory(Variabile *vars, int num_vars, Parameters *param) {
    for (int i = 0; i < num_vars; i++) {
        if (vars[i].name) free(vars[i].name);
        if (vars[i].type) free(vars[i].type);
    }
    free(vars);

    if (param) {
        if (param->file_input) free(param->file_input);
        if (param->file_output) free(param->file_output);
        free(param);
    }
}



/* todo:
Problema di Tokenizzazione (Virgole e Punti e Virgola): Hai tolto , e ; dalla stringa delimiters di strtok per gestirli "a mano" guardando l'ultimo carattere della parola. Questo funziona per int a, b; (con gli spazi). Ma se nel file c'è int a,b; (senza spazi), strtok prenderà a,b; come unico token. Togliendo il ; finale, rimarrà a,b. La funzione is_valid_name lo segnerà come errore (perché contiene la virgola).

Soluzione rapida: Così come hai fatto per l'operatore =, ti conviene aggiungere degli spazi prima e dopo le virgole e i punti e virgola nella fase di pulizia della riga cleaned_line. In questo modo strtok farà tutto il lavoro sporco per te.

⚠️ Mancanze rispetto alla Traccia (Logica)
Tracciamento delle righe degli errori: La traccia specifica chiaramente: "Per ogni errore rilevato, il numero di riga nel file". Nel tuo header.h hai predisposto correttamente una struct Error, ma in funcs.c ti limiti ad incrementare i contatori generali (stats->tot_errors++). Se trovi un tipo o un nome sbagliato, non stai salvando da nessuna parte in quale riga si trova, e di conseguenza in print_result non lo stampi.

Output combinato (-o e -v): La traccia dice che se l'utente lancia myPreCompiler -i file.c -o out.txt -v, il programma deve sia salvare sul file, sia stampare sullo stdout. Attualmente, in print_result, se imposti un file_output, il puntatore out diventa il file testuale e tutte le fprintf scrivono solo lì dentro. Lo schermo rimarrà vuoto. Devi sdoppiare le stampe se entrambi i flag sono attivi.

💡 Consigli per l'ottimizzazione
Falsi positivi sulle variabili usate: Fuori dal blocco dichiarazioni, controlli se un token corrisponde a un nome di variabile per segnarla come used = 1. Fai attenzione: se dichiari una variabile int i; e poi nel codice c'è una stringa come "ciao i", se la tokenizzazione cattura la i da sola, la conterà come usata. Per il livello di un progetto universitario va benissimo questa soluzione rudimentale, ma è un "edge case" da tenere a mente.

La funzione strdup: È un'ottima funzione, ma ricorda che fa parte dello standard POSIX e non dello standard C puro (ANSI C/C90). Se all'esame ti obbligano a compilare con flag stringenti come gcc -std=c89 -pedantic, potrebbe darti un warning. Nel caso, basta implementarsi una piccola funzioncina con malloc e strcpy.

*/
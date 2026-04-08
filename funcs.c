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
                par->file_input = argv[i + 1];     //settiamo il file input tramite puntatore nella posizione dopo -i
                i++; // saltiamo il prossimo argomento
            } else {
                fprintf(stderr, "Errore: parametro -i richiede un argomento!\n");
                free(par);
                return NULL;
            }
        } 
        else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--out") == 0) {
            if (i + 1 < argc) {
                par->file_output = argv[i + 1];   //stessa cosa per il file out
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

void analyze_line(char *line, int current_line, Variabile **array_vars_ptr, int *num_vars, int *capacity_array, Statistics *stats) {

    Variabile *array_vars = *array_vars_ptr; // otteniamo il puntatore all'array di variabili per moodificare la grandezza dell'array se necessario

    const char *delimiters = " \t\n(){}[]*+-/!&|<>\"'"; // delimitatori per tokenizzazione, tranne = ; , che gestiamo a parte
    char *token = strtok(line, delimiters); // otteniamo il primo token

    int in_declaration = 0; // flag per capire se siamo in una dichiarazione di variabile
    int in_assegnment = 0; // flag per capire se siamo in un'assegnazione di variabile
    char type_found[30]; //salviamo il tipo trovato
    
    while (token != NULL) { 

        int len = strlen(token);
        char last_char = token[len - 1];
        int have_comma = (last_char == ',');
        int have_semicolon = (last_char == ';');

        if(have_comma || have_semicolon){ // se il token finisce con , o ;, lo rimuoviamo per analizzarlo correttamente
            token[len - 1] = '\0';  
        }

        if(strcmp(token, "=") == 0) {   // se troviamo un =, entriamo in un'assegnazione
            in_assegnment = 1; 

        } else if(in_assegnment == 1) { // siamo dentro un'assegnazione

            if (have_semicolon) {  // se troviamo un ;, usciamo dall'assegnazione e dalla dichiarazione
                in_declaration = 0; 
                in_assegnment = 0;  
            } else if (have_comma) { //se troviamo una virgola, usciamo solo dall'assegnazione ma restiamo nella dichiarazione (es. int a = 5, b = 10;)
                in_assegnment = 0;  
            }

        } else if(in_declaration == 0) { // se non siamo in una dichiarazione, cerchiamo un tipo valido per iniziare una nuova dichiarazione

            if(is_valid_type(token)){ 
                in_declaration = 1;
                strcpy(type_found, token); 
            }

        } else { // siamo in una dichiarazione, analizziamo il token
            
            if(is_valid_name(token)){

                stats->tot_vars++;  //incrementiamo il contatore totale di variabili trovate

                // se l'array è pieno, raddoppiamo la sua capacità
                if (*num_vars >= *capacity_array) {
                    *capacity_array *= 2;
                    array_vars = realloc(array_vars, capacity_array * sizeof(Variabile));
                    if (array_vars == NULL) {
                        fprintf(stderr, "Errore di allocazione memoria!\n");
                        exit(1);
                    }
                    *array_vars_ptr = array_vars; // aggiorniamo il puntatore all'array di variabili dopo la realloc
                }

                array_vars[*num_vars].name = strdup(token); 
                array_vars[*num_vars].type = strdup(type_found); 
                array_vars[*num_vars].line = current_line; 
                array_vars[*num_vars].used = 0; 
                array_vars[*num_vars].errors = NULL; 
                array_vars[*num_vars].num_errors = 0; 

                (*num_vars)++; 
            }
            else if(strlen(token) > 0) //non usiamo la variabile len perché potrebbe essere stata modificata se il token finiva con , o ;, e non vogliamo contare come errore i token vuoti dopo la rimozione
            {
                stats->tot_errors++;
                stats->names_not_ok++;
            }

            if(have_semicolon){ 
                in_declaration = 0; 
            } 
        }

        //passiamo al prossimo token
        token = strtok(NULL, delimiters); 
    }
}

Variabile* read_file(char *filename, int *num_vars, Statistics *stats) {
    FILE *file = fopen(filename, "r");//apriamo il file in lettura

    //caso base
    if (file == NULL) {
        fprintf(stderr, "Errore: impossibile aprire il file '%s'\n", filename);
        return NULL;
    }

    //array dinamico per salvare le variabili (token) trovati
    int capacity_array = 100; // partiamo con spazio per 100 variabili, possiamo espandere se necessario
    Variabile *array_vars = malloc(capacity_array * sizeof(Variabile));
    
    
    *num_vars = 0; //contatore di variabili trovate, inizialmente 0
    int current_line = 0; // per salvare dove si trova la variabile o l'errore

    char line[256];
    int in_m_comment = 0; // flag per capire se siamo dentro un commento multilinea

    while (fgets(line, sizeof(line), file) != NULL) {
        current_line++; // Incrementa il contatore di linee per l'errore

        rmv_comments(line, &in_m_comment); //puntatore a in_m_comment per modificare il flag dentro la funzione
        
        //passiamo &array_vars (puntatore a puntatore) e &capacity_array
        analyze_line(line, current_line, &array_vars, num_vars, &capacity_array, stats); //analizza la riga e aggiorna l'array di variabili e le statistiche

    }

    fclose(file);
    return array_vars; // Ritorniamo il puntatore all'array di variabili trovate
}


//TODO: gesitre i valori delle variabili usate (es. String = "ciao ciao ciao")
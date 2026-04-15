// in questa classe ci andranno le funzioni di analisi del file.

// cominciano con il parsing dei parametri da linea di comando

#include "header.h"

Parameters *parsing_parameters(int argc, char *argv[])
{

    Parameters *par = malloc(sizeof(Parameters)); // memoria dinamica

    // caso base
    if (par == NULL)
    {
        fprintf(stderr, "Errore di allocazione memoria!\n");
        return NULL;
    }

    par->file_input = NULL;
    par->file_output = NULL;
    par->verbose = 0; // flag per output dettagliato

    for (int i = 1; i < argc; i++)
    { // argc sono la lunghezza totale degli argomenti passati da terminale

        // usiamo strcmp che confronta le stringe (primo argomento col secondo), e se e' uguale a 0 OK
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--in") == 0)
        {
            if (i + 1 < argc)
            {
                // USO STRDUP PER ALLOCARE MEMORIA E COPIARE LA STRINGA, IN MODO DA NON AVERE PROBLEMI SE argv VIENE MODIFICATO O LIBERATO
                par->file_input = strdup(argv[i + 1]); // settiamo il file input tramite puntatore nella posizione dopo -i
                i++;                                   // saltiamo il prossimo argomento
            }
            else
            {
                fprintf(stderr, "Errore: parametro -i richiede un argomento!\n");
                free(par);
                return NULL;
            }
        }
        else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--out") == 0)
        {
            if (i + 1 < argc)
            {
                par->file_output = strdup(argv[i + 1]); // stessa cosa per il file out
                i++;
            }
            else
            {
                fprintf(stderr, "Errore: parametro -o richiede un argomento!\n");
                free(par);
                return NULL;
            }
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
        { // confronto i caratteri con strcmp
            par->verbose = 1;
        }
    }

    return par;
}

// GESTIONE DELLA LISTA DI TIPI VALIDII, USATA PER VALIDARE I TIPI NELLE DICHIARAZIONI, INCLUSI I TYPEDEF PERSONALIZZATI
//-----------------------------------------------------------// */

ListType *create_ListType()
{
    ListType *list = malloc(sizeof(ListType));
    if (list)
    {
        list->head = NULL;
        list->num_types = 0;
    }
    return list;
}

void add_type(ListType *list, char *type)
{
    if (!list || !type)
        return;
    NodeType *new_node = malloc(sizeof(NodeType));
    new_node->types = strdup(type);
    new_node->next = list->head;
    list->head = new_node;
    list->num_types++;
}

int flag_type_exists(ListType *list, char *type)
{
    if (!list || !type)
        return 0;
    NodeType *current = list->head;
    while (current != NULL)
    {
        if (strcmp(current->types, type) == 0)
            return 1;
        current = current->next;
    }
    return 0;
}

void freeListType(ListType *list)
{
    if (!list)
        return;
    NodeType *current = list->head;
    while (current != NULL)
    {
        NodeType *temp = current;
        current = current->next;
        free(temp->types);
        free(temp);
    }
    free(list);
}

//------------------------------------------------------------//

void rmv_comments(char *line, int *in_m_comment)
{
    for (int i = 0; line[i] != '\0'; i++)
    {

        if (*in_m_comment == 0)
        { // fuori dal commento multiriga

            if (line[i] == '/' && line[i + 1] == '/')
            {
                line[i] = '\0'; // tronco la stringa qui, ignoro il resto
                break;          // esco dal ciclo for, la riga è finita
            }
            else if (line[i] == '/' && line[i + 1] == '*')
            {
                *in_m_comment = 1;
                line[i] = ' ';
                line[i + 1] = ' ';
                i++; // salto l'asterisco
            }
        }
        else
        { // dentro il commento multiriga

            if (line[i] == '*' && line[i + 1] == '/')
            {
                *in_m_comment = 0; // chiudo il commento
                line[i] = ' ';
                line[i + 1] = ' ';
                i++; // salto lo slash
            }
            else
            {
                line[i] = ' '; // pulisco con uno spazio vuoto i caratteri dentro il commento
            }
        }
    }
}

// funzione per riconoscere se un nome di una variabile è valido

// MIGLIORARE PER I TIPI COMPOSTI
//  Aggiungi custom_types ai parametri
int is_valid_name(char *name, ListType *custom_types)
{
    if (!name || strlen(name) == 0)
        return 0;

    // Se la parola è un tipo valido (es. "int", "float"), NON è un nome valido!
    if (is_valid_type(name, custom_types))
        return 0;

    if (!isalpha(name[0]) && name[0] != '_')
        return 0;

    for (int i = 1; name[i] != '\0'; i++)
    {
        if (!isalnum(name[i]) && name[i] != '_')
            return 0;
    }
    return 1;
}

// funzione per riconoscere se un tipo è valido, sia standard che personalizzato tramite typedef
int is_valid_type(char *type, ListType *custom_types)
{
    if (!type)
        return 0;

    const char *standard_types[] = {"int", "float", "double", "char", "void", "short", "long", "unsigned", "signed", "struct", "enum", "union", "FILE"};
    int num_std = sizeof(standard_types) / sizeof(standard_types[0]);

    for (int i = 0; i < num_std; i++)
    {
        if (strcmp(type, standard_types[i]) == 0)
            return 1;
    }

    return flag_type_exists(custom_types, type);
}

// riconosce se una parola segna l'inizio del codice effettivo
int is_start_of_code(char *token, Variabile *array_vars, int num_vars)
{
    if (token == NULL)
        return 0;

    const char *keywords[] = {"if", "for", "while", "do", "switch", "return", "printf", "scanf", "else", "main"};
    int num_keywords = sizeof(keywords) / sizeof(keywords[0]);

    for (int i = 0; i < num_keywords; i++)
    {
        if (strcmp(token, keywords[i]) == 0)
            return 1;
    }

    for (int i = 0; i < num_vars; i++)
    {
        if (strcmp(token, array_vars[i].name) == 0)
            return 1;
    }
    return 0;
}

// funzione che analizza riga per riga e ritorna le variabili trovate, aggiornando le statistiche e gestendo il blocco dichiarazioni e il blocco codice eseguibile
void analyze_line(char *line, int current_line, Variabile **array_vars_ptr, int *num_vars, int *capacity_array, Statistics *stats, int *reading_declarations, ListType *custom_types)
{

    Variabile *array_vars = *array_vars_ptr; // otteniamo il puntatore all'array di variabili per moodificare la grandezza dell'array se necessario

    // aggiungiamo degli spazi intorno all'operatore di assegnazione = dato che potremmo avere casi come int a=5; e vogliamo tokenizzare correttamente a e 5

    //---------------------------------------------// PULIZIA DELLA RIGA PER TOKENIZZAZIONE, AGGIUNGENDO SPAZI INTORNO A =
    char cleaned_line[1024] = ""; // buffer temporaneo per la riga modificata
    int j = 0;
    for (int i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == '=' || line[i] == ',' || line[i] == ';')
        {
            cleaned_line[j++] = ' ';
            cleaned_line[j++] = line[i];
            cleaned_line[j++] = ' ';
        }
        else
        {
            cleaned_line[j++] = line[i];
        }
    }
    cleaned_line[j] = '\0'; // termina la stringa
    // strcpy(line, cleaned_line); // copia la riga modificata indietro a line

    const char *delimiters = " \t\n\r(){}[]*+-/!&|<>\"'"; // delimitatori per tokenizzazione, tranne = ; , che gestiamo a parte
    char *token = strtok(cleaned_line, delimiters);       // otteniamo il primo token dalla riga pulita

    if (token == NULL)
        return; // se la riga è vuota o contiene solo delimitatori, esci
    if (token[0] == '#')
        return; // se la riga è una direttiva preprocessor, ignorala (non contiene dichiarazioni di variabili)

    if (*reading_declarations == 1 && is_start_of_code(token, array_vars, *num_vars))
    { // se siamo ancora nel blocco dichiarazioni e troviamo un token che indica l'inizio del codice eseguibile, usciamo dal blocco dichiarazioni
        *reading_declarations = 0;
    }

    // FUORI BLOCCO DICHIARAZIONI, CERCHIAMO VARIABILI USATE
    //----------------------------------------------//
    if (*reading_declarations == 0)
    {
        while (token != NULL)
        {
            // Saltiamo i simboli isolati
            if (strcmp(token, "=") == 0 || strcmp(token, ";") == 0 || strcmp(token, ",") == 0)
            {
                token = strtok(NULL, delimiters);
                continue;
            }

            // Cerchiamo se il token corrisponde a una variabile nota
            for (int i = 0; i < *num_vars; i++)
            {
                if (strcmp(token, array_vars[i].name) == 0)
                {
                    array_vars[i].used = 1;
                    break;
                }
            }
            token = strtok(NULL, delimiters);
        }
        return;
    }

    //---------------------------------------------// BLOCCO DICHIARAZIONI, CERCHIAMO VARIABILI DICHIARATE

    int in_declaration = 0;   // flag per capire se siamo in una dichiarazione di variabile
    int in_assegnment = 0;    // flag per capire se siamo in un'assegnazione di variabile
    char type_found[30] = ""; // salviamo il tipo trovato
    int is_typedef = 0;       // flag per capire se stiamo dichiarando un typedef
    while (token != NULL)
    {
        // 1. Gestione dei simboli isolati
        if (strcmp(token, "=") == 0)
        {
            in_assegnment = 1;
            token = strtok(NULL, delimiters);
            continue;
        }
        if (strcmp(token, ";") == 0)
        {
            in_declaration = 0;
            in_assegnment = 0;
            is_typedef = 0;
            token = strtok(NULL, delimiters);
            continue;
        }
        if (strcmp(token, ",") == 0)
        {
            in_assegnment = 0;
            token = strtok(NULL, delimiters);
            continue;
        }
        // Analisi della parola
        if (strcmp(token, "typedef") == 0)
        {
            is_typedef = 1;
            in_declaration = 1;
        }
        else if (in_declaration == 0)
        {
            if (is_valid_type(token, custom_types))
            {
                in_declaration = 1;
                strcpy(type_found, token);
            }
            else
            {
                stats->tot_errors++;
                stats->types_not_ok++;
                // salta tutti i token fino al prossimo ;
                while (token != NULL && strcmp(token, ";") != 0)
                {
                    token = strtok(NULL, delimiters);
                }
            }
        }
        else if (in_declaration == 1 && in_assegnment == 0)
        {
            if (is_typedef == 1)
            {
                if (!is_valid_type(token, NULL))
                    add_type(custom_types, token);
            }
            else
            {

                if (isdigit(token[0]))
                {
                    // Se è solo un numero puro, ignoralo
                    int is_pure_number = 1;
                    for (int k = 1; token[k] != '\0'; k++)
                    {
                        if (!isdigit(token[k]) && token[k] != '.' && token[k] != 'f')
                        {
                            is_pure_number = 0;
                            break;
                        }
                    }
                    if (is_pure_number)
                    {
                        token = strtok(NULL, delimiters);
                        continue;
                    }
                    // altrimenti cade nel controllo is_valid_name sotto, che darà errore
                }

                if (is_valid_name(token, custom_types))
                {

                    // aggiunto dato che ci dava main come variabile dichiarata, ma in realtà è la funzione principale che segna l'inizio
                    //  del codice eseguibile, quindi usiamo questo flag per uscire dal blocco dichiarazioni e non contare main come variabile
                    if (strcmp(token, "main") == 0)
                    {
                        in_declaration = 0; // Ignoriamo main, non è una variabile da salvare
                        type_found[0] = '\0';
                        token = strtok(NULL, delimiters); // Passiamo al prossimo token
                        continue;
                    }

                    stats->tot_vars++; // Incrementa il contatore totale di variabili trovate

                    // Array dinamico per le variabili
                    if (*num_vars >= *capacity_array)
                    {
                        *capacity_array *= 2;
                        array_vars = realloc(array_vars, (*capacity_array) * sizeof(Variabile));
                        *array_vars_ptr = array_vars;
                    }

                    array_vars[*num_vars].name = strdup(token);
                    array_vars[*num_vars].type = strdup(type_found);
                    array_vars[*num_vars].line = current_line;
                    array_vars[*num_vars].used = 0;
                    (*num_vars)++;
                }
                else
                {
                    stats->tot_errors++;
                    stats->names_not_ok++;
                }
            }
        }
        token = strtok(NULL, delimiters);
    }
}

Variabile *read_file(char *filename, int *num_vars, Statistics *stats, ListType *custom_types)
{
    FILE *file = fopen(filename, "r"); // apriamo il file in lettura

    if (file == NULL)
    {
        fprintf(stderr, "Errore: impossibile aprire il file '%s'\n", filename);
        return NULL;
    }
    // array dinamico per salvare le variabili (token) trovati
    int capacity_array = 20; // verrà espanso dinamicamente se necessario, partiamo con una capacità iniziale di 20 variabili
    Variabile *array_vars = malloc(capacity_array * sizeof(Variabile));

    *num_vars = 0;        // contatore di variabili trovate, inizialmente 0
    int current_line = 0; // per salvare dove si trova la variabile o l'errore

    char line[256];
    int in_m_comment = 0; // flag per capire se siamo dentro un commento multilinea

    int reading_declarations = 1; // siamo nel blocco dichiarazioni (assunzione che tutte le dichiarazioni siano all'inizio del file, prima di qualsiasi codice eseguibile)

    while (fgets(line, sizeof(line), file) != NULL)
    {
        current_line++; // Incrementa il contatore di linee per l'errore

        rmv_comments(line, &in_m_comment); // puntatore a in_m_comment per modificare il flag dentro la funzione

        // passiamo &array_vars (puntatore a puntatore) e &capacity_array
        analyze_line(line, current_line, &array_vars, num_vars, &capacity_array, stats, &reading_declarations, custom_types); // analizza la riga e aggiorna l'array di variabili e le statistiche
    }

    fclose(file);
    return array_vars; // Ritorniamo il puntatore all'array di variabili trovate
}

void calculate_stats(Variabile *vars, int num_vars, Statistics *stats)
{
    for (int i = 0; i < num_vars; i++)
    {
        if (vars[i].used == 0)
        {
            stats->vars_not_used++;
            stats->tot_errors++;
        }
    }
}

void print_result(Statistics *stats, Variabile *vars, int num_vars, Parameters *param)
{

    // ==========================================
    // FASE 1: SCRITTURA SU FILE (se c'è il parametro -o)
    // ==========================================
    if (param->file_output != NULL)
    {
        FILE *file_out = fopen(param->file_output, "w");
        if (file_out)
        {
            fprintf(file_out, "=== RESOCONTO ANALISI ===\n");
            fprintf(file_out, "Variabili trovate: %d\n", stats->tot_vars);
            fprintf(file_out, "Errori totali: %d\n", stats->tot_errors);
            fprintf(file_out, " - Nomi non validi: %d\n", stats->names_not_ok);
            fprintf(file_out, " - Tipi non validi: %d\n", stats->types_not_ok);
            fprintf(file_out, " - Variabili non usate: %d\n", stats->vars_not_used);
            fprintf(file_out, "=========================\n\n");

            // Se nel file vogliamo anche i dettagli, stampiamoli
            fprintf(file_out, "--- DETTAGLIO VARIABILI ---\n");
            for (int i = 0; i < num_vars; i++)
            {
                fprintf(file_out, "Riga %d      | Tipo: %-10s      | Nome: %-15s      | Stato: %s\n",
                        vars[i].line, vars[i].type, vars[i].name,
                        vars[i].used ? "USATA" : "NON USATA [!]");
            }
            fclose(file_out);
            printf("Analisi completata! Risultati salvati in '%s'\n", param->file_output);
        }
        else
        {
            fprintf(stderr, "Errore: impossibile creare il file di output.\n");
        }
    }

    // ==========================================
    // FASE 2: STAMPA A SCHERMO (sempre, oppure solo se richiesto)
    // ==========================================
    // Stampiamo a video se c'è -v, OPPURE se non è stato chiesto nessun file
    if (param->verbose == 1 || param->file_output == NULL)
    {
        printf("\n=== RESOCONTO ANALISI ===\n");
        printf("Variabili trovate: %d\n", stats->tot_vars);
        printf("Errori totali: %d\n", stats->tot_errors);
        printf(" - Nomi non validi: %d\n", stats->names_not_ok);
        printf(" - Tipi non validi: %d\n", stats->types_not_ok);
        printf(" - Variabili non usate: %d\n", stats->vars_not_used);
        printf("=========================\n\n");

        if (param->verbose == 1)
        {
            printf("--- DETTAGLIO VARIABILI ---\n");
            for (int i = 0; i < num_vars; i++)
            {
                printf("Riga %d      | Tipo: %-10s      | Nome: %-15s      | Stato: %s\n",
                       vars[i].line, vars[i].type, vars[i].name,
                       vars[i].used ? "USATA" : "NON USATA [!]");
            }
        }
    }
}

// ============================================================================
// 7. PULIZIA MEMORIA
// ============================================================================
void free_memory(Variabile *vars, int num_vars, Parameters *param)
{
    for (int i = 0; i < num_vars; i++)
    {
        if (vars[i].name)
            free(vars[i].name);
        if (vars[i].type)
            free(vars[i].type);
    }
    free(vars);

    if (param)
    {
        if (param->file_input)
            free(param->file_input);
        if (param->file_output)
            free(param->file_output);
        free(param);
    }
}

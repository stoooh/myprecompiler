//in questa classe ci andranno le funzioni di analisi del file.

//cominciano con il parsing dei parametri da linea di comando

#include "header.h" 

Parametri* parsing_parameters(int argc, char *argv[]) {
    
    Parametri *par = malloc(sizeof(Parametri)); // memoria dinamica

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
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            par->verbose = 1;
        }
    }

    return par;
}

Variabili read_file(char *filename, int *num_vars, Statistiche *stats){
    FILE *file = fopen(filename, "r"); //apriamo il file in lettura

    //caso base
    if (file == NULL) {
        fprintf(stderr, "Errore: impossibile aprire il file '%s'\n", filename);
        return NULL;
    }

    //leggiamo il file riga per riga, e analizziamo le variabili (CHE SONO ALL'INIZIO PER SEMPLICITA')
    char line[256]; //buffer per leggere le righe fino a 256 caratteri

    //flag per capire se siamo dentro un commento multilinea
    int in_m_comment = 0;

    while (fgets(line, sizeof(line), file) != NULL) { //fgets legge una riga dal file e la mette in line
        //analizza la riga per trovare variabili, tipi, nomi e togliamo i commenti
        //se trovi una variabile, incrementa stats->variabili_totali
        //se trovi un errore, incrementa stats->errori_totali

        //tolgo i commenti (sia // che /* */) tramite un ciclo 
        for(int i = 0; line[i] != '\0'; i++){
            
            if(in_m_comment == 0){  //siamo fuori dal commento multilinea

                if(line[i] == '/' && line[i + 1] == '/'){  //commento singolo
                    line[i] = '\0';  //tronco tutto sulla riga
                }

                else if(line[i] == '/' && line[i + 1] == '*'){  //trovo l'inizio del commento multiriga
                in_m_comment = 1; //flag a 1
                line[i] = ' ';
                line[i + 1] = ' ';
                i++; //salto perche ho gia lavorato su *

            } else {

                if (line[i] == '*' && line[i+1] == '/') { //fine del commento multiriga
                in_commento = 0; // flag a 0
                line[i] = ' ';   
                line[i+1] = ' '; 
                i++;             //salto il carattere successivo
                } else {

                    line[i] = ' '; //sostituisco tutto con spazi finche sono dentro il commento

                }

            }
        }
        
        const char *delimiters = " \t\n;(){}[]*=+-/!&|<>,\"'"; 
        char *token = strtok(line, delimiters); //strtok divide la stringa in parole usando i delimitatori

        

    }
}

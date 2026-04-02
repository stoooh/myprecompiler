//il cuore del programma, gestione dei parametri, chiamata funzioni, ecc.
#include "header.h" //includiamo solo l'header che è il cuore

//includeremo funcs solo al momento della compliazione con gcc


int main(int argc, char *argv[]){  //args è il numero di argomenti, e argv l'array

    Parametri *par; 
    Variabile *var;
    Statistiche stats;
    int num_variabili;

//partiamo gestendo prima l'input dell'utente da terminale (-i, -o, -v)

    par = parsing_parameters(argc, argv); 

    //caso base
    if(par == NULL || par->file_input == NULL){ //la freccia è un puntatore
        fprintf(stderr, "Errore: parametro -i obbligatorio!\n"); //stampa l'errore su schermo
        fprintf(stderr, "Uso appropriato: %s -i <file_input> [-o <file_output>] [-v]\n", argv[0]); //stampa l'uso corretto del programma
        return 1; //codice errore
    }

    //inizializziamo le statistiche a zero nello stack
    stats.variabili_totali = 0;
    stats.errori_totali = 0;
    stats.variabili_non_utilizzate = 0;
    stats.nomi_non_corretti = 0;
    stats.tipi_non_corretti = 0;

    //lettura file C

    var = read_file(par->file_input, &num_variabili, &stats); //legge il file

    //caso base
    if(var == NULL){
        fprintf(stderr, "ERRORE, file impossibile leggere il file '%s'\n", par -> file_input);
        return 1;
    }

    calculate_stats(var, num_variabili, &stats);

    print_result(&stats, var, num_variabili, par);

    free_memory(var, num_variabili, par);

    //se tutto va bene ritorniamo 0
    return 0;
}
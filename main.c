//il cuore del programma, gestione dei parametri, chiamata funzioni, ecc.
#include "header.h" //includiamo solo l'header che è il cuore

//includeremo funcs solo al momento della compliazione con gcc


int main(int argc, char *argv[]){  //args è il numero di argomenti, e argv l'array

    Parameters *par; 
    Variabile *var;
    Statistics stats;
    int num_vars;

//partiamo gestendo prima l'input dell'utente da terminale (-i, -o, -v)

    par = parsing_parameters(argc, argv); 

    //caso base
    if(par == NULL || par->file_input == NULL){ //la freccia è un puntatore
        fprintf(stderr, "Errore: parametro -i obbligatorio!\n"); //stampa l'errore su schermo
        fprintf(stderr, "Uso appropriato: %s -i <file_input> [-o <file_output>] [-v]\n", argv[0]); //stampa l'uso corretto del programma
        return 1; //codice errore
    }

    //inizializziamo le statistiche a zero nello stack
    stats.tot_vars = 0;
    stats.tot_errors = 0;
    stats.vars_not_used = 0;
    stats.names_not_ok = 0;
    stats.types_not_ok = 0;

    ListType *custom_types = create_ListType();

    //lettura file C

    var = read_file(par->file_input, &num_vars, &stats, custom_types);

    //caso base
    if(var == NULL){
        fprintf(stderr, "ERRORE, file impossibile leggere il file '%s'\n", par -> file_input);
        freeListType(custom_types);
        return 1;
    }

    calculate_stats(var, num_vars, &stats);

    print_result(&stats, var, num_vars, par);

    free_memory(var, num_vars, par);
    
    freeListType(custom_types);

    //se tutto va bene ritorniamo 0
    return 0;
}
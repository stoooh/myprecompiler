#include <stdio.h>
#include <stdlib.h>

// 1. TYPEDEF COMPLESSI E PUNTATORI A FUNZIONE
typedef unsigned long long int ULLI;
typedef void (*PuntatoreFunzione)(int, float);

// 2. STRUTTURE ANONIME INLINE
struct { int campo_x; char campo_y; } struct_anonima;

int main() {
    // === BLOCCO DICHIARAZIONI ===

    // 3. LA TRAPPOLA DELLE STRINGHE (Il tuo parser legge dentro le stringhe?)
    char finto_commento[] = "Qui c'e un /* finto commento */ che inganna il parser";
    char finto_codice[] = "int variabile_fantasma = 100;";
    char escape_quote = '\'';
    
    // 4. MODIFICATORI MULTIPLI (strtok li separa!)
    unsigned long int super_numero = 42;
    const volatile float pi_greco = 3.14f;

    // 5. NOMI VALIDI DA INCUBO
    int _ = 1;
    int _____ = 2;
    int O0O0O0O = 3;
    
    // 6. SINTASSI CHE SFIDA I DELIMITATORI
    int a, b = 2, c, *p_int = &b, array_pazzo[10][20];
    
    // 7. ERRORI NASCOSTI NEL CAOS
    int 1_illegale = 0;              // Errore: inizia con numero
    char stringa~rotta = 'x';        // Errore: il carattere ~ non e un delimitatore!
    void tipo_void_errato = 0;       // Sintatticamente 'void' e un tipo, ma in C non puoi istanziare una variabile void.

    // === FINE BLOCCO DICHIARAZIONI ===
    // 'if' forzerà l'uscita dal blocco dichiarazioni
    if (b == 2) {
        printf("Sopravvissuti al parsing!\n");
    }

    // === BLOCCO CODICE ESEGUIBILE ===
    
    // 8. LA TRAPPOLA DEL CASTING (Sembra una dichiarazione!)
    a = (int) pi_greco;  

    // 9. OPERATORE TERNARIO E CARATTERI NON DELIMITATI
    c = (a > 0) ? _ : _____; 

    // 10. USO DELLE VARIABILI
    super_numero = O0O0O0O + array_pazzo[0][0];
    struct_anonima.campo_x = a;
    p_int = &c;
    finto_commento[0] = 'A';

    return 0;
}
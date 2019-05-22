#include "geracod.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>


int main (void) {
    FILE *f;
    funcp funcaoSB;
    
    int resultado=0;
    
    if ((f= fopen ("testTrab.txt", "r")) == NULL) {
        perror ("nao conseguiu abrir arquivo!");
        exit(1);
    }
    
    funcaoSB=geracod(f);
    fclose(f);

    resultado=(*funcaoSB)(5);

    printf("\nRes:%d\n", resultado);

    liberacod(funcaoSB);
    
  
    return 0;
}

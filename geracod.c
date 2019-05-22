/* Yuri_Marques_Strack 3WB*/

#include <stdio.h>
#include <stdlib.h>
#include "geracod.h"

typedef struct marcador Marcador;
struct marcador{
	int posAtual;
	int linhaMudar;
	int linhaDestino;
	int ifOrGo; /* valor 0 se for correcao de go, 1 se para jump se menor, 2 para jump se igual */
};

void jump(unsigned char* cd, Marcador* m,int *posAtual,Marcador* vmarcador,int* posInsere,int *temAlg,int *vetPosDeCadaLinha){
	int i,posDestino,dif,pos=*posAtual,posIns=*posInsere;
	Marcador aux;
	aux.posAtual=m->posAtual;
	aux.linhaMudar=m->linhaMudar;
	aux.linhaDestino=m->linhaDestino;
	cd[*posAtual]=0xe9;
	if (m->linhaDestino < m->posAtual){
	/* Corrige jump para tras */
		posDestino=vetPosDeCadaLinha[m->linhaDestino-1];
		dif=(long)&cd[posDestino] - (long)&cd[pos+5];
		pos++;
		for(i=1;i<5;i++){
			cd[pos]=(unsigned char)dif;pos++;
			dif= dif >> 8;
		}
	}
	else{
	/* Preenche espaco para jump para frente */
		vmarcador[posIns]=aux;posIns++;
		pos++;
		for(i=1;i<5;i++){
			cd[pos]=0x00;pos++;
		}
	}
	*posInsere=posIns;
	*temAlg=*temAlg+1;
	*posAtual=pos;
}
	
void corrigeJumpFrente(unsigned char* cd, Marcador *m,int *temAlg,int *vetPosDeCadaLinha){
	int i,dif,pos=vetPosDeCadaLinha[m->linhaMudar-1]+1; /* pos eh a posicao depois do 0xe9 */
	/* Corrige jump */ 
	if(m->ifOrGo==0){
		dif=(long)&cd[vetPosDeCadaLinha[m->linhaDestino-1]] - (long)&cd[vetPosDeCadaLinha[m->linhaMudar-1]+5];
		for(i=1;i<5;i++){
		cd[pos]=(unsigned char)dif;pos++;
		dif= dif >> 8;
		}
	}
	else {
		dif=(long)&cd[vetPosDeCadaLinha[m->linhaDestino-1]] - (long)&cd[m->posAtual+6];
		/* Corrige jump se menor */
		if(m->ifOrGo==1){
			pos=m->posAtual+2;
			for(i=1;i<5;i++){
				cd[pos]=(unsigned char)dif;pos++;
				dif= dif >> 8;
			}
		}
		else{
		/* Corrige jump se igual */
			pos=m->posAtual+2;
			for(i=1;i<5;i++){
				cd[pos]=(unsigned char)dif;pos++;
				dif= dif >> 8;
			}			
		}
	}
	*temAlg=*temAlg-1;
}

void cmp(unsigned char* cd,char v0,int idx,int *posAtual){
	int pos=*posAtual;
	if (v0=='p')
	{
		cd[pos]=0x83;pos++;
		if (idx ==1)
		{
			cd[pos]=0xff;pos++;
		}
		else if (idx==2)
		{
			cd[pos]=0xfe;pos++;
		}
		
	}
	else if (v0 == 'v')
	{
		cd[pos]=0x48;pos++;
    cd[pos]=0x8d;pos++;
    cd[pos]=0x45;pos++;
    cd[pos]=0xf0+(idx-1)*4;pos++;
    /* move o conteudo da variavel para r10d */
    cd[pos]=0x44;pos++;
    cd[pos]=0x8b;pos++;
    cd[pos]=0x10;pos++;
    
    cd[pos]=0x41;pos++;
    cd[pos]=0x83;pos++;
    cd[pos]=0xfa;pos++;
	}
	cd[pos]=0x00;pos++;
	*posAtual=pos;
}

static void error (const char *msg, int line) {
  fprintf(stderr, "erro %s na linha %d\n", msg, line);
  exit(EXIT_FAILURE);
}

funcp geracod(FILE *f) {
  unsigned char *codigo = (unsigned char*) malloc(128*sizeof(unsigned char));
	int vetPosDeCadaLinha[20]; /* vetor que contem o index da primeira instrucao de cada linha */
	Marcador vMarcador[10]; /* vetor de estruturas que contem a correcao necessaria p/ jump */
  int line = 1;
  int  c;
  int pos=8;
  int i=0;
  int posLivre=0; /* variavel para contrlar proxima posicao livre do vetor de estrutura */
  int flagTemAlguem=0; /* flag que verifica se é precis fazer correca em algum lugar do codigo */
	if (codigo==NULL){
		printf("erro\n");exit(1);
	}
	/* pushq e movq */
  codigo[0]=0x55;
  codigo[1]=0x48;
  codigo[2]=0x89;
  codigo[3]=0xe5;
  /* pilha de tamaho 16 (0x10) */
  codigo[4]=0x48;
  codigo[5]=0x83;
  codigo[6]=0xec;
  codigo[7]=0x10;

  while ((c = fgetc(f)) != EOF) {
		vetPosDeCadaLinha[line-1]=pos;
		if(flagTemAlguem > 0){
		/* faz a correcao do codigo */
			for (i=0;i<posLivre;i++){
				if (vMarcador[i].linhaDestino == line){
					corrigeJumpFrente(codigo,&vMarcador[i],&flagTemAlguem,vetPosDeCadaLinha);
				}
			}
		}
    switch (c) {
      case 'r': { /* retorno */
        char var0;
        int idx0;
        if (fscanf(f, "et %c%d", &var0, &idx0) != 2)
          error("comando invalido", line);
        if (var0 == 'p'){
            codigo[pos]=0x89;
            pos++;
            if (idx0 == 1){
                codigo[pos]=0xf8;pos++; /* move do edi pro eax */
            }
            else if (idx0 == 2){
                codigo[pos]=0xf0;pos++; /* move do esi pro eax */
            }
        }
        else if (var0=='$'){
            /* move a constante pro eax */
            codigo[pos]=0xb8;pos++;
          	for(i=0;i<4;i++){
              codigo[pos]=(unsigned char) idx0;pos++;
              idx0 = idx0 >> 8;
            }
        }
        else if (var0 == 'v'){
            codigo[pos]=0x48;pos++; /* leaq -16(%rbp), %rax) */
            codigo[pos]=0x8d;pos++;
            codigo[pos]=0x45;pos++;
            codigo[pos]=0xf0;pos++;

            codigo[pos]=0x48;pos++; /* add $x, %rax */
            codigo[pos]=0x83;pos++;
            codigo[pos]=0xc0;pos++;
            codigo[pos]=(idx0-1)*4;pos++; /* posicao da variavel */

            codigo[pos]=0x8b;pos++; /* movl (%rax), %eax */
            codigo[pos]=0x00;pos++;
        }
        codigo[pos]=0xc9;pos++; /* leave */
        codigo[pos]=0xc3;pos++; /* ret */
        break;
      }
    case 'v':
      case 'p': { /* atribuiÃ§Ã£o e op. aritmetica */
        char var0 = c, var1, op;
        int idx0, idx1;

        if (fscanf(f, "%d %c= %c%d", &idx0, &op, &var1, &idx1) != 4)
            error("comando invalido", line);
        if (var0 == 'p')
        {
            if (var1 == '$')
            {
                if(op == ':')
                {
                    if (idx0 == 1)
                    {
                        codigo[pos]=0xbf;pos++;
                    }
                    else if(idx0 == 2)
                    {
                        codigo[pos]=0xbe;pos++;
                    }
                }

                else if(op == '+')
                {
                    codigo[pos]=0x81;pos++;
                    if (idx0 == 1)
                    {
                        codigo[pos]=0xc7;pos++;
                    }
                    else if(idx0 == 2)
                    {
                        codigo[pos]=0xc6;pos++;
                    }
                }
                else if(op == '-')
                {
                    codigo[pos]=0x81;pos++;
                    if (idx0 == 1)
                    {
                        codigo[pos]=0xef;pos++;
                    }
                    else if(idx0 == 2)
                    {
                        codigo[pos]=0xee;pos++;
                    }
                }
                else if(op == '*')
                {
                    codigo[pos]=0x69;pos++;
                    if (idx0 == 1)
                    {
                        codigo[pos]=0xff;pos++;
                    }
                    else if(idx0 == 2)
                    {
                        codigo[pos]=0xf6;pos++;
                    }
                }
                /* para constantes que ocupam mais de 1 byte */
                for(i=0;i<4;i++){
                  codigo[pos]=(unsigned char) idx1;pos++;
                  idx1 = idx1 >> 8;
                }
            }
            else if(var1 == 'p')
            {
								if(op == ':')
                {
                    codigo[pos]=0x89;pos++;
                }
                else if(op == '+')
                {
                    codigo[pos]=0x01;pos++;
                }
                else if(op == '-')
                {
                    codigo[pos]=0x29;pos++;
                }
                else if(op == '*')
                {
                    codigo[pos]=0x0f;pos++;
                    codigo[pos]=0xaf;pos++;
                }
                if (idx0 == 2)
                {
                    if (idx1 == 2){/* p1 Op= p1 */
                      codigo[pos]=0xff;pos++;
										}
                    else if (idx1 == 1){/* p1 Op= p2 */
                      codigo[pos]=0xfe;pos++;
										}
                }
                else if(idx0 == 1)
                {
                    if (idx1 == 2){/* p2 Op= p1 */
                      codigo[pos]=0xf7;pos++;
										}
                    else if (idx1 == 1)/* p2 Op= p2 */
                      {codigo[pos]=0xf6;pos++;}
                }
            }
            else if(var1 == 'v')
            {
                if(op == ':')
                {
                    codigo[pos]=0x8b;pos++;
                }

                else if(op == '+')
                {
                    codigo[pos]=0x03;pos++;
                }
                else if(op == '-')
                {
                    codigo[pos]=0x2b;pos++;
                }
                else if(op == '*')
                {
                    codigo[pos]=0x0f;pos++;
                    codigo[pos]=0xaf;pos++;
                }
                if (idx0 == 2)/* p2 Op= vX */
                {
                    codigo[pos]=0x75;pos++;
                    codigo[pos]=0xf0+(idx1-1)*4;pos++;
                }
                else if(idx0 == 1)/* p1 Op= vX */
                {
                    codigo[pos]=0x7d;pos++;
                    codigo[pos]=0xf0+(idx1-1)*4;pos++;
                }
            }
        }
        else if(var0 == 'v')
        {
          /* pega a posicao da variavel vX na pilha e joga em rax */
          codigo[pos]=0x48;pos++;
          codigo[pos]=0x8d;pos++;
          codigo[pos]=0x45;pos++;
          codigo[pos]=0xf0+(idx0-1)*4;pos++;
					if(var1 == 'v')
					{
						 /* pega a posicao da variavel vX2 na pilha e joga em r10 */
            codigo[pos]=0x4c;pos++;
            codigo[pos]=0x8d;pos++;
            codigo[pos]=0x55;pos++;
            codigo[pos]=0xf0+(idx1-1)*4;pos++;
						/*move vX1 para r9d*/
						codigo[pos]=0x44;pos++; 
						codigo[pos]=0x8b;pos++;
						codigo[pos]=0x08;pos++;
						/*move vX2 para r11d*/
						codigo[pos]=0x45;pos++; 
						codigo[pos]=0x8b;pos++;
						codigo[pos]=0x1a;pos++;
						if (op== ':')
		        {
		          /* codigo atribuicao  */
							codigo[pos]=0x45;pos++; /*codigo constante */
		          codigo[pos]=0x89;pos++;
							codigo[pos]=0xd9;pos++; /*codigo constante */
		        }
						else
						{
				      if (op== '+')
				      {
				        /* codigo soma  */
				        codigo[pos]=0x45;pos++; /*codigo constante */
		          	codigo[pos]=0x01;pos++;
				      	codigo[pos]=0xd9;pos++; /*codigo constante */
		          }
				      else if (op== '-')
				      {
				        /* codigo sub  */
				        codigo[pos]=0x45;pos++; /*codigo constante */
		          	codigo[pos]=0x29;pos++;
				      	codigo[pos]=0xd9;pos++; /*codigo constante */
		        	}
				      else if (op== '*')
				      {
				        /* codigo mul  */
				        codigo[pos]=0x45;pos++; /*codigo constante */
		          	codigo[pos]=0x0f;pos++;
				        codigo[pos]=0xaf;pos++;
				      	codigo[pos]=0xcb;pos++; /*codigo constante */
		        	}
						}
						/* move para a posicao da pilha que ela ocupa */
			      codigo[pos]=0x44;pos++;
			      codigo[pos]=0x89;pos++;
						codigo[pos]=0x08;pos++;
					}
					else
					{
						if (var1 == '$')
		        {
		          /* move pro r10d o valor da constante */
		          codigo[pos]=0x41;pos++;
		          codigo[pos]=0xba;pos++;
		          /* for para constantes que ocupam mais de 1byte */
		          for(i=0;i<4;i++)
		          {
		            codigo[pos]=(unsigned char) idx1;pos++;
		            idx1 = idx1 >> 8;
		          }
		        }
		        else if (var1 == 'p')
		        {
							/* move o pX para r10d */
		          codigo[pos]=0x41;pos++;
		          codigo[pos]=0x89;pos++;
		          if (idx1 == 1)
		          {
		            codigo[pos]=0xfa;pos++;
		          }
		          if (idx1 == 2)
		          {
		            codigo[pos]=0xf2;pos++;
		          }
		        }
		        if (op == ':'){
								codigo[pos]=0x44;pos++;
								codigo[pos]=0x89;pos++;
				        codigo[pos]=0x10;pos++;					
						}
						else if (op == '+'){
								codigo[pos]=0x44;pos++;
								codigo[pos]=0x01;pos++;
				        codigo[pos]=0x10;pos++;		
						}
						else if (op == '-'){
								codigo[pos]=0x44;pos++;
								codigo[pos]=0x29;pos++;
				        codigo[pos]=0x10;pos++;	
						}
						else if (op == '*'){
								codigo[pos]=0x44;pos++;
								codigo[pos]=0x0f;pos++;
								codigo[pos]=0xaf;pos++;
				        codigo[pos]=0x10;pos++;	
				        
								codigo[pos]=0x44;pos++;
								codigo[pos]=0x89;pos++;
				        codigo[pos]=0x10;pos++;	
						}
					}          
        }
        break;
      }
      case 'i': { /* desvio condicional */
        char var0;
        int idx0, n1, n2;
        Marcador m1,m2;
        if (fscanf(f, "f %c%d %d %d", &var0, &idx0, &n1, &n2) != 4)
            error("comando invalido", line);
				cmp(codigo,var0,idx0,&pos);
        
        m1.posAtual=pos;
				m1.linhaMudar=line;

				m1.linhaDestino=n1;
				m1.ifOrGo=1;
				/* jump se menor */
				codigo[pos]=0x0f;pos++;
				codigo[pos]=0x8c;pos++;
				codigo[pos]=0x00;pos++;
				codigo[pos]=0x00;pos++;
				codigo[pos]=0x00;pos++;
				codigo[pos]=0x00;pos++;
				vMarcador[posLivre]=m1;posLivre++;flagTemAlguem++;
				/* jump se igual */
				m2.posAtual=pos;
				m2.linhaMudar=line;
				m2.linhaDestino=n2;
				m2.ifOrGo=2;
				codigo[pos]=0x0f;pos++;
				codigo[pos]=0x84;pos++;
				codigo[pos]=0x00;pos++;
				codigo[pos]=0x00;pos++;
				codigo[pos]=0x00;pos++;
				codigo[pos]=0x00;pos++;
				vMarcador[posLivre]=m2;posLivre++;flagTemAlguem++;						
				/* se maior continua a execucao */
        break;
      }
      case 'g': { /* desvio incondicional */
        int n1;
				Marcador m;
        if (fscanf(f, "o %d", &n1) != 1)
            error("comando invalido", line);
        m.posAtual=pos;
				m.linhaDestino=n1;
				m.linhaMudar=line;
				m.ifOrGo=0;
				jump(codigo,&m,&pos,vMarcador,&posLivre,&flagTemAlguem,vetPosDeCadaLinha);
        break;
      }
      default: error("comando desconhecido", line);
    } 
    line ++;
    fscanf(f, " ");
  }
  return (funcp)codigo;
}

void liberacod(void *pf){
	free(pf);
}

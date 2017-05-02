#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define ARRAY_SIZE 100000

int main(int argc, char** argv){

    int my_rank;       
    int proc_n;        
    MPI_Status status; 
    MPI_Init();
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);
    int delta = 50;

    /* DUVIDAS
        A arvore vai ser sempre cheia?
        o delta eh fixo?

    */
    
    my_rank = MPI_Comm_rank();  // pega pega o numero do processo atual (rank)

    // recebo vetor

    if ( my_rank == 0 ) // NODO PAI
    {
        //tam_vetor = VETOR_SIZE;               // defino tamanho inicial do vetor
        //Inicializa ( vetor, tam_vetor );      // sou a raiz e portanto gero o vetor - ordem reversa

        int *vetor;
        vetor = (int *)malloc(sizeof(int) * ARRAY_SIZE);

        /*preenche vetor */
        int i;
        for (i=0 ; i<ARRAY_SIZE; i++)              
            vetor[i] = ARRAY_SIZE-i;

    }
    else // NODO FILHO
    {
        /* Descubro quem vai me enviar (pai) calculando pelo meu rank */
        int mysource = (my_rank-1)/2;

        /* Instancia vetor */
        int *vetor;
        vetor = (int *)malloc(sizeof(int) * ARRAY_SIZE);

        /* Espero uma parte do vetor para ordenar */
        MPI_Recv(vetor, ARRAY_SIZE , MPI_INT, mysource, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int size = MPI_Get_count(&Status, MPI_INT, &tam_vetor);  // descubro tamanho da mensagem recebida
        
        /* Pego o rank e a tag de quem me enviou */
        int source = status.MPI_SOURCE;
        int tag - status.MPI_TAG;

        /* Se eu recebi uma msg de suicidio */
        if(tag == 0){
			printf("Suicidando-se, rank %d\n",my_rank);
			MPI_Finalize();
		}

        if(size <= delta){ //Conquista
            /* Ordena o vetor */
            bs(size, vetor);
        }else{

            int newsize = size/2;

            int filhoDireita = (my_rank*2)+1;
            int filhoEsquerda = (my_rank*2)+2
            
            /* Divide o vetor para seus dois filhos, se existirem */
            if(proc_n > filhoDireita){
                MPI_Send(&vetor[0], newsize, MPI_INT, (my_rank*2)+1, 2, MPI_COMM_WORLD);
            }
            if(proc_n > filhoEsquerda){
                MPI_Send(&vetor[size/2], newsize, MPI_INT, (my_rank*2)+2, 2, MPI_COMM_WORLD);
            }

            /* Aguarda a resposta dos filhos */
            if(proc_n > (my_rank*2)+1) MPI_Recv(&vetor[0], newsize, MPI_INT, (my_rank*2)+1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if(proc_n > (my_rank*2)+2) MPI_Recv(&vetor[size/2], newsize, MPI_INT, (my_rank*2)+2, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            /* junta vetores */
            int y;
            for (y=0 ; y<size/2; y++)              
                vetor[y] = vetor_aux1[y];
            int j;
            for (j=(size/2) ; j<size; j++)              
                vetor[j] = vetor_aux2[j-(size/2)];
				
			/* Merge do vetor */
            vetor = interleaving(vetor,size);


        }




    }

    // dividir ou conquistar?

    if ( tam_vetor <= delta ){
        BubbleSort (vetor);  // conquisto
    }else{
        // dividir
        // quebrar em duas partes e mandar para os filhos

        MPI_Send ( &vetor[0], filho esquerda, tam_vetor/2 );  // mando metade inicial do vetor
        MPI_Send ( &vetor[tam_vetor/2], filho direita , tam_vetor/2 );  // mando metade final

        // receber dos filhos

        MPI_Recv ( &vetor[0], filho esquerda);            
        MPI_Recv ( &vetor[tam_vetor/2], filho direita);   

        // intercalo vetor inteiro
    
        Intercala ( vetor );
    }

    // mando para o pai

    if ( my_rank !=0 )
        MPI_Send ( vetor, pai, tam_vetor );  // tenho pai, retorno vetor ordenado pra ele
    else
        Mostra ( vetor );                    // sou o raiz, mostro vetor

    MPI_Finalize();
}

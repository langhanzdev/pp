#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

//#define ARRAY_SIZE 100000

int *interleaving(int vetor[], int tam)
{
    int *vetor_auxiliar;
    int i1, i2, i_aux;

    vetor_auxiliar = (int *)malloc(sizeof(int) * tam);

    i1 = 0;
    i2 = tam / 2;

    for (i_aux = 0; i_aux < tam; i_aux++) {
        if (((vetor[i1] <= vetor[i2]) && (i1 < (tam / 2))) || (i2 == tam))
            vetor_auxiliar[i_aux] = vetor[i1++];
        else
            vetor_auxiliar[i_aux] = vetor[i2++];
    }
    return vetor_auxiliar;
}

/* Bubble Sort */
void bs(int n, int * vetor)
{

    int c=0, d, troca, trocou =1;
    // printf("Ordenando...\n");
    while (c < (n-1) & trocou ){
        trocou = 0;
        for (d = 0 ; d < n - c - 1; d++)
            if (vetor[d] > vetor[d+1]){
                troca      = vetor[d];
                vetor[d]   = vetor[d+1];
                vetor[d+1] = troca;
                trocou = 1;
            }
        c++;
    }
}

int main(int argc, char** argv){

    int my_rank;       
    int proc_n;     
    int ARRAY_SIZE = atoi(argv[1]);   
    MPI_Status status; 
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);
    int delta = ARRAY_SIZE / (((proc_n-1)/2) + 1); // Numero de folhas da minha arvore

    int filhoDireita = (my_rank*2)+1;
    int filhoEsquerda = (my_rank*2)+2;

    int *vetor;
        vetor = (int *)malloc(sizeof(int) * ARRAY_SIZE);

    int size;
    int newsize;
    int localsize;
    int pos1, pos2;
    int par;

    double t1,t2;
    t1 = MPI_Wtime();  // inicia a contagem do tempo

    if ( my_rank == 0 ) // NODO PAI
    {

        /*preenche vetor */
        int i;
        for (i=0 ; i<ARRAY_SIZE; i++){
            vetor[i] = ARRAY_SIZE-i;
        }
        printf("Primeiro [%d]\n",vetor[0]);
        size = ARRAY_SIZE;
        localsize = ARRAY_SIZE/10;
        newsize = (size-localsize)/2;
        pos1 = 0+localsize;
        pos2 = (size/2)+(localsize/2);
        par = (int)(newsize%2);
        printf("tamanho local %d\n",localsize);
        printf("segunda posicao %d\n",pos2);
        printf("new size %d\n",newsize);
        printf("par %d\n",par);

        /* Divide o vetor para seus dois filhos, se existirem */
        if(proc_n > filhoDireita){
            MPI_Send(&vetor[pos1], newsize, MPI_INT, filhoDireita, 1, MPI_COMM_WORLD);
        }
        if(proc_n > filhoEsquerda){
            MPI_Send(&vetor[pos2], newsize+par, MPI_INT, filhoEsquerda, 1, MPI_COMM_WORLD);
        }

        bs(localsize,vetor);

        /* Aguarda a resposta dos filhos */
        if(proc_n > filhoDireita) MPI_Recv(&vetor[pos1], newsize, MPI_INT, filhoDireita, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        if(proc_n > filhoEsquerda) MPI_Recv(&vetor[pos2], newsize+par, MPI_INT, filhoEsquerda, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        /* Merge do vetor */
        vetor = interleaving(vetor,size);

    }
    else // NODO FILHO
    {
        /* Descubro quem vai me enviar (pai) calculando pelo meu rank */
        int mysource = (my_rank-1)/2;

        /* Espero uma parte do vetor para ordenar */
        MPI_Recv(vetor, ARRAY_SIZE , MPI_INT, mysource, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &size);  // descubro tamanho da mensagem recebida
        
        /* Pego o rank e a tag de quem me enviou */
        int source = status.MPI_SOURCE;
        int tag = status.MPI_TAG;

        /* Se eu recebi uma msg de suicidio */
        if(tag == 0){
			printf("Suicidando-se, rank %d\n",my_rank);
			MPI_Finalize();
		}

        if(size <= delta){ //Conquista
            /* Ordena o vetor */
            bs(size, vetor);
        }else{ //Divide

            localsize = ARRAY_SIZE/10;
            newsize = (size-localsize)/2;
            pos1 = 0+localsize;
            pos2 = (size/2)+(localsize/2);
            par = (int)(newsize%2);
            
            /* Divide o vetor para seus dois filhos, se existirem */
            if(proc_n > filhoDireita){
                MPI_Send(&vetor[pos1], newsize, MPI_INT, filhoDireita, 1, MPI_COMM_WORLD);
            }
            if(proc_n > filhoEsquerda){
                MPI_Send(&vetor[pos2], newsize+par, MPI_INT, filhoEsquerda, 1, MPI_COMM_WORLD);
            }

            bs(localsize,vetor);
            
            /* Aguarda a resposta dos filhos */
            if(proc_n > filhoDireita) MPI_Recv(&vetor[pos1], newsize, MPI_INT, filhoDireita, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            vetor = interleaving(vetor,size-(newsize+par));
            if(proc_n > filhoEsquerda) MPI_Recv(&vetor[pos2], newsize+par, MPI_INT, filhoEsquerda, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            /* Merge do vetor */
            vetor = interleaving(vetor,size);

        }
        /* Envia de volta para o pai */
        MPI_Send(vetor, size, MPI_INT, source, 1, MPI_COMM_WORLD);
    }

    if(my_rank==0){
        
        printf("Primeiro ordenado [%d]\n",vetor[0]);
        int abc;
        for(abc=0;abc<ARRAY_SIZE;abc++){
            printf("%d -> [%d]\n",abc,vetor[abc]);
        }
        t2 = MPI_Wtime();
        printf("\nTempo de execucao: %f\n\n", t2-t1);
    }

    MPI_Finalize();
}

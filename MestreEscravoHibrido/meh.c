#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>
#include <math.h>

#define COL 100000
#define LIN 1000
#define NUM_THREADS 4

//int saco_de_trabalho[LIN][COL];

int cmpfunc (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}

int main(int argc, char** argv){
	int my_rank;       // Identificador deste processo
  	int proc_n;        // Numero de processos disparados pelo usuario na linha de comando (np)  
  	int message[10];   // Buffer para as mensagens                    
  	MPI_Status status; // estrutura que guarda o estado de retorno
  	MPI_Status status2; // estrutura que guarda o estado de retorno          
  	MPI_Init(&argc , &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo
  	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
  	MPI_Comm_size(MPI_COMM_WORLD, &proc_n);  // pega informacao do numero de processos (quantidade total)
	//int saco_de_trabalho[LIN][COL];
	int resp[10];
	int f[proc_n];
	int *v =  (int*)malloc(((LIN/NUM_THREADS)*COL)*sizeof(int));

	omp_set_num_threads(4);

	double t1,t2;
	t1 = MPI_Wtime();  // inicia a contagem do tempo

	if( my_rank == 0 ){ //MESTRE
		printf("[M] sou o Mestre\n\n");

		int **saco_de_trabalho = (int**)malloc(LIN*sizeof(int*));
		int i,j;
		
		printf("Preenchendo...\n");
		
		for (i=0 ; i<LIN; i++){              
			saco_de_trabalho[i] = (int*)malloc(COL*sizeof(int));
			for (j=0 ; j<COL; j++){
				saco_de_trabalho[i][j] = COL-j;
			}
		}

		

		printf("Ordenando...\n");

		//Popula saco de trabalho
		// int i;
		// int j;
		// for(i=0;i<LIN;i++){
		// 	for(j=0;j<COL;j++){
		// 		saco_de_trabalho[i][j] = (i+1) * (COL-j);
		// 	}
		// }

		//printa matriz		
		//  printf("[M]\n");
		//  for(i=0;i<LIN/100;i++){
		//  	for(j=0;j<COL/1000;j++){
		//  		printf("[%d] ",saco_de_trabalho[i][j]);
		//  	}
		//  	printf("\n");
		//  }
		
		int order=1;
		int count = 0;
		int pid_slave;

		//Envia todos
		int pos = 0;
		int pos2 = 0;
		int m = LIN/NUM_THREADS;
		while(pos < LIN){
			for(order=1;order < proc_n && pos < LIN;order++){
				
				// if(order >= proc_n) order = 1;
				MPI_Send(saco_de_trabalho[pos], COL*NUM_THREADS, MPI_INT, order, 1, MPI_COMM_WORLD);
				pos += NUM_THREADS;

			}
			//Recebe todos
			
			for(order=1;order < proc_n && pos2 < LIN;order++){
				
				// if(order >= proc_n) order = 1;
				MPI_Recv(saco_de_trabalho[pos2], COL*NUM_THREADS, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				pos2 += NUM_THREADS;
			}
		}


		printf("Ordenacao pronta.\n");
		
		//Mata todos
		int y;
		for(y=1;y<proc_n;y++){
			MPI_Send(0, 0, MPI_INT, y, 0, MPI_COMM_WORLD);
		}
		//printa matriz		
		i=0;
		j=0;
		printf("[M] RESULTADO -----------------------\n");
		for(i=0;i<2;i++){
			for(j=0;j<COL;j++){
				printf("[%d] ",saco_de_trabalho[i][j]);
			}
			printf("\n");
		}


		t2 = MPI_Wtime(); // termina a contagem do tempo
 		printf("\n[M] Tempo de execucao: %f\n\n", t2-t1);

		MPI_Finalize();
		

	}else{ //ESCRAVO
		int tag=0;
		int m = LIN/NUM_THREADS;
		printf("\nM: %d\n",m);
		while(1){
			
			MPI_Recv(v, COL*NUM_THREADS, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			tag = status.MPI_TAG;
			if(tag >= 1){
				
				
				int i;
				int acc = 0;
				omp_set_num_threads(NUM_THREADS);
				#pragma omp parallel for
				for(i=0;i<NUM_THREADS;i++){
					
					qsort(&v[i*COL],COL,sizeof(int),cmpfunc);
					
				}
				// printf("SAI ESSCRAVO\n");
				MPI_Send(v, COL*NUM_THREADS, MPI_INT, 0, 1, MPI_COMM_WORLD);
				// printf("SAI ESSCRAVO MESMO\n");
			}else{
				break;
			}
		}
		
		MPI_Finalize();
	}	
return 0;
}


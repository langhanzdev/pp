#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>

#define COL 100000
#define LIN 1000

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
	int *v =  (int*)malloc(COL*sizeof(int));

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
		/* printf("[M]\n");
		 for(i=0;i<LIN;i++){
		 	for(j=0;j<COL;j++){
		 		printf("[%d] ",saco_de_trabalho[i][j]);
		 	}
		 	printf("\n");
		 }*/
		
		long order=1;
		int count = 0;
		int pid_slave;

		//Envia todos
		for(order=1;order<proc_n;order++){
			MPI_Send(saco_de_trabalho[order-1], COL, MPI_INT, order, order, MPI_COMM_WORLD);
		}
		
		//Recebe e reenvia mais
		//for(y=1;y<=LIN;y++){
		int y;
		int tag = 0;
		int source;
		while(count<LIN && order <= LIN+proc_n-1){
			MPI_Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD, &status2);
			tag = (int)(status2.MPI_TAG);
			
			// printf("[M] TAG RECV %d\n",tag);
			if(tag > 0){
				// printf("\n [M] TAG %d c %d order %d \n\n",tag,count,order);
				MPI_Recv(saco_de_trabalho[tag-1], COL, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
				source = status.MPI_SOURCE;
				if(count<LIN && order <= LIN) {
					MPI_Send(saco_de_trabalho[order-1], COL, MPI_INT, source, order, MPI_COMM_WORLD);
					// printf("[M] SEND order %d\n",order);
				}
				order++;
				count++;
                if(count % 50 == 0) printf(" ordenado %d\n",count);
			}
		}

		printf("Ordenacao pronta.\n");
		
		/*
		MPI_Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD, &status);
			
		MPI_Recv(saco_de_trabalho[status.MPI_TAG-1], COL, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
		count++;
		if(count<COL) MPI_Send(&saco_de_trabalho[count], COL, MPI_INT, status.MPI_SOURCE, y, MPI_COMM_WORLD);
		y++;*/

		

		//Mata todos
		for(y=1;y<proc_n;y++){
			MPI_Send(saco_de_trabalho[0], 1, MPI_INT, y, 0, MPI_COMM_WORLD);
		}

		//printa matriz		
		i=0;
		j=0;
		/*printf("[M] RESULTADO -----------------------\n");
		for(i=0;i<LIN/100;i++){
			for(j=0;j<COL/10000;j++){
				printf("[%d] ",saco_de_trabalho[i][j]);
			}
			printf("\n");
		}*/


		t2 = MPI_Wtime(); // termina a contagem do tempo
 		printf("\n[M] Tempo de execucao: %f\n\n", t2-t1);
		MPI_Finalize();
		

	}else{ //ESCRAVO
		//printf("[E] sou o Escravo, Pid: %d  \n",my_rank);
		int tag=0;
		while(1){
			MPI_Recv(v, COL, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			tag = status.MPI_TAG;
			// printf("[E] tag: %d\n",tag);
			if(tag >= 1){
				
				
				qsort(v,COL,sizeof(int),cmpfunc);

				
				// int s;
				// for(s=0;s<COL;s++){
				// 	printf("[E] [%d] ",&v[s]);
				// }printf("\n");

				// printf("[E] SEND tag %d\n",tag);
				MPI_Send(v, COL, MPI_INT, 0, tag, MPI_COMM_WORLD);
				
				//printf("[E] Escravo terminou %d\n",my_rank);
			}else{
				break;
			}
		}
		//printf("[E] Escravo se matando %d.\n",my_rank);
		MPI_Finalize();
	}	
return 0;
}

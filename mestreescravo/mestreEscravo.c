#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <math.h>

#define COL 4
#define LIN 4

void quickSort( int[], int, int);
int partition( int[], int, int);

main(int argc, char** argv){
	int my_rank;       // Identificador deste processo
  	int proc_n;        // Numero de processos disparados pelo usuario na linha de comando (np)  
  	int message[10];   // Buffer para as mensagens                    
  	MPI_Status status; // estrutura que guarda o estado de retorno          
  	MPI_Init(&argc , &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo
  	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
  	MPI_Comm_size(MPI_COMM_WORLD, &proc_n);  // pega informacao do numero de processos (quantidade total)
	int saco_de_trabalho[LIN][COL];
	int resp[10];
	int f[proc_n];
	int v[10];

	

	double t1,t2;
	t1 = MPI_Wtime();  // inicia a contagem do tempo

	if( my_rank == 0 ){ //MESTRE
		printf("[M] sou o Mestre\n\n");

		int x;
		for(x=0;x<proc_n;x++){
			f[x] = 0;
		}
		
		//Popula saco de trabalho
		int i;
		int j;
		for(i=0;i<LIN;i++){
			for(j=0;j<COL;j++){
				saco_de_trabalho[i][j] = rand() % 10;
			}
		}

		//printa matriz		
		printf("[M]\n");
		for(i=0;i<LIN;i++){
			for(j=0;j<COL;j++){
				printf("[%d] ",saco_de_trabalho[i][j]);
			}
			printf("\n");
		}
		
		int y;
		int count = 0;
		int pid_slave;

		//Envia todos
		for(y=1;y<proc_n;y++){
			MPI_Send(&saco_de_trabalho[y-1], COL, MPI_INT, y, 1, MPI_COMM_WORLD);
		}
		
		//Recebe e reenvia mais
		//for(y=1;y<=LIN;y++){
		while(count<COL-1){
			MPI_Recv(&resp, COL, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			count++;
			saco_de_trabalho[count][0] = &resp;
			if(count<COL) MPI_Send(&saco_de_trabalho[count], COL, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
		}
		

		//printa matriz		
		printf("[M] RESULTADO -----------------------\n");
		for(i=0;i<LIN;i++){
			for(j=0;j<COL;j++){
				printf("[%d] ",saco_de_trabalho[i][j]);
			}
			printf("\n");
		}

		//Mata todos
		for(y=1;y<proc_n;y++){
			MPI_Send(&saco_de_trabalho[0], COL, MPI_INT, y, 0, MPI_COMM_WORLD);
		}


		t2 = MPI_Wtime(); // termina a contagem do tempo
 		printf("\n[M] Tempo de execucao: %f\n\n", t2-t1);
		MPI_Finalize();
		

	}else{ //ESCRAVO
		printf("[E] sou o Escravo, Pid: %d  \n",my_rank);

		while(1){
			MPI_Recv(&v, COL, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			if(status.MPI_TAG == 1){
				
				int s;
				for(s=0;s<COL;s++){
					printf("[E] [%d] ",v[s]);
				}printf("\n");

				//quickSort( v, 0, COL-1);

				MPI_Send(&v, COL, MPI_INT, 0, 1, MPI_COMM_WORLD);
				
				printf("[E] Escravo terminou %d\n",my_rank);
			}else{
				break;
			}
		}
		printf("[E] Escravo se matando %d.\n",my_rank);
		MPI_Finalize();
	}

	

	
}

void quickSort( int a[], int l, int r)
{
   int j;

   if( l < r ) 
   {
   	// divide and conquer
        j = partition( a, l, r);
       quickSort( a, l, j-1);
       quickSort( a, j+1, r);
   }
	
}



int partition( int a[], int l, int r) {
   int pivot, i, j, t;
   pivot = a[l];
   i = l; j = r+1;
		
   while( 1)
   {
   	do ++i; while( a[i] <= pivot && i <= r );
   	do --j; while( a[j] > pivot );
   	if( i >= j ) break;
   	t = a[i]; a[i] = a[j]; a[j] = t;
   }
   t = a[l]; a[l] = a[j]; a[j] = t;
   return j;
}
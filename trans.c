#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

enum {mat_size = 8, SIZE = mat_size* mat_size, NBLOCKS = SIZE/4  } ;

int *get_slice( int arr[SIZE], int slice[4], int arrSize, int sliceSize, int i, int j )
{
    int k = 0;
    int l = 0;
    int spliceColumnSize = sqrt(sliceSize);
    int arrayColumnSize = sqrt(arrSize);
    for( k = 0 ; k < spliceColumnSize ; ++k ) {
        for (l = 0; l < spliceColumnSize; ++l) {
            slice[l + (k * (spliceColumnSize))] = arr[(i+k)*arrayColumnSize+j+l];
        }
    }
    
    return slice;
}
void printSlice(int arr[], int sliceSize)
{
    int size = sqrt(sliceSize);
    for(int i=0;i<size;++i){
        for(int j=0;j<size;++j)
        {
            printf("arr[%d][%d]:%2d, ",i,j,arr[i*(size)+j]);
        }
        printf("\n");
    }
}

void swap(int* num1, int* num2)
{
    long int temp;
    temp = *num1;
    *num1 = *num2;
    *num2 = temp;
}


void transpose(int *arr,int sliceSize){

    int size = sqrt(sliceSize);
    int i,j;
    for (int i = 0; i < size; i++) {
            for (int j = i+1; j < size; j++) {
                   swap((arr + i*size + j),(arr + j*size + i));
            }
    }
}


int** create_matrix(int row, int col) {
    int i = 0;
    int **m = (int**) malloc(row * sizeof(int*));
    int *n = (int*) malloc(row * col * sizeof(int));
    for (i=0;i<row;i++) m[i] = &(n[i * col]);
    return m;
}

void init_matrix_rand(int **m, int row, int col) {
int i, j = 0,num =0;
    for (i=0;i<row;i++) {
        for (j=0;j<col;j++) {
            m[i][j] = ++num;
        }
    }
}

int** create_rand_matrix(int size) {
    int **m = create_matrix(size, size);
    init_matrix_rand(m, size, size);
    return m;
}

void print_matrix(int **m, int row, int col) {
    int i, j = 0;
    for (i=0;i<row;i++) {
        for (j=0;j<col;j++) {
            printf("%d", m[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char** argv) {
    
  int i, j, nprocs, rank;

  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

/*
  if (nprocs > mat_size)
    {
      if (rank == 0)
	printf ("Processors exceed matrix size\n");
      MPI_Finalize ();
      return 1;
    }
  */
         
    double process_time, process_time_to_avg, comm_time = 0;
    int **rand_matrix, **trans_matrix = NULL;
    int *recv_buffer = (int*) malloc(mat_size * sizeof(int));
    int *recv_buffer2 = (int*) malloc(mat_size * sizeof(int));
    
    MPI_Status stat;
    MPI_Datatype rowtype;
    MPI_Datatype Columntype;

    MPI_Type_contiguous(mat_size, MPI_INT, &rowtype);
    MPI_Type_commit(&rowtype);
    
    MPI_Type_contiguous(mat_size, MPI_INT, &Columntype);
    MPI_Type_commit(&Columntype);

    // scatter rows
    if (rank == 0) {
        rand_matrix = create_rand_matrix(mat_size); 
       // print_matrix(rand_matrix, mat_size, mat_size);
        
    int matrix[SIZE] ;
    int block[SIZE];   
    int* Mat[NBLOCKS]; // bigger matrix
           
    int j = 0;
    int i = 0;
    for (j = 0; j < SIZE; ++j) {
        matrix[j] = 1 + j;
    }

    int matrixColumnSize = sqrt(SIZE);
    for (i = 0; i < matrixColumnSize; ++i) {
        for (j = 0; j < matrixColumnSize; ++j) {
            //printf("matrix[%d][%d] -> %2d ", i, NBLOCKS, matrix[j + i * matrixColumnSize]);
        }
        //printf("\n");
    }
    //printf("\n");
    //printf("*************\n");

    int block_num =0;
 
            for( int i = 0 ; i < sqrt(NBLOCKS) ; ++i ){
                for( int j = 0 ; j < sqrt(NBLOCKS) ; ++j ){
                    int sliceColumnSize = sqrt(SIZE / NBLOCKS);
                    int *A = get_slice( matrix, block, SIZE, SIZE / NBLOCKS ,i*(sliceColumnSize), j*(sliceColumnSize) );
                    //printf("block:%d\n",block_num++);
                    //printSlice(block,SIZE / NBLOCKS);
                    //printf("*******After block trans******\n");
                    transpose(A,SIZE / NBLOCKS); // do local transpose then send
                    //printSlice(A,SIZE / NBLOCKS);
                    Mat[block_num] = A; // block to bigger matrix
                    
                }
           }
           
        printf("*******Final transpose******\n");
         //Transpose/////////////

        process_time -= MPI_Wtime();
        comm_time -= MPI_Wtime();
         
        // send different rows
        
        MPI_Scatter(&(rand_matrix[0][0]), mat_size, MPI_INT, &(recv_buffer[0]), mat_size, MPI_INT, 0, MPI_COMM_WORLD); //rank 0

        comm_time += MPI_Wtime();
        
        comm_time -= MPI_Wtime();
    } else {
        MPI_Scatter(NULL, 0, MPI_INT, &(recv_buffer[0]), mat_size, MPI_INT, 0, MPI_COMM_WORLD);
    }

 // do transpose of scrattered rows
    MPI_Alltoall(recv_buffer, 1, MPI_INT, recv_buffer2, 1, MPI_INT, MPI_COMM_WORLD);
  
    // gather to form matrix
    if (rank == 0) {
        comm_time += MPI_Wtime();
        trans_matrix = create_matrix(mat_size, mat_size);
        comm_time -= MPI_Wtime();
        MPI_Gather(recv_buffer2, mat_size, MPI_INT, &(trans_matrix[0][0]), mat_size, MPI_INT, 0, MPI_COMM_WORLD);
    } else {
        MPI_Gather(recv_buffer2, mat_size, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        comm_time += MPI_Wtime();
        process_time += MPI_Wtime();
        //print_matrix(trans_matrix, mat_size, mat_size);
        printf("Final Tranpose took: %d\t%lf\t%lf\n", mat_size, process_time, comm_time);

        free(rand_matrix[0]);
        free(trans_matrix[0]);
        free(rand_matrix);
        free(trans_matrix);
    }
    free(recv_buffer);
    free(recv_buffer2);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}

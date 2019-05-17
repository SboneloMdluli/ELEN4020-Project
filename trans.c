#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

void trans (double *a, int n) {
  int i, j;
  int ij, ji, l;
  double tmp;
  ij = 0;
  l = -1;
  for (i = 0; i < n; i++)
    {
      l += n + 1;
      ji = l;
      ij += i + 1;
      for (j = i+1; j < n; j++)
        {
          tmp = a[ij];
          a[ij] = a[ji];
          a[ji] = tmp;
          ij++;
          ji += n;
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

    // due to the nature of MPI_Alltoall
    // the number of rows must be the same as the number of processors
    // to even more simplify the experiment we will use square matrix m[mat_size x mat_size]
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

    int mat_size= 4;
    int i, world_rank, world_size;
    double process_time, process_time_to_avg, comm_time = 0;
    int **rand_matrix, **trans_matrix = NULL;
    int *recv_buffer = (int*) malloc(mat_size * sizeof(int));
    int *recv_buffer2 = (int*) malloc(mat_size * sizeof(int));
    
    MPI_Status stat;
    MPI_Datatype rowtype;
    MPI_Datatype Columntype;

    srand(time(NULL));
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    MPI_Type_contiguous(mat_size, MPI_INT, &rowtype);
    MPI_Type_commit(&rowtype);
    
    MPI_Type_contiguous(mat_size, MPI_INT, &Columntype);
    MPI_Type_commit(&Columntype);

    // scatter rows
    if (world_rank == 0) {
        rand_matrix = create_rand_matrix(mat_size); // generate matrix
        print_matrix(rand_matrix, mat_size, mat_size);

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
    if (world_rank == 0) {
        comm_time += MPI_Wtime();
        trans_matrix = create_matrix(mat_size, mat_size);
        comm_time -= MPI_Wtime();
        MPI_Gather(recv_buffer2, mat_size, MPI_INT, &(trans_matrix[0][0]), mat_size, MPI_INT, 0, MPI_COMM_WORLD);
    } else {
        MPI_Gather(recv_buffer2, mat_size, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
    }

    if (world_rank == 0) {
        comm_time += MPI_Wtime();
        process_time += MPI_Wtime();
        // print after transpose
        print_matrix(trans_matrix, mat_size, mat_size);
        //printf("%d\t%d\t%lf\t%lf\n", mat_size, mat_size * mat_size, process_time, comm_time);

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

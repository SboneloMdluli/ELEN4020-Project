#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


int main(int argc, char** argv){

        // Input and Output files

        char *in = argv [1];
        char *out = argv[2];

        // setup
        int rank, numProcs;
        int inputError;
        int SIZE;
        int *sizeChecker;
        int *tempBuffer;
        int n;

        // Initialize MPI
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

        // MPI setup
        MPI_File fh;
        MPI_Win win;
        MPI_Status status;
        MPI_Offset disp;
        MPI_Offset blockSize;

        // Open file
        inputError = MPI_File_open(MPI_COMM_WORLD, in, MPI_MODE_RDONLY,MPI_INFO_NULL,&fh);

        // File succesfully opened?

     if(inputError){

                if (rank == 0) printf("Error! %s: Couldn't open file %s\n", argv[0], argv[1]);
                MPI_Finalize();
                exit(1);
        }

        // Get the size of the matrix
        //sizeChecker = (int*)malloc(sizeof(int));
        //MPI_File_read(fh, sizeChecker, 1, MPI_INT, &status);
        SIZE =128; //N is size, change manually to test

      //  printf("Size %d\n", SIZE);

        //free(sizeChecker);

        n = (int)(SIZE/numProcs);

        // Condition the buffer for each rank
        blockSize = SIZE*n;
        disp = ((rank*blockSize)+1)*sizeof(int);
        tempBuffer = (int*)malloc(SIZE*n*sizeof(int));

        double process_time, process_time2;
        // Read the file into the buffer

        process_time= MPI_Wtime();
        MPI_File_set_view(fh, disp, MPI_INT, MPI_INT,"native",MPI_INFO_NULL);
        MPI_File_read(fh,tempBuffer,blockSize,MPI_INT, &status);
        MPI_File_close(&fh);
        process_time2 = MPI_Wtime();
        printf("%f\n", process_time2-process_time);
        //MPI_Finalize();
        // Confirm
        // for(int i = 0; i < blockSize; i++){

          //              printf("{rank %d} ==> %i [%i]\n", rank, tempBuffer[i], i);

        // }
        MPI_Finalize();
return 0;
}


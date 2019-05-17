#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>


int main(int argc, char* argv[])
{

    int i, j, rank, size, N=128; // N is size, change manually to test

    srand(time(0));

    MPI_File fhw;
    MPI_Status status;
    MPI_Offset offset;
    MPI_Offset chunk;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int buf[(N*N/size)];
    int tmp;

    MPI_File_open(MPI_COMM_WORLD, "data",MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &fhw);

   // offset =1;
   // if (rank ==0)
            //MPI_File_write_at(fhw, offset,&N,1,MPI_INT,&status);



    offset = rank*(N*N/size)*sizeof(int);
    chunk = N*N/size;

    double process_time=0;
    double process_time2=0;

  //  process_time =MPI_Wtime();
    for(i=0;i<((N*N)/size);i++){
        int tmp = rand()%1000;
        for  (j=0; j<i;j++){
                if (tmp =buf[j]){
                        tmp = rand()%1000;
                }
        }
        buf[i] =tmp;
        //MPI_File_write_at(fhw,offset,buf,(N/size),MPI_INT, &status);

    }

    MPI_File_write_at_all(fhw,offset,buf,chunk,MPI_INT, &status);
    MPI_File_close(&fhw);
    //MPI_Finalize();
  //  process_time2 =MPI_Wtime();
 //   printf("%f\n",process_time2-process_time);
    MPI_Finalize();
    return 0;
}


/*
Assignment prepared by Sidharth Kumar (kumar.sidharth87@gmail.com)
*/

#include <unistd.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <mpi.h>


static void parse_args(int argc, char **argv);

// Global dimension of dataset 25600 x 25600
static int  gbs[2] = {2560, 2560};

// Default of 1
static int  stencil_size = 3;

// Name of file to read
static char file_name[512] = "../data/brain_2560_2560.raw";

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);

  int rank, nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  parse_args(argc, argv);

  printf("Rank %d nprocs %d\n", rank, nprocs);

  // READ THE FILE IN PARALLEL
  double io_read_start = MPI_Wtime();
  // STEPS
  // How do you know where in file you want to read the data from
  // Need to compute "offset" in the file where one needs to read (hint: use the rank)
  // Compute the "size" of the local buffer (hint: global image size / nprocs).
  // The size of buffer can also include space for ghost cells (your choice). In case you are allocating buffer for both local image data + ghost cell, then remember that processes with rank 0 and nprocs - 1 will only have one layer of ghost cells as compared to other processes which will have two layers of ghost cells (above and below).
  // allocate local "buffer" to store image data

  // Try posix IO
  // open https://linux.die.net/man/3/open
  // pread https://man7.org/linux/man-pages/man2/pread.2.html
  // close

  // nprocs = 4
  //int of = (#total_file_size/nproc) * rank;
  //P0 of = 0
  //p1 of = 25
  //p2 of = 50
  //p3 of = 75

  //int fp = open (filename);
  //pread(fp, (#total_file_size/nprocs), of, buffer);
  //close

  double io_read_end = MPI_Wtime();
  


  // GHOST (HALO) CELL EXCHANGE
  double communication_start = MPI_Wtime();
  // Allocate buffer to recieve data from neighboring process (hint: will depend on the stencil point size), or use the local buffer of previous step to recieve halo data
  // Identify rank of neighboring process (hint: rank - 1 and rank + 1)
  // Make sure you consider the edge processes rank 0 and rank nprocs - 1, they will recieve and send data from only one process instead of two processes
  // Use MPI_Isend and MPI_Irecv for communication
  // You can or cannot use MPI datatypes for communication
  double communication_end = MPI_Wtime();




  // PERFORM THE ACTUAL BLURING OPERATION
  double compute_start = MPI_Wtime();
  // allocate a new buffer of size equal to local "buffer", that will store the value of the blured image
  // You can skip the border pixels for bluring. Start from (1,1) to (25600 - 1, 25600 - 1)
  // My pixel value = (my pixel value) / 9 + (sum of all neigbor pixel value) / 9
  // To perform the previous step you need to use the ghost cell you obtained in the previous step
  double compute_end = MPI_Wtime();





  // WRITE THE FILE IN PARALLEL (EXACT OPPOSITE of THE FIRST STEP)
  double io_write_start = MPI_Wtime();
  // STEPS
  // How do you know where in file you want to write the data to
  // Need to compute "offset" in the file where one needs to write (hint: use the rank)
  // You already know the "size" of the local buffer (hint: global image size / nprocs)

  // Try out both collective writes (using MPI_File_write_at_all) and one without collectives (using MPI_write_read_at)
  // MPI_File_open
  // MPI_File_write_at_all(..., offset, buffer, size, ..., MPI_UNSIGNED_CHAR, ....)
  // MPI_File_close()
  double io_write_end = MPI_Wtime();




  double total_time = io_write_end - io_read_start;
  double max_time;
  MPI_Allreduce(&total_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
  if (max_time == total_time)
  {
    printf("Time take to blur %d x %d image is %f \n Time decomposistion (IO + COMM + COMP + IO): %f + %f + %f + %f \n", gbs[0], gbs[1], max_time, (io_read_end - io_read_start), 
    (communication_end - communication_start), 
    (compute_end - compute_start), 
    (io_write_end - io_write_start));
  }


  MPI_Finalize();
}


static void parse_args(int argc, char **argv)
{
  char flags[] = "g:s:f:";
  int one_opt = 0;
  int with_rst = 0;

  while ((one_opt = getopt(argc, argv, flags)) != EOF)
  {
    /* postpone error checking for after while loop */
    switch (one_opt)
    {
    case('g'): // global dimension
      if ((sscanf(optarg, "%dx%dx%d", &gbs[0], &gbs[1], &gbs[2]) == EOF))
        exit(-1);
      break;

    case('s'): // local dimension
      if ((sscanf(optarg, "%d", &stencil_size) == EOF))
        exit(-1);
      break;

    case('f'): // output file name
      if (sprintf(file_name, "%s", optarg) < 0)
        exit(-1);  
      break;

    }
  }
}

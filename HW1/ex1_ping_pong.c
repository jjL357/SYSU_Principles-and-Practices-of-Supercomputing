/*
 * Purpose:  Using MPI to imitate a table tennis match.
 *           
 * Compile:  mpicc -g -Wall -o ex1_ping_pong ex1_ping_pong.c
 * Run:      mpiexec ./ex1_ping_pong
 *
 * Input:    None
 * Output:   None
 *
 * Note:    On each turn, please print out the rank for each process and the value of "ping_pong_count".
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  const int PING_PONG_LIMIT = 10;

  // Initialize the MPI environment
  MPI_Init(NULL, NULL);
  // Find out rank, size
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // We are assuming 2 processes for this task
  if (world_size != 2) {
    fprintf(stderr, "World size must be two for %s\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int ping_pong_count = 0;
  int receive_ping_pong_count=0;
  /*******************************************************************/


   while(ping_pong_count<PING_PONG_LIMIT || ping_pong_count==PING_PONG_LIMIT){//大于limit时退出循环
    if(world_rank==0){
        ping_pong_count++;
        if(ping_pong_count>PING_PONG_LIMIT)break;//大于limit时退出循环
        printf("Process 0 sent and incremented ping_pong_count(value=%d) to process 1\n",ping_pong_count);
        MPI_Send(&ping_pong_count,1,MPI_INT,1,0,MPI_COMM_WORLD);//向1核发送消息即ping_pong_count
        MPI_Recv(&ping_pong_count,1,MPI_INT,1,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);//接收1核消息
        if(ping_pong_count>PING_PONG_LIMIT)break;//大于limit时退出循环
     	printf("Process 0 received ping_pong_count(value=%d) from process 1\n",ping_pong_count);
        
    }
    else{
        MPI_Recv(&ping_pong_count,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);//接收0核消息
        printf("Process 1 received ping_pong_count(value=%d) from process 0\n",ping_pong_count);
        ping_pong_count++;
        if(ping_pong_count>PING_PONG_LIMIT)break;//大于limit时退出循环
        printf("Process 1 sent and incremented ping_pong_count(value=%d) to process 0\n",ping_pong_count);
        MPI_Send(&ping_pong_count,1,MPI_INT,0,1,MPI_COMM_WORLD);//向0核发送消息即ping_pong_count
      	

    }
   }


   /******************************************************************/
  MPI_Finalize();
  return 0;
}
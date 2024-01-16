/* File:     ex1_pi_mutex.c
 *
 * Purpose:  Estimate (pi^2)/6 using series
 *
 *              (pi^2)/6 = [1/(1^2) + 1/(2^2) + 1/(3^2) + 1/(4^2) . . . ]
 *
 *           It uses a semaphore to protect the critical section.
 *
 * Compile:  gcc -g -Wall -o ex1_pi_mutex ex1_pi_mutex.c -lpthread
 *           timer.h needs to be available
 * Run:      ./ex1_pi_mutex <number of threads> <n>
 *      or   ./ex1_pi_mutex  (using defalut)
 *           n is the number of terms of the Maclaurin series to use
 *           n should be evenly divisible by the number of threads
 *
 * Input:    none
 * Output:   The estimate of pi using multiple threads, one thread, and the
 *           value computed by the math library arctan function
 *           Also elapsed times for the multithreaded and singlethreaded
 *           computations.
 *
 * Notes:
 *    1.  The radius of convergence for the series is only 1.  So the
 *        series converges quite slowly.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}


const int MAX_THREADS = 1024;

long thread_count;
long  long n;
long double sum;

sem_t sem;

void* Thread_sum(void* rank);

/* Only executed by main thread */
void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
double Serial_pi(long long n);

int main(int argc, char* argv[]) {
   long       thread;  /* Use long in case of a 64-bit system */
   pthread_t* thread_handles;
   double start, finish, elapsed;

   /* please choose terms 'n', and the threads 'thread_count' here. */
   n = 100000000;
   thread_count = 4;

   /* You can also get number of threads from command line */
   //Get_args(argc, argv);

   thread_handles = (pthread_t*) malloc (thread_count*sizeof(pthread_t));
   sem_init(&sem, 0, 1);
   sum = 0.0;
   GET_TIME(start);
    for (thread = 0; thread < thread_count; thread++)
       pthread_create(&thread_handles[thread], NULL,
           Thread_sum, (void*)thread);

    for (thread = 0; thread < thread_count; thread++)
       pthread_join(thread_handles[thread], NULL);
    GET_TIME(finish);
    elapsed = finish - start;
    // sum = 4.0*sum;
       printf("With n = %lld terms,\n", n);
       printf("   Our estimate of pi = %.15Lf\n", sum);
       printf("The elapsed time is %e seconds\n", elapsed);
       GET_TIME(start);
       sum = Serial_pi(n);
       GET_TIME(finish);
       elapsed = finish - start;
       printf("   Single thread est  = %.15Lf\n", sum);
       printf("The elapsed time is %e seconds\n", elapsed);
       printf("                   pi = %.15lf\n", (4.0*atan(1.0))*(4.0*atan(1.0))/6 );
       	sem_destroy(&sem);
          free(thread_handles);
          return 0;
}  /* main */

void* Thread_sum(void* rank) {
    long long  my_rank = (long long) rank;//获取自己的线程号
    long double my_sum = 0.0;//线程计算的和
    
    /*******************************************************************/


    
    long long local_n=n/thread_count;//此线程计算的个数
    long long start=1+(my_rank)*local_n;//此线程计算范围的开始
    long long end=(my_rank+1)*local_n;//此线程计算范围的结束
    //对本线程计算的范围求和
    for(long long i=start;i<=end;i++){
   
    my_sum += 1.0 / (i*i);
 
    }
   sem_wait(&sem);//sem_wait可以用来阻塞当前线程，直到信号量的值大于0，解除阻塞,解除阻塞后，sem的值-1，表示公共资源被执行减少了
   sum += my_sum;//全局变量sum加上本线程计算的和
   sem_post(&sem);//当有线程阻塞在这个信号量上时，调用这个函数会使其中的一个线程不再阻塞


   /******************************************************************/

    return NULL;
}  /* Thread_sum */

double Serial_pi(long long n) {
                long double sum = 0.0;
                long long i;

                for ( i = 1; i <= n; i++ ) {
                   sum += 1.0 / (i*i);
                }
                return sum;

}  /* Serial */
                
void Get_args(int argc, char* argv[]) {
                   if (argc != 3) Usage(argv[0]);
                   thread_count = strtol(argv[1], NULL, 10);
                   if (thread_count <= 0 || thread_count > MAX_THREADS) Usage(argv[0]);
                   n = strtoll(argv[2], NULL, 10);
                   if (n <= 0) Usage(argv[0]);
                }  /* Get_args */


void Usage(char* prog_name) {
                      fprintf(stderr, "usage: %s <number of threads> <n>\n", prog_name);
                      fprintf(stderr, "   n is the number of terms and should be >= 1\n");
                      fprintf(stderr, "   n should be evenly divisible by the number of threads\n");
                      exit(0);
}  /* Usage */


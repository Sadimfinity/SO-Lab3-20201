/**
 * @defgroup   SAXPY saxpy
 *
 * @brief      This file implements an iterative saxpy operation
 * 
 * @param[in] <-p> {vector size} 
 * @param[in] <-s> {seed}
 * @param[in] <-n> {number of threads to create} 
 * @param[in] <-i> {maximum itertions} 
 *
 * @author     Danny Munera
 * @date       2020
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

sem_t mutex;

void *compute(void *);

typedef struct _param
{
  int ini;
  int end;
  double *X;
  double a;
  double *Y;
  double *Y_avgs;
  int it;
  int p;
  int max_iters;
} param_t;

void *compute(void *arg)
{ //Funcion que es ejecutada por los hilos
  param_t *par = (param_t *)arg;
  int ini = par->ini;
  int end = par->end;
  double *X = par->X;
  double a = par->a;
  double *Y = par->Y;
  double *Y_avgs = par->Y_avgs;
  int it = par->it;
  int p = par->p;
  int i;
  int max_iters = par->max_iters;
  double acc;
#ifdef DEBUG
  printf("Thread values start = %d, end = %d, max_iters = %d, p = %d \n", ini, end, max_iters, p);
#endif
  //SAXPY iterative SAXPY mfunction
  acc = 0; //Variable local para optimizar el calculo del promedio
  for (i = ini; i < end; i++)
  {
    Y[i] = Y[i] + a * X[i];
    acc += Y[i];
  }
  sem_wait(&mutex);
  Y_avgs[it] += acc / p; //Seccion critica protegida con semaforo binario
  sem_post(&mutex);

  return NULL;
}


int main(int argc, char *argv[])
{

  int it;
  int p;
  unsigned int seed = 1;
  int n_threads = 2;
  int max_iters = 1000;

  // Variables to perform SAXPY operation
  double *X;
  double a;
  double *Y;
  double *Y_avgs;
  int i;
  // Variables to get execution time
  struct timeval t_start, t_end;
  double exec_time;
  // Getting input values
  int opt;
  while ((opt = getopt(argc, argv, ":p:s:n:i:")) != -1)
  {
    switch (opt)
    {
    case 'p':
      printf("vector size: %s\n", optarg);
      p = strtol(optarg, NULL, 10);
      assert(p > 0 && p <= 2147483647);
      break;
    case 's':
      printf("seed: %s\n", optarg);
      seed = strtol(optarg, NULL, 10);
      break;
    case 'n':
      printf("threads number: %s\n", optarg);
      n_threads = strtol(optarg, NULL, 10);
      break;
    case 'i':
      printf("max. iterations: %s\n", optarg);
      max_iters = strtol(optarg, NULL, 10);
      break;
    case ':':
      printf("option -%c needs a value\n", optopt);
      break;
    case '?':
      fprintf(stderr, "Usage: %s [-p <vector size>] [-s <seed>] [-n <threads number>]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  srand(seed);

  printf("p = %d, seed = %d, n_threads = %d, max_iters = %d\n",
         p, seed, n_threads, max_iters);

  // initializing data
  X = (double *)malloc(sizeof(double) * p);
  Y = (double *)malloc(sizeof(double) * p);
  Y_avgs = (double *)malloc(sizeof(double) * max_iters);

  for (i = 0; i < p; i++)
  {
    X[i] = (double)rand() / RAND_MAX;
    Y[i] = (double)rand() / RAND_MAX;
  }
  for (i = 0; i < max_iters; i++)
  {
    Y_avgs[i] = 0.0;
  }
  a = (double)rand() / RAND_MAX;

#ifdef DEBUG
  printf("vector X= [ ");
  for (i = 0; i < p - 1; i++)
  {
    printf("%f, ", X[i]);
  }
  printf("%f ]\n", X[p - 1]);

  printf("vector Y= [ ");
  for (i = 0; i < p - 1; i++)
  {
    printf("%f, ", Y[i]);
  }
  printf("%f ]\n", Y[p - 1]);

  printf("a= %f \n", a);
#endif

  /*
	 *	Function to parallelize 
	 */
  gettimeofday(&t_start, NULL);
  //SAXPY iterative SAXPY mfunction

  //hilos aca
  sem_init(&mutex, 0, 1);
  switch (n_threads)
  {
  case 1:
  {
    pthread_t h1;

    param_t param1;

    param1.ini = 0;
    param1.end = p;
    param1.X = X;
    param1.Y = Y;
    param1.a = a;
    param1.Y_avgs = Y_avgs;
    param1.p = p;
    param1.max_iters = max_iters;
    pthread_create(&h1, NULL, &compute, &param1);
    for (it = 0; it < max_iters; it++)
    {
      param1.it = it;
    }
    pthread_join(h1, NULL);
    break;
  }

  case 2:
  {
    pthread_t h1;
    pthread_t h2;

    param_t param1;
    param_t param2;

    param1.ini = 0;
    param1.end = p / 2;
    param1.X = X;
    param1.Y = Y;
    param1.a = a;
    param1.Y_avgs = Y_avgs;
    param1.p = p;
    param1.max_iters = max_iters;

    param2.ini = p / 2;
    param2.end = p;
    param2.X = X;
    param2.Y = Y;
    param2.a = a;
    param2.Y_avgs = Y_avgs;
    param2.p = p;
    param2.max_iters = max_iters;

    pthread_create(&h1, NULL, &compute, &param1);
    pthread_create(&h2, NULL, &compute, &param2);

    for (it = 0; it < max_iters; it++)
    {

      param1.it = it;
      param2.it = it;
    }

    pthread_join(h1, NULL);
    pthread_join(h2, NULL);
    break;
  }
  case 4:
  {
    pthread_t h1;
    pthread_t h2;
    pthread_t h3;
    pthread_t h4;

    param_t param1;
    param_t param2;
    param_t param3;
    param_t param4;

    param1.ini = 0;
    param1.end = p / 4;
    param1.X = X;
    param1.Y = Y;
    param1.a = a;
    param1.Y_avgs = Y_avgs;
    param1.p = p;
    param1.max_iters = max_iters;

    param2.ini = p / 4;
    param2.end = p / 2;
    param2.X = X;
    param2.Y = Y;
    param2.a = a;
    param2.Y_avgs = Y_avgs;
    param2.p = p;
    param2.max_iters = max_iters;

    param3.ini = p / 2;
    param3.end = 3 * p / 4;
    param3.X = X;
    param3.Y = Y;
    param3.a = a;
    param3.Y_avgs = Y_avgs;
    param3.p = p;
    param3.max_iters = max_iters;

    param4.ini = 3 * p / 4;
    param4.end = p;
    param4.X = X;
    param4.Y = Y;
    param4.a = a;
    param4.Y_avgs = Y_avgs;
    param4.p = p;
    param4.max_iters = max_iters;

    pthread_create(&h1, NULL, &compute, &param1);
    pthread_create(&h2, NULL, &compute, &param2);
    pthread_create(&h3, NULL, &compute, &param3);
    pthread_create(&h4, NULL, &compute, &param4);

    for (it = 0; it < max_iters; it++)
    {
      param1.it = it;
      param2.it = it;
      param3.it = it;
      param4.it = it;
    }

    pthread_join(h1, NULL);
    pthread_join(h2, NULL);
    pthread_join(h3, NULL);
    pthread_join(h4, NULL);
    break;
  }
  case 8:
  {
    pthread_t h1;
    pthread_t h2;
    pthread_t h3;
    pthread_t h4;
    pthread_t h5;
    pthread_t h6;
    pthread_t h7;
    pthread_t h8;

    param_t param1;
    param_t param2;
    param_t param3;
    param_t param4;
    param_t param5;
    param_t param6;
    param_t param7;
    param_t param8;

    param1.ini = 0;
    param1.end = p / 8;
    param1.X = X;
    param1.Y = Y;
    param1.a = a;
    param1.Y_avgs = Y_avgs;
    param1.p = p;
    param1.max_iters = max_iters;

    param2.ini = p / 8;
    param2.end = p / 4;
    param2.X = X;
    param2.Y = Y;
    param2.a = a;
    param2.Y_avgs = Y_avgs;
    param2.p = p;
    param2.max_iters = max_iters;

    param3.ini = p / 4;
    param3.end = 3*p / 8;
    param3.X = X;
    param3.Y = Y;
    param3.a = a;
    param3.Y_avgs = Y_avgs;
    param3.p = p;
    param3.max_iters = max_iters;

    param4.ini = 3*p / 8;
    param4.end = p / 2;
    param4.X = X;
    param4.Y = Y;
    param4.a = a;
    param4.Y_avgs = Y_avgs;
    param4.p = p;
    param4.max_iters = max_iters;

    param5.ini = p / 2;
    param5.end = 5*p / 8;
    param5.X = X;
    param5.Y = Y;
    param5.a = a;
    param5.Y_avgs = Y_avgs;
    param5.p = p;
    param5.max_iters = max_iters;

    param6.ini = 5*p / 8;
    param6.end = 3*p / 4;
    param6.X = X;
    param6.Y = Y;
    param6.a = a;
    param6.Y_avgs = Y_avgs;
    param6.p = p;
    param6.max_iters = max_iters;

    param7.ini = 3 * p / 4;
    param7.end = 7 * p / 8;
    param7.X = X;
    param7.Y = Y;
    param7.a = a;
    param7.Y_avgs = Y_avgs;
    param7.p = p;
    param7.max_iters = max_iters;

    param8.ini = 7 * p / 8;
    param8.end = p;
    param8.X = X;
    param8.Y = Y;
    param8.a = a;
    param8.Y_avgs = Y_avgs;
    param8.p = p;
    param8.max_iters = max_iters;

    pthread_create(&h1, NULL, &compute, &param1);
    pthread_create(&h2, NULL, &compute, &param2);
    pthread_create(&h3, NULL, &compute, &param3);
    pthread_create(&h4, NULL, &compute, &param4);
    pthread_create(&h5, NULL, &compute, &param5);
    pthread_create(&h6, NULL, &compute, &param6);
    pthread_create(&h7, NULL, &compute, &param7);
    pthread_create(&h8, NULL, &compute, &param8);

    for (it = 0; it < max_iters; it++)
    {
      param1.it = it;
      param2.it = it;
      param3.it = it;
      param4.it = it;
      param5.it = it;
      param6.it = it;
      param7.it = it;
      param8.it = it;
    }

    pthread_join(h1, NULL);
    pthread_join(h2, NULL);
    pthread_join(h3, NULL);
    pthread_join(h4, NULL);
    pthread_join(h5, NULL);
    pthread_join(h6, NULL);
    pthread_join(h7, NULL);
    pthread_join(h8, NULL);
    break;
  }
  default:
    break;
  }

  // hilos finaliza

  gettimeofday(&t_end, NULL);

#ifdef DEBUG
  printf("RES: final vector Y= [ ");
  for (i = 0; i < p - 1; i++)
  {
    printf("%f, ", Y[i]);
  }
  printf("%f ]\n", Y[p - 1]);
#endif

  // Computing execution time
  exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;    // sec to ms
  exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms
  printf("Execution time: %f ms \n", exec_time);
  printf("Last 3 values of Y: %f, %f, %f \n", Y[p - 3], Y[p - 2], Y[p - 1]);
  printf("Last 3 values of Y_avgs: %f, %f, %f \n", Y_avgs[max_iters - 3], Y_avgs[max_iters - 2], Y_avgs[max_iters - 1]);
  return 0;
}
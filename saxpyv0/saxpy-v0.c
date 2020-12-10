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

void *saxpy(void *);

double *X;
double a;
double *Y;
double *Y_avgs;
sem_t mutex;

typedef struct _param
{
    int ini;
    int end;
    int it;
    int p;
    int max_iters;
} parameters;

int main(int argc, char *argv[])
{
    unsigned int seed = 1;
    int p = 10000000;
    int n_threads = 2;
    int max_iters = 1000;
    int i, t;
    struct timeval t_start, t_end;
    double exec_time;

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

    gettimeofday(&t_start, NULL);

    pthread_t threads[n_threads];
    parameters p_threads[n_threads];

    sem_init(&mutex, 0, 1);

    /**
     * Ciclo principal para crear y ejecutar cada uno de los n hilos basados en el valor
     * ingresado por el usuario.
     * Nótese que el inicio y final de cada hilo está determinado por la operación:
     * inicio = (p / n_threads) * t
     * fin = (p / n_threads) * (t + 1)
     * En la creación del hilo se le asignan los parámetros previamente asignados y la
     * función saxpy.
    */

    for (t = 0; t < n_threads; t++)
    {
        p_threads[t].ini = (p / n_threads) * t;
        p_threads[t].end = (p / n_threads) * (t + 1);
        p_threads[t].p = p;
        p_threads[t].max_iters = max_iters;

        pthread_create(&threads[t], NULL, &saxpy, &p_threads[t]);
    }

    /**
     * Aquí se realiza un ciclo aparte para los join debido a que, en el caso en el que
     * estos se hagan en el mismo ciclo (create y join), tendremos un problema de 
     * sincronización afectando el rendimiento dado que cada uno de los hilos tendrá 
     * que esperar que los demás terminen.
    */
    for (t = 0; t < n_threads; t++) pthread_join(threads[t], NULL);

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

void *saxpy(void *arg)
{
    parameters *par = (parameters *)arg;
    int ini = par->ini;
    int end = par->end;
    int it = par->it;
    int p = par->p;
    int i;
    int max_iters = par->max_iters;
    double sumY;
    /**
     * Aquí se ejecutará el ciclo de máximas iteraciones definido por el usuario con
     * el parámetro p, como vemos este se ejecutará por cada uno de los hilos que se
     * creen.
     * Se utiliza un semáforo para proteger la variable que almacenará los promedios,
     * debido a que todos los hilos pueden acceder a ella lo que crearía una race
     * condition.
    */
    for (it = 0; it < max_iters; it++)
    {
        sumY = 0; 
        for (i = ini; i < end; i++)
        {
            Y[i] = Y[i] + a * X[i];
            sumY += Y[i];
        }
        sem_wait(&mutex);
        Y_avgs[it] += sumY / p; 
        sem_post(&mutex);
    }

    return NULL;
}

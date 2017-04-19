#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

/* maximum number of Philosophers to initalize the array of chopsticks */
#define MAX_PHI 27


/* variables for input parameters */
static int numPhil;
static int minThink;
static int maxThink;
static int minEat;
static int maxEat;
static int dist; // will take a value of 0 for "uniform" or 1 for "exponential"
static int count;

/* Mean Variables*/
static int meanThink;
static int meanEat;

/* mutex locks */
static pthread_mutex_t chopsticks[MAX_PHI]; /* chopsticks to eat with */
static pthread_mutex_t pickup = PTHREAD_MUTEX_INITIALIZER; /* Protexts chopstics, Only one philosopher can pick up chopstics at a time */
static pthread_mutex_t release = PTHREAD_MUTEX_INITIALIZER; /* Only oen philosopher cna put down chopsticks at a time */
static pthread_mutex_t parentLock = PTHREAD_MUTEX_INITIALIZER; /* Used at the start when initalizing variables */
static pthread_mutex_t childLock = PTHREAD_MUTEX_INITIALIZER; /* Used at the start when initalizing variables */

/* Function to generate exponential variate for thinking*/
int randThink()
{
    double mu = (float)rand()/(float)(RAND_MAX) ;
    double x = log(mu) * - 1;
    x = x * meanThink;
    while( x < minThink || x > maxThink){
        mu = (float)rand()/(float)(RAND_MAX);
        x = log(mu) * -1;
        x = x * meanThink;

    }
    return x;
}
/* Function to generate exponential variate for eating*/
int randEat()
{
    double mu = (float)rand()/(float)(RAND_MAX);
    double x = log(mu) * -1;
    x = x / meanEat;
    while( x < minEat || x > maxEat){
        mu = (float)rand()/(float)(RAND_MAX);
        x = log(mu) * -1;
        x = x * meanEat;
    }
    return x;
}

/* Initialize the static variables, will warn the user if input arguments are not proper, main extits if -1 is returned */
int init( char **argv)
{
    numPhil = atoi(argv[1]);
    minThink = atoi( argv[2]) * 1000;
    maxThink = atoi( argv[3]) * 1000;
    minEat = atoi( argv[4]) * 1000;
    maxEat = atoi( argv[5]) * 1000;
    if( strcmp( argv[6], "uniform") == 0){
        dist = 0;
    }
    else if( strcmp( argv[6], "exponential") == 0){
        dist = 1;
    }
    else{
        printf( "error, write \"uniform\" or \"exponential\"   as your 6th input");
        return -1;
    }
    count = atoi( argv [7]);

    meanThink = (maxThink + minThink) / 2;
    meanEat = (maxEat + minEat) / 2;
    return 0;
}

/* Thinking function */
void think()
{
    int thinkTime;
    if(dist == 1){ /* if dist is 1 then we use exponential distribution*/
        thinkTime = randThink();
        usleep(thinkTime);
    }
    else{
        thinkTime = rand() % (maxThink - minThink) ;
        usleep(thinkTime);
    }
}

/* Philosopher thread function*/
void* Phil_Thread( void* arg)
{
    pthread_mutex_lock(&parentLock);
    /* critical section for parameter arg */
    int* np = (int*) arg;
    int n = *np;
    pthread_mutex_unlock(&childLock);
    int c = 0;
    int t = 0;
    clockid_t clk;
    clockid_t clkHungry;

    for( c = 0; c < count; c++){
        printf( "$ Philosopher <%d> <Thinking>\n", n + 1);
        think();
        clk = clock();
        printf( "$ Philosopher <%d> <Hungry>\n", n + 1);
        if( dist){
            t = randEat();
        }
        else{
            t = (rand() % (maxEat - minEat)) + minEat;
        }
        clk = clock() - clk;
        clkHungry = clkHungry + clk;
        pthread_mutex_lock(&pickup);
        pthread_mutex_lock(&chopsticks[ (n+1) % numPhil]);
        pthread_mutex_lock(&chopsticks[ (n) % numPhil]);
        pthread_mutex_unlock(&pickup);

        /* Philosopher eating section */
        printf( "$ Philosopher <%d> <Eating>\n", n + 1);
        usleep( t);

        pthread_mutex_lock(&release);
        pthread_mutex_unlock( &chopsticks[ (n+1) % numPhil]);
        pthread_mutex_unlock( &chopsticks[ (n) % numPhil]);
        pthread_mutex_unlock( &release);
    }

    printf( "$ philosopher <%d> duration of hungry state = <%d>\n", n + 1, clkHungry);
    pthread_exit(0);
}

int main( int argc, char *argv[])
{
/*
    char *argv[8];
    argv[1] = "5";
    argv[2] = "500";
    argv[3] = "1000";
    argv[4] = "50";
    argv[5] = "100";
    argv[6] = "exponential";
    argv[7] = "100";
*/


    int cond = init( argv);

    if (cond == -1){
        return 0;
    }

    pthread_t tid[numPhil];
    int ret;
    int par = 0;

    /* Initialize the chopstics */
    int i = 0;
    for( i = 0; i < MAX_PHI; i++){
        pthread_mutex_init( &chopsticks[i], NULL);
    }


    /* Start philosopher threads */
    for( i = 0; i < numPhil; i++){
        pthread_mutex_lock(&childLock);
        par = i;
        ret = pthread_create( &(tid[i]), NULL, Phil_Thread, (void *) &par );
        pthread_mutex_unlock(&parentLock);
    }

    /* Join all threads and finish */
    for ( i = 0; i < numPhil; i++){
        ret = pthread_join(tid[i], NULL);
    }
    return 0;
}

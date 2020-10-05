#include <pthread.h> // thread functionality library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h> /* for times system call */
#include <sys/time.h>  /* for gettimeofday system call */
#include <unistd.h>
#include <error.h>
#include "lab.h"

// function prototypes
int check_if_sorted(int A[], int n);
void generate_random_array(int A[], int n, int seed);
void serial_mergesort(int A[], int p, int r);
void merge(int A[], int p, int q, int r);
void insertion_sort(int A[], int p, int r);
double getMilliSeconds(void);

// my function prototypes
void* parrallel_mergesort(void * arg);
void printinfo(int array[], int n);
void dataModeMain(void);

int print;
int dataMode;

int currentLevel = 0;
int totalLevels = 0;
int threadsUsed = 1;

int *masterArray;
int arrayLength;

struct threadData{
int p;
int q;
int r;
};

/*
---------------------------------------------------------------------------
clock_t times(struct tms *buffer);
times() fills the structure pointed to by buffer with
time-accounting information.  The structure defined in
<sys/times.h> is as follows:
struct tms {
    clock_t tms_utime;       user time
    clock_t tms_stime;       system time
    clock_t tms_cutime;      user time, children
    clock_t tms_cstime;      system time, children
The time is given in units of 1/CLK_TCK seconds where the
value of CLK_TCK can be determined using the sysconf() function
with the agrgument _SC_CLK_TCK.
---------------------------------------------------------------------------
*/

int main(int argc, char **argv) {

	if (argc < 2) { // there must be at least one command-line argument
			fprintf(stderr, "Usage: mytests <input size> [<seed>] [<number of lvls>] [<enable helper printf()>] [<enable efficiency data mode>] \n");
			exit(1);
	}

	int n = atoi(argv[1]);
	int seed = 1;
	if (argc >= 3)
		seed = atoi(argv[2]);
  
  int lvl = 1; // default threads
  if (argc >= 4)
    lvl = atoi(argv[3]);
  
  if (argc >= 5)
    print = atoi(argv[4]);
    
  if(argc == 6)
    dataMode = atoi(argv[5]);
  
  if(argc > 6){ // if too many arguments used
			fprintf(stderr, "Usage: mytests <input size> [<seed>] [<number of lvls>] [<enable helper printf()>] [<enable efficiency data mode>] \n");
			exit(1);
	}   
  
  printf("\ninputSize: %d, seed: %d, lvl: %d, printMode: %d, dataMode: %d \n",n,seed,lvl,print,dataMode);
  
  if(dataMode){
    dataModeMain();
    exit (EXIT_SUCCESS);
  }
  
	int *A = (int *) malloc(sizeof(int) * (n+1)); // n+1 since we are using A[1]..A[n]
 
	// generate random input
	generate_random_array(A,n, seed);
  
	double start_time;
	double sorting_time;

	// sort the input (and time it)
	start_time = getMilliSeconds();
	serial_mergesort(A,1,n);
	sorting_time = getMilliSeconds() - start_time;
 
  // print results if correctly sorted otherwise cry foul and exit
	if (check_if_sorted(A,n)) {
		printf("\nSorting %d elements took %4.2lf seconds.\n", n,  sorting_time/1000.0);
	} else {
		printf("%s: sorting failed!!!!\n", argv[0]);
		exit(EXIT_FAILURE);
	}
  
  if(print)
	  printinfo(A, n);
  
  printf("Default has completed.\n");
  free(A);
  
  // START OF MY CODE: generate random input to be used by threads.
  start_time = 0;
  sorting_time = 0;
  long i = 1;
  masterArray = (int *)malloc( (sizeof(int) * (n+1)) );
  
  if(lvl > 1)
    for(i=0; i < lvl-1; i++)
      threadsUsed *= 2;
	// generate random input
  generate_random_array(masterArray,n,seed);
 
  //determine how many levels of threads
  //printf("\nThreads to be used: %d\n", threadsUsed);

  // thread critical values
  totalLevels = lvl;
  arrayLength = n;
  currentLevel= 1;
  
  pthread_t tid[1]; // init threads, for starters we only have 1 thread.
  int error;
  
  struct threadData *d = (struct threadData*)malloc(sizeof(d));
  d->p = 1;
  d->r = arrayLength;
  d->q = d->r + (d->p - d->r)/2;
  
  start_time = getMilliSeconds();
  
  error = pthread_create(&(tid[0]), NULL, &parrallel_mergesort, (void*)d);
  if(error != 0)
    printf("\n Thread can't be created : [%s]", strerror(error));
  
  pthread_join(tid[0], NULL); // ends thread.
  
  sorting_time = getMilliSeconds() - start_time;
	
  if(print)
    printinfo(masterArray, n);
  
	// print results if correctly sorted otherwise cry foul and exit
	if (check_if_sorted(masterArray, n)) {
		printf("\nSorting %d elements took %4.2lf seconds.\n", n,  sorting_time/1000.0);
	} 
  else {
		printf("%s: sorting failed!!!!\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	free(masterArray);
  free(d);
	exit(EXIT_SUCCESS);
}

// MY METHODS

void* parrallel_mergesort(void * arg)
{  
  struct threadData *tD = arg;
  int left = tD->p;
  int right = tD->r;
  int middle = right + (left - right)/2 - 1;
  
  if(totalLevels == 1){
    serial_mergesort(masterArray, 1, arrayLength);
    return NULL;
  }
  else if(currentLevel < totalLevels){
    currentLevel++;
    //printf("\nLeft:%d\n",left);
    //printf("\nRight:%d\n",right);
    //printf("\nMiddle:%d\n",middle);
    
    pthread_t tid[2];
    
    struct threadData *newtD1 = (struct threadData*)malloc(sizeof(newtD1));
    newtD1->p = left;
    newtD1->r = middle;
    
    //startThread1
    int error;
    error = pthread_create(&(tid[0]), NULL, &parrallel_mergesort, (void*)newtD1);
    if(error != 0)
      printf("\n Thread can't be created : [%s]", strerror(error));
    
    struct threadData *newtD2 = (struct threadData*)malloc(sizeof(newtD2));
    newtD2->p = middle+1;
    newtD2->r = right;
    
    //startThread2
    error = pthread_create(&(tid[1]), NULL, &parrallel_mergesort, (void*)newtD2);
    if(error != 0)
      printf("\n Thread can't be created : [%s]", strerror(error));
      
    // both threads end.
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    
    merge(masterArray, left, middle, right);
    free(newtD1);
    free(newtD2);
    return NULL;
  }
  else
  {
    serial_mergesort(masterArray, left, right);
    return NULL;
  }
  
}

void printinfo(int A[], int n)
{
  int i;
  printf("\n");
  for(i=0; i < n; i++)
    printf("%d,",A[i]);
  printf("\n");
}

void dataModeMain(void){
  printf("success\n");
  int n = 1000000;
  int seed = 1;
  int lvl = 3; // 4 threads
  
  FILE *fp;

  fp = fopen("data.txt", "w+");
  fprintf(fp, "Beginning of text file\n");
  
  int i;
  for(i=0;i<100;i++)
  {
    int *A = (int *) malloc(sizeof(int) * (n+1)); // n+1 since we are using A[1]..A[n]
     
  	// generate random input
  	generate_random_array(A,n, seed);
    
  	double start_time;
  	double sorting_time;
  
  	// sort the input (and time it)
  	start_time = getMilliSeconds();
  	serial_mergesort(A,1,n);
  	sorting_time = getMilliSeconds() - start_time;
   
    // print results if correctly sorted otherwise cry foul and exit
  	if (check_if_sorted(A,n)) {
  		printf("\nSorting %d elements took %4.2lf seconds.\n", n,  sorting_time/1000.0);
      fprintf(fp, "Serial, %d, %4.2lf", n,  sorting_time/1000.0);
  	} 
    else {
  		printf("mytests: sorting failed!!!!\n");
  		exit(EXIT_FAILURE);
  	}
    
    if(print)
  	  printinfo(A, n);
    
    printf("Default has completed.\n");
    free(A);
    
    // START OF MY CODE: generate random input to be used by threads.
    start_time = 0;
    sorting_time = 0;
    long i = 1;
    masterArray = (int *)malloc( (sizeof(int) * (n+1)) );
    
    if(lvl > 1)
      for(i=0; i < lvl-1; i++)
        threadsUsed *= 2;
  	// generate random input
    generate_random_array(masterArray,n,seed);
   
    //determine how many levels of threads
    //printf("\nThreads to be used: %d\n", threadsUsed);
  
    // thread critical values
    totalLevels = lvl;
    arrayLength = n;
    currentLevel= 1;
    
    pthread_t tid[1]; // init threads, for starters we only have 1 thread.
    int error;
    
    struct threadData *d = (struct threadData*)malloc(sizeof(d));
    d->p = 1;
    d->r = arrayLength;
    d->q = d->r + (d->p - d->r)/2;
    
    start_time = getMilliSeconds();
    
    error = pthread_create(&(tid[0]), NULL, &parrallel_mergesort, (void*)d);
    if(error != 0)
      printf("\n Thread can't be created : [%s]", strerror(error));
    
    pthread_join(tid[0], NULL); // ends thread.
    
    sorting_time = getMilliSeconds() - start_time;
  	
    if(print)
      printinfo(masterArray, n);
    
  	// print results if correctly sorted otherwise cry foul and exit
  	if (check_if_sorted(masterArray, n)) {
  		printf("\nSorting %d elements took %4.2lf seconds.\n", n,  sorting_time/1000.0);
      fprintf(fp, "Threaded, %d, %4.2lf", n,  sorting_time/1000.0);
  	} 
    else {
  		printf("mytests: sorting failed!!!!\n");
  		exit(EXIT_FAILURE);
  	}
  	free(masterArray);
    free(d);
    
    n += 1000000;
    
  }
  fclose(fp);
}

//HELPER METHODS
float report_cpu_time(void)
{
	struct tms buffer;
	float cputime;

	times(&buffer);
	cputime = (buffer.tms_utime)/ (float) sysconf(_SC_CLK_TCK);
	return (cputime);
}


float report_sys_time(void)
{
	struct tms buffer;
	float systime;

	times(&buffer);
	systime = (buffer.tms_stime)/ (float) sysconf(_SC_CLK_TCK);
	return (systime);
}

double getMilliSeconds(void)
{
    struct timeval now;
    gettimeofday(&now, (struct timezone *)0);
    return (double) now.tv_sec*1000.0 + now.tv_usec/1000.0;
}


/*
 * generate_random_array(int A[], int n, int seed):
 *
 * description: Generate random integers in the range [0,RANGE]
 *              and store in A[1..n]
 */

#define RANGE 1000000

void generate_random_array(int A[], int n, int seed)
{
    int i;

	srandom(seed);
    for (i=1; i<=n; i++)
        A[i] = random()%RANGE;
}


/*
 * check_if_sorted(int A[], int n):
 *
 * description: returns TRUE if A[1..n] are sorted in nondecreasing order
 *              otherwise returns FALSE
 */

int check_if_sorted(int A[], int n)
{
	int i=0;

	for (i=1; i<n; i++) {
		if (A[i] > A[i+1]) {
			return FALSE;
		}
	}
	return TRUE;
}

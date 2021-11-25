#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>

#define COUNTER_VALUE (1UL << 24)
#define T 1000 //threshold count for updating global

static long int globalCounter;

//per individual thread
void* threadFunc(void* arg)
{
    int i = 0, tcount = 0;
    for (i = 0; i < COUNTER_VALUE; i++) {
        tcount++; //each thread has its own counter
        if (tcount > T) {
        //atomically update global counter
            __sync_fetch_and_add(&globalCounter, tcount, __ATOMIC_RELAXED);
            tcount = 0;
        }
    }
    //one more atomic update needed for any remaining
    //thread value, post-loop
    __sync_fetch_and_add(&globalCounter, tcount, __ATOMIC_RELAXED);
    
    return NULL;
}

long int elapsed(struct timeval start, struct timeval end)
{
	return ((end.tv_sec - start.tv_sec) * 1000000 
		+ end.tv_usec - start.tv_usec) / 1000;
}

int main(int argc, char** argv) 
{
	//exe name is argc 1, argv[0], numThreads is argc 2, argv[1]
	if (argc != 2) {
		printf("Please pass only one argument.\n");
		exit(1);
	}

	//argv[1] is non-null at this point
	if (!isdigit(*argv[1])) {
		printf("Please use a numerical argument.\n");
		exit(1);
	}
	
	//ASCII to int, we have a char ptr and we want int
	int numThreads = atoi(argv[1]);
	struct timeval start, end;
	pthread_t tid[numThreads];
	int i;

	gettimeofday(&start, NULL);
	
	// Thread Creation.
	for (i = 0; i < numThreads; i++) {
		pthread_create(&tid[i], NULL, threadFunc, NULL);
	}
	
	// Waiting on Threads to Exit.
	for (i = 0; i < numThreads; i++) {
		pthread_join(tid[i], NULL);
	}
	
	gettimeofday(&end, NULL);
	
	long int expected = COUNTER_VALUE * numThreads;
	
	printf("Counter finish in %ld ms\nThe value of counter should be %ld\nThe value of counter is %ld\n", elapsed(start, end), expected, globalCounter);
	
	return 0;
}

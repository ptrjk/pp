# c2 u1

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#define NUM_THREADS 5

uint32_t err, poc;

void *PrintHello(void *thread_index) { 
	
	printf("Thread %d : Hello World!\n", (uint32_t)thread_index);
	pthread_exit(NULL);
} 

int main (int argc, char *argv[]) { 

	pthread_t threads[NUM_THREADS];

	for(poc = 0; poc < NUM_THREADS; poc++) { 

		printf("Creating thread %d\n", poc); 
		err = pthread_create(&threads[poc], NULL, PrintHello, (void *)poc);
		
		if (err) { 
			
			printf("ERROR from pthread_create() %d\n", err); 
			exit(-1); 
		}
	}
	
	printf("Press enter");
	getchar();

	pthread_exit(NULL);
}


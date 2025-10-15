#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_MAIN_THREADS 7
#define THREADS_PER_PARENT 2
#define INCREMENT_COUNT 5

// Globálne premenné
uint32_t counter = 0;
pthread_mutex_t counter_mutex;
pthread_once_t once_control = PTHREAD_ONCE_INIT;

// Pole pre uloženie ID vlákien vytvorených v Main
pthread_t threads[NUM_MAIN_THREADS];

// Štruktúra pre predanie parametrov do vlákna Trd1
typedef struct {
    uint32_t parent_index;
} thread_arg_t;

// Funkcia init - vykoná sa práve 2x
void init() {
    pthread_t self_id = pthread_self();
    printf("Init function executed by thread ID = %lu\n", (unsigned long)self_id);
    sleep(1); // Simulácia dlhšej inicializácie
}

// Inicializačná funkcia pre vlákna vytvorené v Fun0
void *Trd1(void *arg) {
    thread_arg_t *params = (thread_arg_t *)arg;
    uint32_t parent_index = params->parent_index;
    pthread_t self_id = pthread_self();
    
    // Výpis informácie o vytvorení
    printf("Thread with index = %u created thread with ID = %lu\n", 
           parent_index, (unsigned long)self_id);
    
    // 5x inkrementácia globálnej premennej counter s paralelným prístupom
    for (int i = 0; i < INCREMENT_COUNT; i++) {
        pthread_mutex_lock(&counter_mutex);
        counter++;
        printf("Thread with ID = %lu , counter = %u\n", 
               (unsigned long)self_id, counter);
        pthread_mutex_unlock(&counter_mutex);
        usleep(10000); // Malé oneskorenie pre lepšiu viditeľnosť paralelizmu
    }
    
    free(params);
    pthread_exit(NULL);
}

// Inicializačná funkcia pre vlákna vytvorené v Main
void *Fun0(void *arg) {
    uint32_t index = (uint32_t)(uintptr_t)arg;
    pthread_t self_id = pthread_self();
    
    // Výpis informácie o vytvorení vlákna v Main
    printf("Main thread created thread with index = %u and ID = %lu\n", 
           index, (unsigned long)self_id);
    
    // Vykonanie funkcie init práve 2x (pomocou pthread_once)
    pthread_once(&once_control, init);
    
    // Vytvorenie dvoch ďalších vlákien
    pthread_t child_threads[THREADS_PER_PARENT];
    
    for (int i = 0; i < THREADS_PER_PARENT; i++) {
        thread_arg_t *params = (thread_arg_t *)malloc(sizeof(thread_arg_t));
        params->parent_index = index;
        
        if (pthread_create(&child_threads[i], NULL, Trd1, (void *)params) != 0) {
            fprintf(stderr, "Error creating child thread\n");
            free(params);
            continue;
        }
    }
    
    // Počkanie na ukončenie detských vlákien
    for (int i = 0; i < THREADS_PER_PARENT; i++) {
        pthread_join(child_threads[i], NULL);
    }
    
    // Oznámenie o ukončení vlákna
    printf("Thread with ID = %lu finished.\n", (unsigned long)self_id);
    
    pthread_exit(NULL);
}

int main(void) {
    // Inicializácia mutexu
    pthread_mutex_init(&counter_mutex, NULL);
    
    // Vytvorenie 7 vlákien v hlavnej funkcii
    for (int i = 0; i < NUM_MAIN_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, Fun0, (void *)(uintptr_t)i) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }
    
    // Počkanie na ukončenie všetkých vlákien vytvorených v Main
    for (int i = 0; i < NUM_MAIN_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Výpis finálnej hodnoty countera
    printf("\nFinal counter value: %u\n", counter);
    
    // Zničenie mutexu
    pthread_mutex_destroy(&counter_mutex);
    
    printf("Main thread finished.\n");
    
    return 0;
}

/*
1) Implemente um programa que crie uma cadeia de N threads (além da thread principal). A cadeia deve obedecer à seguinte
sequência: 1a thread cria a 2a thread, 2a thread cria a 3a thread, ..., (N-1)a thread cria a Na thread. Cada thread deve imprimir
seu ID e o ID da thread que a criou. Garanta que a informação exibida na tela ocorrerá na ordem inversa da criação das
threads, ou seja, inicialmente aparecem as informações da na thread, depois da (N-1)a

, ..., depois da 2a e por fim da 1a.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int total_threads;
    pthread_t creator_id;
} thread_args;

// Função que cria a cadeia de threads
// Recursão garante que a cadeia seja impressa na ordem inversa da criação das threads
void *create_thread_chain(void *arg) {
    thread_args *args = (thread_args *)arg;

    int total_threads = args->total_threads;
    pthread_t creator = args->creator_id;

    if (total_threads > 0) {
        pthread_t thread;
        // diminui o total de threads e passa o ID da thread que criou a thread atual
        thread_args next_args = { total_threads - 1, pthread_self() };
        
        // cria uma nova thread que cria a próxima thread a partir dela
        pthread_create(&thread, NULL, create_thread_chain, &next_args);
        pthread_join(thread, NULL); // espera a thread terminar
    }

    // só imprime depois que toda a cadeia já imprimiu,
    // para evitar imprimir a thread principal antes de todas as threads filhas
    // pthread_self() pega o ID da thread atual, creator é o ID da thread que criou a thread atual
    printf("Thread %lu criada pela thread %lu\n", (unsigned long)pthread_self(), (unsigned long)creator);

    return NULL;
}

int main () {
    int total_threads = 0;
    printf("Enter the total number of threads: ");
    scanf("%d", &total_threads);

    // pthread_self() pega o ID da thread principal
    thread_args args = { total_threads, pthread_self() };
    create_thread_chain(&args);

    return 0;
}

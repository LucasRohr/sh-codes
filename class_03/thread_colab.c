/*
2) Implemente um programa que crie três threads, ou seja, três funções. Uma função deve escrever na tela “AAAAA”, a
segunda “BBBBB” e a terceira “CCCCC”. Execute as três threads, garantindo que “AAAAABBBBBCCCCC” seja sempre
exibido na tela, nessa ordem. Para ter certeza de que a ordem impressa é a correta, independentemente da ordem do
escalonamento, inclua um comando sleep (2) na segunda função, ou seja, a que escreve “BBBBB”. Não é permitido usar
MUTEX ou semáforos.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Função que escreve "AAAAA"
void *write_a(void *arg) {
    printf("AAAAA\n");
    return NULL;
}

// Função que escreve "BBBBB" com sleep de 2 segundos para garantir a ordem
void *write_b(void *arg) {
    printf("BBBBB\n");
    sleep(2);
    return NULL;
}

// Função que escreve "CCCCC"
void *write_c(void *arg) {
    printf("CCCCC\n");
    return NULL;
}

int main() {
    pthread_t thread_a, thread_b, thread_c;

    // cria as threads
    pthread_create(&thread_a, NULL, write_a, NULL);
    pthread_create(&thread_b, NULL, write_b, NULL);
    pthread_create(&thread_c, NULL, write_c, NULL);

    // espera as threads terminarem na ordem correta
    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
    pthread_join(thread_c, NULL);

    return 0;
}

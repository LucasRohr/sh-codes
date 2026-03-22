// Comando para compilar: gcc -pthread -o filosofos filosofos.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> // Necessário para as constantes O_CREAT no sem_open
#include <signal.h>

#define N 5
#define LEFT (i+N-1)%N
#define RIGHT (i+1)%N
#define THINKING 0
#define HUNGRY 1
#define EATING 2

int state[N];
int ids[N]; 

sem_t *mutex;
sem_t *sem_fil[N];

// Struct para estatisticas dos filósofos
typedef struct {
    int tried_to_eat; // Tentou comer
    int ate; // Comeu
} Stats;

Stats philosofers_stats[N]; // Array global para armazenar as estatísticas dos filósofos

// Protótipos das funções
void mostrar(void);
void pensar(int);
void pegar_garfo(int);
void por_garfo(int);
void comer(int);
void test(int);
void *acao_filosofo(void *);

// Função para imprimir as estatísticas
void print_stats_and_exit(int sig) {
    printf("\n\n--- ESTATISTICAS FINAIS ---\n");

    for(int i = 0; i < N; i++) {
        printf("Filosofo %d: Tentou comer %d vezes, Comeu %d vezes\n", i + 1,
                philosofers_stats[i].tried_to_eat, philosofers_stats[i].ate);
    }

    printf("------------------------------------------------\n");
    
    // Encerra o processo inteiro (threads inclusas)
    exit(0); 
}

int main() {
    int res, i;
    pthread_t thread[N];
    void *thread_result;
    char sem_name[20]; // Para criar nomes dinâmicos para os semáforos

    // Registra o tratador de sinais no início do main
    // Isso avisa ao SO que o SIGINT (Ctrl+C) deve chamar a função que printa as stats
    signal(SIGINT, print_stats_and_exit);

    // Inicializa os estados e mostra status inicial
    for(i = 0; i < N; i++) {
        state[i] = 0; 
    }
    mostrar();

    // MAC OS FIX: Inicialização com sem_open ao invés de sem_init
    // O sem_unlink garante que apagamos semáforos fantasmas de execuções anteriores
    sem_unlink("/mutex_filosofos");
    mutex = sem_open("/mutex_filosofos", O_CREAT, 0644, 1);
    if (mutex == SEM_FAILED) {
        perror("Erro na inicializacao do mutex!");
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < N; i++) {
        sprintf(sem_name, "/sem_fil_%d", i); // Cria nomes como /sem_fil_0, /sem_fil_1...
        sem_unlink(sem_name);
        sem_fil[i] = sem_open(sem_name, O_CREAT, 0644, 0);
        
        if (sem_fil[i] == SEM_FAILED) {
            perror("Erro na inicializacao do semaforo!");
            exit(EXIT_FAILURE);
        }
    }

    // Cria as threads (filósofos)
    for(i = 0; i < N; i++) {
        ids[i] = i; 
        res = pthread_create(&thread[i], NULL, acao_filosofo, &ids[i]); 
        if (res != 0) {
            perror("Erro na inicializacao da thread!");
            exit(EXIT_FAILURE);
        }
    }

    // Faz um join nas threads
    for(i = 0; i < N; i++) {
        pthread_join(thread[i], &thread_result);
    }

    return 0;
}

// ============================================================================
// As funções de ação sofrem apenas uma pequena mudança: 
// em vez de &mutex, passamos apenas mutex (pois já é um ponteiro)
// ============================================================================

void *acao_filosofo(void *j) {
    int i = *(int*) j; 
    while(1) {
        pensar(i);
        pegar_garfo(i);
        comer(i);
        por_garfo(i);
    }
    return NULL;
}

void pensar(int i) {
    float float_rand = 0.001 * random();
    usleep((int)float_rand);
}

void pegar_garfo(int i) {
    sem_wait(mutex); // Sem o '&' comercial agora
    state[i] = HUNGRY;
    mostrar();
    test(i);
    sem_post(mutex);
    sem_wait(sem_fil[i]);
}

void comer(int i) {
    float float_rand = 0.001 * random();
    usleep((int)float_rand);
}

void test(int i) {
    if (state[i] == HUNGRY) {
        philosofers_stats[i].tried_to_eat++; // Incrementa tentativas de comer
    }

    if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[i] = EATING;

        philosofers_stats[i].ate++; // Incrementa vezes que comeu

        mostrar();
        sem_post(sem_fil[i]);
    }
}

void por_garfo(int i) {
    sem_wait(mutex);
    mostrar();
    test(LEFT);
    test(RIGHT);
    state[i] = THINKING;
    sem_post(mutex);
}

void mostrar() {
    int i;
    for(i = 1; i <= N; i++) {
        if (state[i-1] == THINKING) {
            printf("O filosofo %d esta pensando!\n", i);
        }
        if (state[i-1] == HUNGRY) {
            printf("O filosofo %d esta com fome!\n", i);
        }
        if (state[i-1] == EATING) {
            printf("O filosofo %d esta comendo!\n", i);
        }
    }
    printf("---\n");
}
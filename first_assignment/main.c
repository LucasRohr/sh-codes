#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "conveyor.h"

// Declarações das funções definidas em producer.c e consumer.c
void generate_products(Conveyor *conveyor, int producer_id,
                       sem_t *sem_mutex,
                       sem_t *sem_empty[NUM_QUALITY_LEVELS],
                       sem_t *sem_full[NUM_QUALITY_LEVELS]);

void consume_products(Conveyor *conveyor, int consumer_id,
                      sem_t *sem_mutex,
                      sem_t *sem_empty[NUM_QUALITY_LEVELS],
                      sem_t *sem_full[NUM_QUALITY_LEVELS]);

// Arrays de nomes dos semáforos por qualidade — mesma ordem de Quality (0=BAD, 1=MED, 2=GOOD)
static const char *SEM_EMPTY_NAMES[NUM_QUALITY_LEVELS] = {
    SEM_EMPTY_BAD, SEM_EMPTY_MED, SEM_EMPTY_GOOD
};
static const char *SEM_FULL_NAMES[NUM_QUALITY_LEVELS] = {
    SEM_FULL_BAD, SEM_FULL_MED, SEM_FULL_GOOD
};

int main(void) {
    // Cria e mapeia a memória compartilhada
    // A Conveyor precisa estar em shared memory para que todos os processos filhos leiam e escrevam na mesma região.
    // Todos os processos filhos leem e escrevem na mesma região.

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666); // cria a memória compartilhada

    // Tratamento de erro
    if (shm_fd == -1) {
        perror("Erro ao criar memória compartilhada"); return 1;
    }

    // Trunca a memória compartilhada para o tamanho do Conveyor
    if (ftruncate(shm_fd, sizeof(Conveyor)) == -1) { 
        perror("ftruncate"); return 1;
    }

    // 
    Conveyor *conveyor = mmap(NULL, sizeof(Conveyor),
                              PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Tratamento de erro
    if (conveyor == MAP_FAILED) {
        perror("Erro ao mapear memória compartilhada"); return 1;
    }

    // Inicializa a esteira
    conveyor_init(conveyor);

    // Cria os semáforos nomeados
    // Semáforos nomeados vivem no kernel e são acessíveis por nome em qualquer processo
    sem_t *sem_mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);

    // Tratamento de erro
    if (sem_mutex == SEM_FAILED) {
        perror("Erro ao criar semáforo mutex"); return 1;
    }

    // Cria os semáforos de fila
    sem_t *sem_empty[NUM_QUALITY_LEVELS]; // Semáforos para os slots vazios
    sem_t *sem_full[NUM_QUALITY_LEVELS]; // Semáforos para os slots cheios

    for (int q = 0; q < NUM_QUALITY_LEVELS; q++) {
        // Cria os semáforos de fila para cada qualidade
        sem_empty[q] = sem_open(SEM_EMPTY_NAMES[q], O_CREAT, 0666, QUEUE_CAPACITY);
        sem_full[q]  = sem_open(SEM_FULL_NAMES[q],  O_CREAT, 0666, 0);

        // Tratamento de erro
        if (sem_empty[q] == SEM_FAILED || sem_full[q] == SEM_FAILED) {
            perror("Erro ao criar semáforos de fila");
            return 1;
        }
    }

    // Cria os processos produtores usando fork e salva os PIDs
    pid_t producers_pids[NUM_PRODUCERS];

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producers_pids[i] = fork(); // Cria o processo produtor

        // Tratamento de erro
        if (producers_pids[i] == -1) { perror("Erro no fork do produtor"); return 1; }

        if (producers_pids[i] == 0) {
            // Processo filho: gera uma semente única por PID para sequências aleatórias distintas
            srand((unsigned int)(time(NULL) ^ getpid()));
            generate_products(conveyor, i, sem_mutex, sem_empty, sem_full); // Gera os produtos
            exit(0); // Termina o processo produtor
        }

        printf("[PAI] Produtor %d iniciado (PID %d)\n", i, producers_pids[i]);
    }

    // Cria os processos consumidores usando fork e salva os PIDs
    pid_t consumers_pids[NUM_CONSUMERS];

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumers_pids[i] = fork(); // Cria o processo consumidor

        // Tratamento de erro
        if (consumers_pids[i] == -1) { perror("Erro no fork do consumidor"); return 1; }

        if (consumers_pids[i] == 0) {
            // Processo filho: gera uma semente única por PID para sequências aleatórias distintas
            srand((unsigned int)(time(NULL) ^ getpid()));
            consume_products(conveyor, i, sem_mutex, sem_empty, sem_full); // Consome os produtos
            exit(0); // Termina o processo consumidor
        }

        printf("[PAI] Consumidor %d iniciado (PID %d)\n", i, consumers_pids[i]);
    }

    // Aguarda todos os produtores terminarem
    // Só depois disso sinalizamos simulation_done, garantindo que nenhum produto seja perdido antes de os consumidores encerrarem.
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        waitpid(producers_pids[i], NULL, 0);
        printf("[PAI] Produtor %d finalizado.\n", i);
    }

    // Sinaliza fim da simulação
    sem_wait(sem_mutex); // Entra na seção crítica
    conveyor->simulation_done = 1; // Sinaliza que a simulação terminou
    sem_post(sem_mutex); // Sai da seção crítica

    // Envia um post extra em cada semáforo full para desbloquear
    // consumidores que estejam presos em sem_wait sem item disponível.
    for (int q = 0; q < NUM_QUALITY_LEVELS; q++) {
        sem_post(sem_full[q]);
    }

    // Aguarda todos os consumidores terminarem
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        waitpid(consumers_pids[i], NULL, 0);
        printf("[PAI] Consumidor %d finalizado.\n", i);
    }

    // Imprime as estatísticas finais da simulação
    conveyor_print_stats(conveyor);

    // Cleanup: fecha e remove todos os recursos do sistema
    sem_close(sem_mutex); // Fecha o semáforo mutex
    sem_unlink(SEM_MUTEX_NAME); // Remove o semáforo mutex

    // Para cada semaforo da fila, fecha e remove
    for (int q = 0; q < NUM_QUALITY_LEVELS; q++) {
        sem_close(sem_empty[q]);
        sem_close(sem_full[q]);
        sem_unlink(SEM_EMPTY_NAMES[q]);
        sem_unlink(SEM_FULL_NAMES[q]);
    }

    // Desmapear a memória compartilhada
    munmap(conveyor, sizeof(Conveyor));

    // Remove a memória compartilhada
    shm_unlink(SHM_NAME);

    return 0;
}

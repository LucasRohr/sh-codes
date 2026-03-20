#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>
#include "conveyor.h"

// Nomes dos tipos de produto para exibição no terminal
static const char *PRODUCT_TYPE_NAMES[NUM_PRODUCT_TYPES] = {
    "Eletronico", "Alimenticio", "Fragil"
};

// Nomes dos níveis de qualidade para exibição no terminal
static const char *QUALITY_NAMES[NUM_QUALITY_LEVELS] = {
    "RUIM", "NEUTRO", "BOM"
};


// Loop principal do consumidor
// Cada consumidor é responsável por UMA qualidade fixa, determinada pelo consumer_id (0=RUIM, 1=NEUTRO, 2=BOM).
// Roda até a simulação terminar E a fila da sua qualidade esvaziar.
void consume_products(Conveyor *conveyor, int consumer_id,
                      sem_t *sem_mutex,
                      sem_t *sem_empty[NUM_QUALITY_LEVELS],
                      sem_t *sem_full[NUM_QUALITY_LEVELS]) {

    Quality quality = (Quality)consumer_id; // cada consumidor trata uma qualidade fixa

    while (1) {
        // Bloqueia até o pai sinalizar via sem_post(sem_full[quality])
        // O pai envia um post extra após simulation_done = 1 para desbloquear consumidores que estejam em espera.
        sem_wait(sem_full[quality]);

        // Entra na seção crítica: retira o produto da fila
        sem_wait(sem_mutex);

        Product product;

        // Remove o produto da fila, tratando cenários de erro
        if (conveyor_dequeue(conveyor, quality, &product) == -1) {
            // Fila vazia: o post foi o post extra do pai sinalizando o fim.
            // Se simulation_done estiver setado, encerra. Caso contrário devolve o token e continua pois houve erro
            
            sem_post(sem_mutex); // sai da seção crítica
            if (conveyor->simulation_done) break; // se a simulação terminou, encerra
            sem_post(sem_full[quality]); // devolve o token ao semáforo se houve erro

            continue;
        }

        clock_gettime(CLOCK_REALTIME, &product.timestamp_out); // registra saída da esteira

        // Calcula o tempo de permanência na esteira
        double time_in_conveyor = timespec_diff_ms(product.timestamp_in, product.timestamp_out);

        // Atualiza as estatísticas da esteira
        conveyor->total_consumed++; // incrementa o total de produtos consumidos da esteira
        conveyor->consumed_by_quality[product.quality]++; // incrementa a contagem de produtos consumidos por qualidade
        conveyor->consumed_by_type[product.type]++; // incrementa a contagem de produtos consumidos por tipo
        conveyor->total_time_in_conveyor_ms += time_in_conveyor; // incrementa o tempo total de permanência na esteira

        sem_post(sem_mutex); // sai da seção crítica
        sem_post(sem_empty[quality]); // libera um slot vazio para os produtores

        printf("[CONSUMIDOR %d | %s] Produto #%d %-14s | Tipo: %-12s | Tempo na esteira: %.0f ms\n",
               consumer_id,
               QUALITY_NAMES[quality],
               product.id,
               product.name,
               PRODUCT_TYPE_NAMES[product.type],
               time_in_conveyor);
        fflush(stdout);

        // Simula tempo de processamento do produto entre 0 e 1 segundo (mais rápido que o produtor)
        int sleep_secs = rand() % 2;
        sleep(sleep_secs);
    }

    printf("[CONSUMIDOR %d | %s] Finalizado.\n", consumer_id, QUALITY_NAMES[quality]);
    fflush(stdout);
}

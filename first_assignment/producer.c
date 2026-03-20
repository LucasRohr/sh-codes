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

// Loop principal do produtor:
// Gera produtos aleatórios e os insere na esteira usando os semáforos corretos
void generate_products(Conveyor *conveyor, int producer_id,
                               sem_t *sem_mutex,
                               sem_t *sem_empty[NUM_QUALITY_LEVELS],
                               sem_t *sem_full[NUM_QUALITY_LEVELS]) {
    for (int i = 0; i < GENERATED_PRODUCTS; i++) {
        // Produtor: tempo aleatório entre 2 e 5 segundos (mais lento que o consumidor)
        int sleep_secs = (rand() % 4) + 2;
        sleep(sleep_secs);

        // Monta o produto com dados aleatórios
        Product product;
        product.id = producer_id * GENERATED_PRODUCTS + i; // ID único sem colisão entre produtores
        product.producer_id = producer_id;
        product.quality = (Quality)(rand() % NUM_QUALITY_LEVELS);
        product.type = (ProductType)(rand() % NUM_PRODUCT_TYPES);
        product.production_time_ms = sleep_secs * 1000.0;

        snprintf(product.name, sizeof(product.name), "P%d-Item%03d", producer_id, i);

        // timestamp_out é zerado aqui; será preenchido pelo consumidor
        memset(&product.timestamp_out, 0, sizeof(product.timestamp_out));

        /* Protocolo do produtor:
        1. Espera um slot vazio na fila da qualidade gerada
        2. Entra na seção crítica
        3. Insere o produto e atualiza estatísticas
        4. Sai da seção crítica e sinaliza item disponível */

        sem_wait(sem_empty[product.quality]); // espera um slot vazio na fila da qualidade gerada
        sem_wait(sem_mutex); // entra na seção crítica

        clock_gettime(CLOCK_REALTIME, &product.timestamp_in); // registra entrada na esteira

        conveyor_enqueue(conveyor, product); // insere o produto na esteira

        conveyor->total_produced++; // incrementa o total de produtos inseridos na esteira
        conveyor->produced_by_quality[product.quality]++; // incrementa a contagem de produtos por qualidade
        conveyor->produced_by_type[product.type]++; // incrementa a contagem de produtos por tipo

        sem_post(sem_mutex); // sai da seção crítica
        sem_post(sem_full[product.quality]); // sinaliza item disponível

        printf("[PRODUTOR %d] Produto #%d | %-14s | Tipo: %-12s | Qualidade: %s | Tempo de producao: %.0f ms\n",
               producer_id, product.id,
               product.name,
               PRODUCT_TYPE_NAMES[product.type],
               QUALITY_NAMES[product.quality],
               product.production_time_ms);
        fflush(stdout);
    }
}


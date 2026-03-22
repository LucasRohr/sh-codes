#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

float temperatures[10]; // Variável compartilhada para armazenar as temperaturas
int readings_count = 0; // Contador de leituras

float average_temperature = 0.0; // Média calculada com variável compartilhada
int new_average_available = 0; // Flag para avisar nova média disponível

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread para capturar a temperatura
void* thread_c(void* args) {
    while(1) {
        // Temperatura aleatória entre 0 e 40
        float random_temperature = (rand() % 41);

        // Lock para acessar a variável compartilhada
        pthread_mutex_lock(&mutex);

        // Mostra temperatura
        printf("Temperatura capturada: %.2f\n", random_temperature);

        // Armazenar a temperatura lida
        temperatures[readings_count] = random_temperature;
        readings_count++; // Incrementa contador

        // Unlock para sair da seção crítica
        pthread_mutex_unlock(&mutex);

        sleep(1); // Espera 1 segundo
    }
}

void* thread_p(void* arg) {
    // Recebe o parâmetro de tempo (ex: 10 segundos) passado na criação da thread
    int processing_time = *(int*)arg;

    while(1) {
        sleep(processing_time); // Aguarda o tempo parametrizado

        // Lock para acessar a variável compartilhada
        pthread_mutex_lock(&mutex);

        if (readings_count > 0) {
            for (int i = 0; i < readings_count; i++) {
                average_temperature += temperatures[i]; // Soma todas temperaturas
            }

            average_temperature /= readings_count; // Divide por total para calcular a média
            new_average_available = 1; // Sinaliza nova média disponível

            readings_count = 0; // Reseta contador
        }

        // Unlock para sair da seção crítica
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void* thread_m(void* arg) {
    // Recebe o parâmetro do limite de irrigação
    float threshold = *(float*)arg;

    while(1) {
        // Lock para acessar a variável compartilhada
        pthread_mutex_lock(&mutex);

        if (new_average_available) {
            if (average_temperature < threshold) {
                printf("Média de temperatura %.2f abaixo do limite %.2f. Irrigação ativada.\n", average_temperature, threshold);
            } else {
                printf("Média de temperatura %.2f acima do limite %.2f. Nada aconteceu.\n", average_temperature, threshold);
            }

            new_average_available = 0; // Reseta flag após usar a média
        }

        // Unlock para sair da seção crítica
        pthread_mutex_unlock(&mutex);

        sleep(1); // Pausa para evitar busy waiting elevado
    }

    return NULL;
}

int main() {
    pthread_t threads[3];

    int param_processing_time = 10; // Tempo para a thread de processamento
    float param_thresold = 30.0; // Limite para a thread M

    pthread_create(&threads[0], NULL, thread_c, NULL);
    pthread_create(&threads[1], NULL, thread_p, &param_processing_time);
    pthread_create(&threads[2], NULL, thread_m, &param_thresold);

    // Aguarda todas as threads terminarem
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destruir o mutex para liberar recursos
    pthread_mutex_destroy(&mutex);

    return 0;
}
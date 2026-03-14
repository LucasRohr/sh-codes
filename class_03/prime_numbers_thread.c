/*
3) A seguir, apresenta-se a versão serial de um programa para calcular todos os números primos até um determinado valor.
Implemente uma versão multithread para esse programa. Avalie o desempenho do seu programa com valores de entrada
grandes, como por exemplo: 10.000.000 e 100.000.000. A comparação deve ser realizada com o programa sequência e com
uma versão que utiliza processos pesados, i.e., fork. Ao comparar, não imprima o resultado na tela, pois a concorrência na
realização de E/S tornará o programa mais lento e afetará a avaliação. Sugestão: use o comando time, para coletar os tempos
de execução.
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    long int inicio; // inicio do intervalo de números a serem verificados
    long int fim; // fim do intervalo de números a serem verificados
    long int total_primos; // resultado parcial da thread
    short int imprimir; // flag para imprimir os primos
} thread_args;

int verifica_se_primo(long int numero) {
    if (numero <= 1) return 0; // 0 e 1 não são primos
    if (numero == 2) return 1; // 2 é o único número primo par
    if (numero % 2 == 0) return 0; // 2 é o único número primo par

    for (long int ant = 3; ant <= (long int)sqrt(numero); ant += 2) {
        if (numero % ant == 0)
            return 0;
    }

    return 1;
}

// Função que conta os números primos entre inicio e fim
// A estratégia é dividir o intervalo em partes e contar os números primos em cada parte
// Cada thread conta os números primos em um intervalo específico para otimizar o desempenho
void *conta_primos(void *arg) {
    thread_args *args = (thread_args *)arg;
    args->total_primos = 0;

    for (long int n = args->inicio; n <= args->fim; n++) {
        if (verifica_se_primo(n)) {
            args->total_primos++; // Incrementa o total de primos
            if (args->imprimir == 1) // Se a flag para imprimir os primos estiver ativa, imprime o número primo
                printf("%ld eh primo.\n", n);
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso correto: %s <numero> <imprimir=1,nao_imprimir=0> <total_threads>\n\n", argv[0]);
        return 1;
    }

    long int numero = atol(argv[1]);
    short int imprimir = atoi(argv[2]);
    int num_threads = atoi(argv[3]); // Recebe o total de threads

    long int total_primos = 0;

    thread_args args[num_threads]; // Array de argumentos para as threads

    pthread_t threads[num_threads]; // Array de threads
    long int faixa = numero / num_threads; // Calcula o intervalo de cada thread

    for (int i = 0; i < num_threads; i++) {
        args[i].inicio = i * faixa + (i == 0 ? 2 : 1); // Calcula o inicio de cada thread
        args[i].fim = (i == num_threads - 1) ? numero : (i + 1) * faixa; // Calcula o fim de cada thread
        args[i].imprimir = imprimir; // Passa a flag para imprimir os primos de forma opcional
        pthread_create(&threads[i], NULL, conta_primos, &args[i]); // Cria a thread para contar os primos
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL); // Espera a thread terminar para cada uma delas
        total_primos += args[i].total_primos; // Soma o total de primos de cada thread
    }

    printf("Total de primos ate %ld: %ld\n", numero, total_primos);

    return 0;
}

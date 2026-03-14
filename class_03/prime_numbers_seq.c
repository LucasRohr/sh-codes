/* primos.c – identifica todos os numeros primos até um certo valor*/
/* complile com -lm. Por exemplo: “gcc primo.c -o primo -lm”*/

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

int verifica_se_primo(long int numero) {
    if (numero <= 1) return 0;
    if (numero == 2) return 1;
    if (numero % 2 == 0) return 0;
    for (long int ant = 3; ant <= (long int)sqrt(numero); ant += 2) {
        if (numero % ant == 0)
            return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso correto: %s <numero> <imprimir=1,nao_imprimir=0>\n\n", argv[0]);
        return 1;
    }

    long int numero = atol(argv[1]);
    short int imprimir = atoi(argv[2]);
    long int total_primos = 0;

    for (long int num_int = 2; num_int <= numero; num_int++) {
        if (verifica_se_primo(num_int)) {
            total_primos++;
            if (imprimir == 1)
                printf("%ld eh primo.\n", num_int);
        }
    }

    printf("Total de primos ate %ld: %ld\n", numero, total_primos);
    return 0;
}
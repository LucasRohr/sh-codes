#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

// 2) Implemente um programa que cria dois processos filhos. O primeiro grava em um arquivo os números de 1 a 10. O
// segundo grava em um arquivo letras de A a Z. O processo pai, após cada filho terminar sua execução, lista o conteúdo dos
// arquivos criados na tela.

void write_numbers_to_file() {
    FILE *file = fopen("numbers.txt", "w"); // abre o arquivo para escrita

    for (int i = 1; i <= 10; i++) {
        fprintf(file, "%d\n", i); // escreve o número no arquivo
    }

    fclose(file); // fecha o arquivo
}

void write_letters_to_file() {
    FILE *file = fopen("letters.txt", "w"); // abre o arquivo para escrita

    for (char c = 'A'; c <= 'Z'; c++) {
        fprintf(file, "%c\n", c); // escreve a letra no arquivo
    }

    fclose(file);
}

void list_files() {
    FILE *file_numbers = fopen("numbers.txt", "r"); // abre o arquivo para leitura
    FILE *file_letters = fopen("letters.txt", "r"); // abre o arquivo para leitura


    int number;
    char letter;

    printf("Numbers:\n");

    while (fscanf(file_numbers, "%d", &number) != EOF) { // lê o número do arquivo
        printf("%d\n", number); // imprime o número
    }

    printf("Letters:\n");

    while (fscanf(file_letters, "%c", &letter) != EOF) { // lê a letra do arquivo
        printf("%c", letter); // imprime a letra
    }

    fclose(file_numbers); // fecha o arquivo
    fclose(file_letters); // fecha o arquivo
}

int main () {
    pid_t pid_numbers = fork(); // cria um novo processo filho para escrever os números
    if (pid_numbers == 0) { // se o processo filho é o filho
        write_numbers_to_file(); // escreve os números no arquivo
        exit(0);
    }

    pid_t pid_letters = fork(); // cria um novo processo filho para escrever as letras
    if (pid_letters == 0) { // se o processo filho é o filho
        write_letters_to_file(); // escreve as letras no arquivo
        exit(0);
    }

    wait(NULL); // espera o filho terminar
    wait(NULL); // espera o filho terminar

    list_files(); // lista os arquivos no processo pai (main)

    return 0;
}

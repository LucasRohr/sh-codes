// 4) Instale o aplicativo figlet, que recebe um texto e o exibe em uma forma estilosa no terminal. Implemente um programa
// que, ao receber o sinal (por exemplo, SIGUSR2), execute uma função que chama o aplicativo figlet em um processo filho. O
// texto deve ser digitado pelo usuário antes da chamada.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void handle_signal(int sig) {
    printf("Recebendo sinal %d\n", sig);

    char text[100];
    printf("Escreva o texto para exibir: ");
    fgets(text, sizeof(text), stdin); // lê o texto do usuário, aceitando espaços
    text[strcspn(text, "\n")] = '\0'; // remove o \n que o fgets inclui no final

    pid_t pid = fork();

    // processo filho para exibir o texto com figlet
    if (pid == 0) {
        printf("Exibindo texto com figlet\n");
        fflush(stdout);

        char command[120];
        snprintf(command, sizeof(command), "figlet '%s'", text); // formata o comando figlet para exibir o texto
        execlp("sh", "sh", "-c", command, NULL); // executa o comando figlet para exibir o texto

        exit(0); // termina o processo filho
    }
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); // inicializa o struct sigaction com 0
    sa.sa_handler = &handle_signal; // define a função de tratamento de sinal

    printf("PID do processo pai: %d\n", getpid());

    sigaction(SIGUSR2, &sa, NULL); // define o sinal SIGUSR2 para o tratamento de sinal

    printf("Aguardando sinal SIGUSR2...\n");

    pause(); // suspende o processo até receber qualquer sinal
    wait(NULL); // espera o processo filho terminar

    return 0;
}
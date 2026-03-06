#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

// Exercicio 1: Crie uma cadeia de processos filhos onde cada processo filho cria um novo processo filho,
// até que o total de processos seja atingido e imprima o PID e o PPID de cada processo de forma inversa.
void create_process_chain(int total_processes) {
    if (total_processes > 0) {
        pid_t pid = fork(); // cria um novo processo filho

        if (pid == 0) { // se o processo filho é o filho
            create_process_chain(total_processes - 1); // cria um novo processo filho a partir do filho
            exit(0); // termina o processo filho
        }

        wait(NULL); // espera o filho (e toda a cadeia abaixo) terminar
    }

    // só imprime depois que toda a cadeia já imprimiu,
    // para evitar imprimir o PID e o PPID do processo pai antes de todos os filhos
    printf("PID = %d, PPID = %d\n", getpid(), getppid());
}

int main () {
    int total_processes = 0;
    printf("Enter the total number of processes: ");
    scanf("%d", &total_processes);
    create_process_chain(total_processes);

    return 0;
}
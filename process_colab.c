// 3) Implemente dois processos que realizam uma tarefa, colaborativamente. Um processo deve baixar páginas na Internet,
// por exemplo, usando o programa curl. O segundo processo recebe essas páginas e executa um parser para contar o número
// de uma determinada palavra, por exemplo, usando os programas egrep e wc. O segundo programa devolve o número de
// ocorrências do primeiro processo.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    // lê a palavra antes do fork para não bloquear o pipe durante o download
    char word[100];
    printf("Escreva a palavra para contar: ");
    scanf("%s", word);

    int pipe_fd[2];
    pipe(pipe_fd); // cria o pipe: pipe_fd[0] = leitura, pipe_fd[1] = escrita

    // cria um novo processo filho para baixar a página de forma colaborativa
    pid_t pid_download = fork();

    // processo filho para baixar a página
    if (pid_download == 0) {
        close(pipe_fd[0]); // fecha a leitura do pipe
        dup2(pipe_fd[1], STDOUT_FILENO); // redireciona stdout para o pipe para enviar a página baixada
        close(pipe_fd[1]); // fecha a escrita do pipe

        // executa o comando curl para baixar a página de forma silenciosa
        execlp("curl", "curl", "-s", "https://g1.globo.com", NULL);

        exit(0); // termina o processo filho
    } else {
        // processo pai para fazer o parsing da página e contar o número de ocorrências da palavra

        close(pipe_fd[1]); // fecha a escrita do pipe
        dup2(pipe_fd[0], STDIN_FILENO); // redireciona stdin para ler do pipe
        close(pipe_fd[0]); // fecha a leitura do pipe

        printf("Contagem de ocorrências da palavra '%s': ", word);
        fflush(stdout);

        // executa egrep | wc -l via shell para contar ocorrências da palavra na página
        // tr -d ' ' remove os espaços da saída do wc -l
        char command[200];
        snprintf(command, sizeof(command), "egrep -o '%s' | wc -l | tr -d ' '", word);
        execlp("sh", "sh", "-c", command, NULL);
    }

    wait(NULL); // espera o processo filho (curl) terminar

    return 0;
}
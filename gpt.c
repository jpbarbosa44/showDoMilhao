#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_PERGUNTA 256
#define MAX_ALTERNATIVA 10
int fimDeJogo = 0;

typedef struct {
    char pergunta[MAX_PERGUNTA];
    char alternativaA[MAX_ALTERNATIVA];
    char alternativaB[MAX_ALTERNATIVA];
    char alternativaC[MAX_ALTERNATIVA];
    char alternativaD[MAX_ALTERNATIVA];
    char respostaCorreta;
} Pergunta;

void lerPerguntas(Pergunta perguntas[], int *quantidade) {
    FILE *arquivo = fopen("questoes.txt", "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de perguntas");
        exit(1);
    }

    int i = 0;
    while (fscanf(arquivo, " %255[^|]|%99[^|]|%99[^|]|%99[^|]|%99[^|]|%c\n",
                  perguntas[i].pergunta, perguntas[i].alternativaA,
                  perguntas[i].alternativaB, perguntas[i].alternativaC,
                  perguntas[i].alternativaD, &perguntas[i].respostaCorreta) == 6) {
        i++;
    }
    *quantidade = i;
    fclose(arquivo);
}

int main() {
    int pipePaiFilho1[2], pipeFilho1Pai[2];
    int pipePaiFilho2[2], pipeFilho2Pai[2];
    int pontosFilho1 = 0, pontosFilho2 = 0;
    Pergunta perguntas[10];
    int totalPerguntas;

    // Lê as perguntas do arquivo
    lerPerguntas(perguntas, &totalPerguntas);

    // Cria os pipes
    pipe(pipePaiFilho1);
    pipe(pipeFilho1Pai);
    pipe(pipePaiFilho2);
    pipe(pipeFilho2Pai);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Processo Filho 1
        close(pipePaiFilho1[1]); // Fecha o lado de escrita do pai->filho
        close(pipeFilho1Pai[0]); // Fecha o lado de leitura do filho->pai

        char buffer[MAX_PERGUNTA + 4 * MAX_ALTERNATIVA + 10];
        while (read(pipePaiFilho1[0], buffer, sizeof(buffer)) > 0) {
            printf("Filho 1 recebeu a pergunta: %s\n", buffer);

            // Simula uma resposta (alternativa 'a', 'b', 'c' ou 'd')
            char resposta;
           
            printf("\nFilho 1 responde: ");
            scanf("%c", &resposta);

            write(pipeFilho1Pai[1], &resposta, sizeof(resposta));
        }

        close(pipePaiFilho1[0]);
        close(pipeFilho1Pai[1]);
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Processo Filho 2
        close(pipePaiFilho2[1]); // Fecha o lado de escrita do pai->filho
        close(pipeFilho2Pai[0]); // Fecha o lado de leitura do filho->pai

        char buffer[MAX_PERGUNTA + 4 * MAX_ALTERNATIVA + 10];
        while (read(pipePaiFilho2[0], buffer, sizeof(buffer)) > 0) {
            printf("Filho 2 recebeu a pergunta: %s\n", buffer);

            // Simula uma resposta (alternativa 'a', 'b', 'c' ou 'd')
            char resposta = 'b'; // Simulação
            printf("Filho 2 responde: %c\n", resposta);

            write(pipeFilho2Pai[1], &resposta, sizeof(resposta));
        }

        close(pipePaiFilho2[0]);
        close(pipeFilho2Pai[1]);
        exit(0);
    }

    // Processo Pai
    close(pipePaiFilho1[0]);
    close(pipeFilho1Pai[1]);
    close(pipePaiFilho2[0]);
    close(pipeFilho2Pai[1]);

    for (int i = 0; i < totalPerguntas; i++) {
          fimDeJogo++;
        char buffer[MAX_PERGUNTA + 4 * MAX_ALTERNATIVA + 10];
        snprintf(buffer, sizeof(buffer), "%s|%s|%s|%s|%s",
                 perguntas[i].pergunta, perguntas[i].alternativaA,
                 perguntas[i].alternativaB, perguntas[i].alternativaC,
                 perguntas[i].alternativaD);

        if (i % 2 == 0) { // Vez do Filho 1
            write(pipePaiFilho1[1], buffer, strlen(buffer) + 1);
            char resposta;
            read(pipeFilho1Pai[0], &resposta, sizeof(resposta));

            if (resposta == perguntas[i].respostaCorreta) {
                printf("Filho 1 acertou!\n");
                pontosFilho1++;
            } else {
                printf("Filho 1 errou. Resposta correta: %c\n", perguntas[i].respostaCorreta);
            }
        } else { // Vez do Filho 2
            write(pipePaiFilho2[1], buffer, strlen(buffer) + 1);
            char resposta;
            read(pipeFilho2Pai[0], &resposta, sizeof(resposta));

            if (resposta == perguntas[i].respostaCorreta) {
                printf("Filho 2 acertou!\n");
                pontosFilho2++;
            } else {
                printf("Filho 2 errou. Resposta correta: %c\n", perguntas[i].respostaCorreta);
            }
        }

    }

    // Final do jogo
  
    printf("Pontuação final:\n");
    printf("Filho 1: %d pontos\n", pontosFilho1);
    printf("Filho 2: %d pontos\n", pontosFilho2);

    close(pipePaiFilho1[1]);
    close(pipeFilho1Pai[0]);
    close(pipePaiFilho2[1]);
    close(pipeFilho2Pai[0]);
    
    

    wait(NULL);
    wait(NULL);

    return 0;
}

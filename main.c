#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_PERGUNTA 256
#define MAX_ALTERNATIVA 100
int pergunta = 0;

typedef struct {
    char pergunta[MAX_PERGUNTA];
    char alternativaA[MAX_ALTERNATIVA];
    char alternativaB[MAX_ALTERNATIVA];
    char alternativaC[MAX_ALTERNATIVA];
    char alternativaD[MAX_ALTERNATIVA];
    char resposta;
} Questao;

Questao perguntas[10];
int totalDePerguntas = 0;

void imprimeQuestao(Questao questao) {
    printf("Pergunta: %s\n", questao.pergunta);
    printf("Alternativa A: %s\n", questao.alternativaA);
    printf("Alternativa B: %s\n", questao.alternativaB);
    printf("Alternativa C: %s\n", questao.alternativaC);
    printf("Alternativa D: %s\n", questao.alternativaD);
}


void lerPerguntas(Questao perguntas[], int *totalDePerguntas) {
    FILE *arquivo = fopen("questoes.txt", "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de perguntas");
        exit(1);
    }

    int i = 0;
    while (fscanf(arquivo, " %255[^|]|%99[^|]|%99[^|]|%99[^|]|%99[^|]|%c\n",
                  perguntas[i].pergunta, perguntas[i].alternativaA,
                  perguntas[i].alternativaB, perguntas[i].alternativaC,
                  perguntas[i].alternativaD, &perguntas[i].resposta) == 6) {
        i++;   
    }
    *totalDePerguntas = i;
    
    fclose(arquivo);
}

void jogador1(int readfd, int writefd) {

    char buffer[MAX_PERGUNTA + 4 * MAX_ALTERNATIVA + 100];
    while(read(readfd, buffer, sizeof(buffer)) > 0){
        printf("\nPergunta ao jogador 1: \n");
        printf(buffer);
        printf("Responda -> | a | b | c | d |\n");
        char resposta;
        scanf("%c", &resposta);
        getchar();
        fflush(stdin);
        pergunta++;
        write(writefd, &resposta, 1);
    }
}

void jogador2(int readfd, int writefd) {

    char buffer[MAX_PERGUNTA + 4 * MAX_ALTERNATIVA + 100];
    while(read(readfd, buffer, sizeof(buffer)) > 0){
        printf("\nPergunta ao jogador 2: \n");
        printf(buffer);
        printf("Responda -> | a | b | c | d |\n");
        char resposta;
        scanf("%c", &resposta);
        getchar();
        fflush(stdin);
        pergunta++;
        write(writefd, &resposta, 1);
    }
     
}

int main(){
    int pipeApresentadorJogador1[2], pipeJogador1Apresentador[2];
    int pipeApresentadorJogador2[2], pipeJogador2Apresentador[2];
    int pontosJogador1 = 0, pontosJogador2 = 0;
    lerPerguntas(perguntas, &totalDePerguntas);

    pipe(pipeApresentadorJogador1);
    pipe(pipeJogador1Apresentador);
    pipe(pipeApresentadorJogador2);
    pipe(pipeJogador2Apresentador);

    pid_t pidJogador1 = fork();
    if (pidJogador1 == 0){
        close(pipeJogador1Apresentador[0]); //Fecha a leitura do pipe jogador 1 -> apresentador
        close(pipeApresentadorJogador1[1]); //Fecha a escrita do pipe apresentador -> jogador 1
        jogador1(pipeApresentadorJogador1[0], pipeJogador1Apresentador[1]);
    }

     pid_t pidJogador2 = fork();
    if (pidJogador2 == 0){
        close(pipeJogador2Apresentador[0]); //Fecha a leitura do pipe jogador 1 -> apresentador
        close(pipeApresentadorJogador2[1]); //Fecha a escrita do pipe apresentador -> jogador 1
        jogador2(pipeApresentadorJogador2[0], pipeJogador2Apresentador[1]);
    }
    
    for (int i = 0; i < totalDePerguntas; i++) {
        char buffer[MAX_PERGUNTA + 4 * MAX_ALTERNATIVA + 10];
        snprintf(buffer, sizeof(buffer), "%s\na)%s\nb)%s\nc)%s\nd)%s\n",
                 perguntas[i].pergunta, perguntas[i].alternativaA,
                 perguntas[i].alternativaB, perguntas[i].alternativaC,
                 perguntas[i].alternativaD);

        if (i % 2 == 0) { // Vez do Filho 1
            write(pipeApresentadorJogador1[1], buffer, strlen(buffer) + 1);
            char resposta;
            read(pipeJogador1Apresentador[0], &resposta, sizeof(resposta));

            if (resposta == perguntas[i].resposta) {
                printf("Filho 1 acertou!\n");
                pontosJogador1+=10;
            } else {
                printf("Filho 1 errou. Resposta correta: %c\n", perguntas[i].resposta);
            }
        } else{ //Vez filho 2
            write(pipeApresentadorJogador2[1], buffer, strlen(buffer) + 1);
            char resposta;
            read(pipeJogador2Apresentador[0], &resposta, sizeof(resposta));

            if (resposta == perguntas[i].resposta) {
                printf("Filho 2 acertou!\n");
                pontosJogador2+=10;
            } else {
                printf("Filho 2 errou. Resposta correta: %c\n", perguntas[i].resposta);
            }
        }
    }

    printf("Pontuacao 1: %d\n", pontosJogador1);
    printf("Pontuacao 2: %d\n", pontosJogador2);
    printf("\n\nFIM");
    return 0;
}
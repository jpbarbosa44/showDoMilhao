#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#define MAX_PERGUNTA 256
#define MAX_ALTERNATIVA 100
#define TEMPO_MAXIMO_RESPONDER 25 // Tempo em segundos para resposta

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
pthread_mutex_t mutex;
pthread_cond_t cond;
bool perguntaDoMilhao = false;
int indicePerguntaDoMilhao = 0;
int pontosJogador1 = 0, pontosJogador2 = 0;

bool validaAlternativa(char alternativa) {
    return alternativa == 'a' || alternativa == 'b' || alternativa == 'c' || alternativa == 'd';
}

void exibePlacar() {
    printf("\nJogador 1: %d pontos\n", pontosJogador1);
    printf("Jogador 2: %d pontos\n", pontosJogador2);
    fflush(stdout);
}

void *geraPerguntaDoMilhao() {
    if (!perguntaDoMilhao) {
        srand(time(NULL));

        int min = 8, max = 9;
        pthread_mutex_lock(&mutex);
        indicePerguntaDoMilhao = rand() % (max - min + 1) + min;
        pthread_mutex_unlock(&mutex);
        perguntaDoMilhao = true;
    }
}

// Função para controlar o tempo de resposta
void* controleDeTempo(void* jogador) {
    int tempoRestante = TEMPO_MAXIMO_RESPONDER;
    
    while (tempoRestante > 0) {
        sleep(1); // Aguarda um segundo
         if(tempoRestante == 20){
            printf("Tempo restante: %d\n", tempoRestante);
         }
         if(tempoRestante == 10){
            printf("Tempo restante: %d\n", tempoRestante);
         }
         if(tempoRestante == 5){
            printf("Tempo restante: %d\n", tempoRestante);
         }
         tempoRestante--;
    }
    printf("Tempo esgotado!\n");
   
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
    while (read(readfd, buffer, sizeof(buffer)) > 0) {

        pthread_t threadTempo;
        pthread_create(&threadTempo, NULL, controleDeTempo, NULL); // Thread para o tempo

       
        printf("\nPergunta ao jogador 1: \n");
        printf(buffer);
        printf("Responda -> | a | b | c | d |\n");
        char resposta;
        do{
            scanf("%c", &resposta);
            getchar();
            
            if(validaAlternativa(resposta)) 
                break;
            printf("Resposta invalida\n");
        }while(true);  
        
       
        write(writefd, &resposta, 1);
        pthread_cancel(threadTempo); // Cancela a thread de tempo se a resposta for dada
    }
}

void jogador2(int readfd, int writefd) {
    char buffer[MAX_PERGUNTA + 4 * MAX_ALTERNATIVA + 100];
    while (read(readfd, buffer, sizeof(buffer)) > 0) {
        pthread_t threadTempo;
        pthread_create(&threadTempo, NULL, controleDeTempo, NULL); // Thread para o tempo
        
        printf("\nPergunta ao jogador 2: \n");
        printf(buffer);
        printf("Responda -> | a | b | c | d |\n");

        char resposta;
        do{
            scanf("%c", &resposta);
            getchar();
            
            if(validaAlternativa(resposta)) 
                break;
            printf("Resposta invalida\n");
        }while(true);
        write(writefd, &resposta, 1);
        pthread_cancel(threadTempo); // Cancela a thread de tempo se a resposta for dada
    }
}

void printBoca() {
    printf(" ____    ____    _____             _____    ____    __  __  _____  _       _    _             ____  \n");
    printf("|  _ \\  / __ \\  / ____|    /\\     |  __ \\  / __ \\  |  \\/  ||_   _|| |     | |  | |    /\\     / __ \\ \n");
    printf("| |_) || |  | || |        /  \\    | |  | || |  | | | \\  / |  | |  | |     | |__| |   /  \\   | |  | |\n");
    printf("|  _ < | |  | || |       / /\\ \\   | |  | || |  | | | |\\/| |  | |  | |     |  __  |  / /\\ \\  | |  | |\n");
    printf("| |_) || |__| || |____  / ____ \\  | |__| || |__| | | |  | | _| |_ | |____ | |  | | / ____ \\ | |__| |\n");
    printf("|____/  \\____/  \\_____|/_/    \\_\\ |_____/  \\____/  |_|  |_||_____||______||_|  |_|/_/    \\_\\ \\____/ \n");
    printf("                                                                                                    \n");
    printf("                                                                                                    \n");
}

void printObrigado() {
    printf("  ____   ____   _____   _____   _____            _____    ____  \n");
    printf(" / __ \\ |  _ \\ |  __ \\ |_   _| / ____|    /\\    |  __ \\  / __ \\ \n");
    printf("| |  | || |_) || |__) |  | |  | |  __    /  \\   | |  | || |  | | \n");
    printf("| |  | ||  _ < |  _  /   | |  | | |_ |  / /\\ \\  | |  | || |  | | \n");
    printf("| |__| || |_) || | \\ \\  _| |_ | |__| | / ____ \\ | |__| || |__| | \n");
    printf(" \\____/ |____/ |_|  \\_\\|_____| \\_____|/_/    \\_\\|_____/  \\____/ \n");
}

void boasVindas(){
    printBoca();
    printf("******************************************************\n");
    printf("Bem vindo ao jogo do Boca do Milhao!\n");
    printf("******************************************************\n\n");
    printf("Responda as perguntas corretamente e descubra quem sera o ganhador do milhao!\n\n");

    printf("Aperte qualquer tecla para comecar!\n");
    getchar();
}

int main() {
    int pipeApresentadorJogador1[2], pipeJogador1Apresentador[2];
    int pipeApresentadorJogador2[2], pipeJogador2Apresentador[2];
    lerPerguntas(perguntas, &totalDePerguntas);
    pthread_t threadIndicePerguntaDoMilhao;
    pthread_mutex_init(&mutex, NULL);

    if (pipe(pipeApresentadorJogador1) < 0 || pipe(pipeJogador1Apresentador) < 0 ||
        pipe(pipeApresentadorJogador2) < 0 || pipe(pipeJogador2Apresentador) < 0) {
        printf("\nErro ao iniciar PIPES!");
        exit(0);
    }

    if (pthread_create(&threadIndicePerguntaDoMilhao, NULL, geraPerguntaDoMilhao, NULL) != 0) {
        perror("Erro ao criar a thread");
        exit(1);
    }

    boasVindas();

    pid_t pidJogador1 = fork();
    if (pidJogador1 < 0) {
        printf("\nErro ao iniciar o processo Jogador1");
        exit(0);
    }
    if (pidJogador1 == 0) {
        close(pipeJogador1Apresentador[0]); // Fecha a leitura do pipe jogador 1 -> apresentador
        close(pipeApresentadorJogador1[1]); // Fecha a escrita do pipe apresentador -> jogador 1
        jogador1(pipeApresentadorJogador1[0], pipeJogador1Apresentador[1]);
    }

    pid_t pidJogador2 = fork();
    if (pidJogador2 < 0) {
        printf("\nErro ao iniciar o processo Jogador2");
        exit(0);
    }
    if (pidJogador2 == 0) {
        close(pipeJogador2Apresentador[0]); // Fecha a leitura do pipe jogador 2 -> apresentador
        close(pipeApresentadorJogador2[1]); // Fecha a escrita do pipe apresentador -> jogador 2
        jogador2(pipeApresentadorJogador2[0], pipeJogador2Apresentador[1]);
    }

    for (int i = 0; i < totalDePerguntas; i++) {
        char buffer[MAX_PERGUNTA + 4 * MAX_ALTERNATIVA + 10];
        snprintf(buffer, sizeof(buffer), "%s\na)%s\nb)%s\nc)%s\nd)%s\n",
                 perguntas[i].pergunta, perguntas[i].alternativaA,
                 perguntas[i].alternativaB, perguntas[i].alternativaC,
                 perguntas[i].alternativaD);

        if (i % 2 == 0) { // Vez do Jogador 1
            write(pipeApresentadorJogador1[1], buffer, strlen(buffer) + 1);
            char resposta;
            read(pipeJogador1Apresentador[0], &resposta, sizeof(resposta));

            if (resposta == perguntas[i].resposta) {
                if (i == indicePerguntaDoMilhao && i != 0) {
                    printf("Jogador 1 ganhou o Milhao!\n");
                    break;
                }
                printf("Jogador 1 acertou!\n");
                pontosJogador1 += 10;
            } else {
                printf("Jogador 1 errou. Resposta correta: %c\n", perguntas[i].resposta);
            }
        } else { // Vez Jogador 2
            write(pipeApresentadorJogador2[1], buffer, strlen(buffer) + 1);
            char resposta;
            read(pipeJogador2Apresentador[0], &resposta, sizeof(resposta));
            if (resposta == perguntas[i].resposta) {
                if (i == indicePerguntaDoMilhao && i != 0) {
                    printf("Jogador 2 ganhou o Milhao!\n");
                    break;
                }
                printf("Jogador 2 acertou!\n");
                pontosJogador2 += 10;
            } else {
                printf("Jogador 2 errou. Resposta correta: %c\n", perguntas[i].resposta);
            }
        }

        exibePlacar();
    }

    printf("******Pontuação final******\n");
    exibePlacar();
    if (pontosJogador1 > pontosJogador2) {
        printf("Jogador 1 ganhou!\n");
    }else if (pontosJogador1 < pontosJogador2){
        printf("Jogador 2 ganhou!\n");
    }else{
        printf("Empate!\n");
    }

    close(pipeApresentadorJogador1[1]);
    close(pipeApresentadorJogador1[0]);
    close(pipeApresentadorJogador2[1]);
    close(pipeJogador2Apresentador[0]);
    printObrigado();
    printf("Desenvolvido por: Gabriel H Coetti & Joao Pedro Barbosa");
    wait(NULL);
    wait(NULL);
    
    return 0;
}

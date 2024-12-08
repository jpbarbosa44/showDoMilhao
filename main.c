#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#define MAX_PERGUNTA 256 // Tamanho máximo da pergunta
#define MAX_ALTERNATIVA 100 // Tamanho máximo da alternativa
#define TEMPO_MAXIMO_RESPONDER 30 // Tempo em segundos para resposta

// ******************************************************
// * Estrutura que representa uma questão com pergunta, *
// * alternativas e a resposta correta.                 *
// ******************************************************
typedef struct {
    char pergunta[MAX_PERGUNTA];
    char alternativaA[MAX_ALTERNATIVA];
    char alternativaB[MAX_ALTERNATIVA];
    char alternativaC[MAX_ALTERNATIVA];
    char alternativaD[MAX_ALTERNATIVA];
    char resposta;
} Questao;

// Declaração de variáveis globais
Questao perguntas[10]; // Vetor de perguntas. As perguntas são lidas do arquivo questoes.txt.
int totalDePerguntas = 0;
pthread_mutex_t mutex; // Mutex para sincronização de threads
bool perguntaDoMilhao = false; // Indica se a pergunta especial foi escolhida
int indicePerguntaDoMilhao = 0; // Índice da pergunta especial
int pontosJogador1 = 0, pontosJogador2 = 0; // Pontuação dos jogador

// ***********************************************************************************
// * Valida se a alternativa escolhida pelo jogador é válida ('a', 'b', 'c' ou 'd'). *
// ***********************************************************************************
bool validaAlternativa(char alternativa) {
    // caso a alternativa seja válida, retorna true.
    return alternativa == 'a' || alternativa == 'b' || alternativa == 'c' || alternativa == 'd';
}

// ***********************************************
// * Exibe o placar atual dos jogadores na tela. *
// ***********************************************
void exibePlacar() {
    printf("\nJogador 1: %d pontos\n", pontosJogador1);
    printf("Jogador 2: %d pontos\n", pontosJogador2);
    fflush(stdout);
}

// ******************************************************************
// * Thread responsável por gerar o índice de uma pergunta especial *
// * (do milhão) entre as últimas duas perguntas.                   *
// ******************************************************************
void *geraPerguntaDoMilhao() {
    // Garante que a pergunta do milhão seja escolhida apenas uma vez
    if (!perguntaDoMilhao) {
        srand(time(NULL)); // Inicializa o gerador de números aleatórios

        int min = 8, max = 9; // define o intervalo das últimas duas perguntas
        pthread_mutex_lock(&mutex); // Bloqueia o mutex para garantir a exclusão mútua
            indicePerguntaDoMilhao = rand() % (max - min + 1) + min; // Gera um índice aleatório para pergunta do milhão
        pthread_mutex_unlock(&mutex); // Libera o mutex
        perguntaDoMilhao = true; // Indica que a pergunta do milhão foi escolhida
    }
}

// ******************************************************
// * Thread responsável pelo controle do tempo restante * 
// * para que o jogador responda à pergunta.            *
// ******************************************************
void* controleDeTempo(void* jogador) {
    int tempoRestante = TEMPO_MAXIMO_RESPONDER; // Tempo restante para responder a pergunta
    // Enquanto o tempo não esgotar, aguarda um segundo
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
         tempoRestante--; // Decrementa o tempo restante
    }
    printf("Tempo esgotado!\n");
}

// *************************************************************************************
// * Lê as perguntas de um arquivo de texto e as armazena no array global `perguntas`. *
// *************************************************************************************
void lerPerguntas(Questao perguntas[], int *totalDePerguntas) {
    FILE *arquivo = fopen("questoes.txt", "r"); // Abre o arquivo de perguntas
    // Verifica se o arquivo foi aberto corretamente
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de perguntas");
        exit(1);
    }

    int i = 0; // Índice da pergunta
    // Lê cada linha do arquivo no formato especificado e armazena no array de perguntas
    // pergunta | alternativaA | alternativaB | alternativaC | alternativaD | resposta
    while (fscanf(arquivo, " %255[^|]|%99[^|]|%99[^|]|%99[^|]|%99[^|]|%c\n",
                  perguntas[i].pergunta, perguntas[i].alternativaA,
                  perguntas[i].alternativaB, perguntas[i].alternativaC,
                  perguntas[i].alternativaD, &perguntas[i].resposta) == 6) {
        i++;
    }
    *totalDePerguntas = i; // Atualiza o total de perguntas lidas
    fclose(arquivo); // Fecha o arquivo de perguntas
}

// ****************************************************************
// * Função que controla o fluxo de perguntas para o Jogador 1.   *
// * Recebe as perguntas e envia as respostas para o apresentador.*
// ****************************************************************
void jogador1(int readfd, int writefd) {
    // Buffer para armazenar a pergunta e alternativas
    char buffer[MAX_PERGUNTA + 4 * MAX_ALTERNATIVA + 100];
    // Enquanto houver perguntas para serem respondidas
    while (read(readfd, buffer, sizeof(buffer)) > 0) {
        pthread_t threadTempo; // Thread para o tempo
        // Inicia a thread de tempo
        if (pthread_create(&threadTempo, NULL, controleDeTempo, NULL) != 0) {
            perror("Erro ao criar a thread");
            exit(1);
        }; 

        printf("\nPergunta ao jogador 1: \n");
        printf(buffer); // Exibe a pergunta e alternativas
        printf("Responda -> | a | b | c | d |\n");
        char resposta; // Resposta do jogador

        // Aguarda a resposta do jogador
        do{
            scanf("%c", &resposta);
            getchar();
            
            if(validaAlternativa(resposta)) // Valida a alternativa escolhida
                break;
            printf("Resposta invalida\n");
        }while(true);  
        
        write(writefd, &resposta, 1);// Envia a resposta para o apresentador
        pthread_cancel(threadTempo); // Cancela a thread de tempo se a resposta for dada
    }
}

// *****************************************************************
// * Função que controla o fluxo de perguntas para o Jogador 2.    *
// * Recebe as perguntas e envia as respostas para o apresentador. *
// *****************************************************************
void jogador2(int readfd, int writefd) {
    // Buffer para armazenar a pergunta e alternativas
    char buffer[MAX_PERGUNTA + 4 * MAX_ALTERNATIVA + 100];
    // Enquanto houver perguntas para serem respondidas
    while (read(readfd, buffer, sizeof(buffer)) > 0) {
        pthread_t threadTempo; // Thread para o tempo
        // Inicia a thread de tempo
        if (pthread_create(&threadTempo, NULL, controleDeTempo, NULL) != 0) {
            perror("Erro ao criar a thread");
            exit(1);
        }
        
        printf("\nPergunta ao jogador 2: \n");
        printf(buffer); // Exibe a pergunta e alternativas
        printf("Responda -> | a | b | c | d |\n");

        char resposta; // Resposta do jogador
        // Aguarda a resposta do jogador
        do{
            scanf("%c", &resposta);
            getchar();
            
            if(validaAlternativa(resposta)) // Valida a alternativa escolhida
                break;
            printf("Resposta invalida\n");
        }while(true);
        write(writefd, &resposta, 1); // Envia a resposta para o apresentador
        pthread_cancel(threadTempo); // Cancela a thread de tempo se a resposta for dada
    }
}

// **************************************
// * Define o nome do jogo em ASCII Art *
// **************************************
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

// **************************************
// * Define o fim do jogo em ASCII Art *
// **************************************
void printObrigado() {
    printf("  ____   ____   _____   _____   _____            _____    ____  \n");
    printf(" / __ \\ |  _ \\ |  __ \\ |_   _| / ____|    /\\    |  __ \\  / __ \\ \n");
    printf("| |  | || |_) || |__) |  | |  | |  __    /  \\   | |  | || |  | | \n");
    printf("| |  | ||  _ < |  _  /   | |  | | |_ |  / /\\ \\  | |  | || |  | | \n");
    printf("| |__| || |_) || | \\ \\  _| |_ | |__| | / ____ \\ | |__| || |__| | \n");
    printf(" \\____/ |____/ |_|  \\_\\|_____| \\_____|/_/    \\_\\|_____/  \\____/ \n");
}

// ***************************
// * Define o inicio do jogo *
// ***************************
void boasVindas(){
    printBoca();
    printf("******************************************************\n");
    printf("Bem vindo ao jogo do Boca do Milhao!\n");
    printf("******************************************************\n\n");
    printf("Responda as perguntas corretamente e descubra quem sera o ganhador do milhao!\n\n");

    printf("Aperte qualquer tecla para comecar!\n");
    getchar(); // Aguarda o jogador pressionar qualquer tecla para iniciar o jogo
}

int main() {
    int pipeApresentadorJogador1[2], pipeJogador1Apresentador[2]; // Pipes para comunicação entre apresentador e jogador 1
    int pipeApresentadorJogador2[2], pipeJogador2Apresentador[2]; // Pipes para comunicação entre apresentador e jogador 2
    lerPerguntas(perguntas, &totalDePerguntas); // Lê as perguntas do arquivo e armazena no array global
    pthread_t threadIndicePerguntaDoMilhao; // Thread para gerar o índice da pergunta do milhão
    pthread_mutex_init(&mutex, NULL); // Inicializa o mutex

    // Inicializa os pipes, caso haja um erro, encerra o programa
    if (pipe(pipeApresentadorJogador1) < 0 || pipe(pipeJogador1Apresentador) < 0 ||
        pipe(pipeApresentadorJogador2) < 0 || pipe(pipeJogador2Apresentador) < 0) {
        printf("\nErro ao iniciar PIPES!");
        exit(0);
    }

    // Inicializa a thread para gerar o índice da pergunta do milhão, caso haja um erro, encerra o programa
    if (pthread_create(&threadIndicePerguntaDoMilhao, NULL, geraPerguntaDoMilhao, NULL) != 0) {
        perror("Erro ao criar a thread");
        exit(1);
    }

    boasVindas(); // Exibe a mensagem de boas vindas

    pid_t pidJogador1 = fork(); // Cria um processo para o jogador 1
    if (pidJogador1 < 0) { // Caso haja um erro ao criar o processo, encerra o programa
        printf("\nErro ao iniciar o processo Jogador1");
        exit(0);
    }
    if (pidJogador1 == 0) { // Processo do jogador 1
        close(pipeJogador1Apresentador[0]); // Fecha a leitura do pipe jogador 1 -> apresentador
        close(pipeApresentadorJogador1[1]); // Fecha a escrita do pipe apresentador -> jogador 1
        jogador1(pipeApresentadorJogador1[0], pipeJogador1Apresentador[1]); // Inicia o jogador 1
    }

    pid_t pidJogador2 = fork(); // Cria um processo para o jogador 2
    if (pidJogador2 < 0) { // Caso haja um erro ao criar o processo, encerra o programa
        printf("\nErro ao iniciar o processo Jogador2");
        exit(0);
    }
    if (pidJogador2 == 0) { // Processo do jogador 2
        close(pipeJogador2Apresentador[0]); // Fecha a leitura do pipe jogador 2 -> apresentador
        close(pipeApresentadorJogador2[1]); // Fecha a escrita do pipe apresentador -> jogador 2
        jogador2(pipeApresentadorJogador2[0], pipeJogador2Apresentador[1]); // Inicia o jogador 2
    }

    // Processo do apresentador
    for (int i = 0; i < totalDePerguntas; i++) {
        char buffer[MAX_PERGUNTA + 4 * MAX_ALTERNATIVA + 10]; // Buffer para armazenar a pergunta e alternativas
        snprintf(buffer, sizeof(buffer), "%s\na)%s\nb)%s\nc)%s\nd)%s\n",
                 perguntas[i].pergunta, perguntas[i].alternativaA,
                 perguntas[i].alternativaB, perguntas[i].alternativaC,
                 perguntas[i].alternativaD); // Formata a pergunta e alternativas

        if (i % 2 == 0) { // Vez do Jogador 1
            write(pipeApresentadorJogador1[1], buffer, strlen(buffer) + 1); // Envia a pergunta para o jogador 1
            char resposta; // Resposta do jogador
            read(pipeJogador1Apresentador[0], &resposta, sizeof(resposta)); // Recebe a resposta do jogador
            
            // Verifica se a resposta está correta e atualiza a pontuação
            if (resposta == perguntas[i].resposta) {
                if (i == indicePerguntaDoMilhao && i != 0) { // Verifica se a pergunta do milhão foi respondida corretamente
                    printf("Jogador 1 ganhou o Milhao!\n");
                    break;
                }
                printf("Jogador 1 acertou!\n");
                pontosJogador1 += 10; // Atualiza a pontuação do jogador 1
            } else {
                printf("Jogador 1 errou. Resposta correta: %c\n", perguntas[i].resposta); // Caso o jogar erre, exibe a resposta correta
            }
        } else { // Vez Jogador 2
            write(pipeApresentadorJogador2[1], buffer, strlen(buffer) + 1); // Envia a pergunta para o jogador 2
            char resposta; // Resposta do jogador 2
            read(pipeJogador2Apresentador[0], &resposta, sizeof(resposta)); // Recebe a resposta do jogador 2
            if (resposta == perguntas[i].resposta) { // Verifica se a resposta está correta e atualiza a pontuação
                if (i == indicePerguntaDoMilhao && i != 0) { // Verifica se a pergunta do milhão foi respondida corretamente
                    printf("Jogador 2 ganhou o Milhao!\n");
                    break;
                }
                printf("Jogador 2 acertou!\n");
                pontosJogador2 += 10; // Atualiza a pontuação do jogador 2
            } else {
                printf("Jogador 2 errou. Resposta correta: %c\n", perguntas[i].resposta); // Caso o jogador erre, exibe a resposta correta
            }
        }

        exibePlacar(); // Exibe o placar atual
    }

    printf("******Pontuação final******\n");
    exibePlacar(); // Exibe o placar final
    if (pontosJogador1 > pontosJogador2) { // Verifica o vencedor
        printf("Jogador 1 ganhou!\n");
    }else if (pontosJogador1 < pontosJogador2){
        printf("Jogador 2 ganhou!\n");
    }else{
        printf("Empate!\n");
    }

    // Encerra os processos dos jogadores
    close(pipeApresentadorJogador1[1]);
    close(pipeApresentadorJogador1[0]);
    close(pipeApresentadorJogador2[1]);
    close(pipeJogador2Apresentador[0]);
    printObrigado(); // Exibe a mensagem de agradecimento
    printf("Desenvolvido por: Gabriel H Coetti & Joao Pedro Barbosa");
    wait(NULL); // Aguarda os processos filhos encerrarem
    wait(NULL);
    
    return 0;
}

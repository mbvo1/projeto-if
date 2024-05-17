#include <stdio.h>      // Biblioteca padrão de entrada e saída
#include <unistd.h>     // Biblioteca para usleep() e manipulação de POSIX
#include <stdlib.h>     // Biblioteca para funções gerais, como rand()
#include <termios.h>    // Biblioteca para controle do terminal
#include <fcntl.h>      // Biblioteca para manipulação de arquivos

#define WIDTH 50      // Define a largura do campo de jogo
#define HEIGHT 25      // Define a altura do campo de jogo

int playerX, bulletX, bulletY, enemyX, enemyY, gameOver;
char screen[HEIGHT][WIDTH];  // Matriz que representa a tela do jogo

// Função que checa se uma tecla foi pressionada
int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    // Salva a configuração atual do terminal
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    // Desativa entrada canônica e eco
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    // Define o modo de leitura não bloqueante
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    // Restaura as configurações originais do terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);  // Devolve o caractere lido ao buffer
        return 1;
    }

    return 0;
}

// Inicializa o jogo
void initialize() {
    int i, j;
    playerX = WIDTH / 2;
    bulletX = -1;
    bulletY = -1;
    enemyX = rand() % (WIDTH - 2) + 1;
    enemyY = 1;
    gameOver = 0;
    
    // Preenche a tela com bordas e espaço vazio
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            if (i == 0 || i == HEIGHT - 1 || j == 0 || j == WIDTH - 1)
                screen[i][j] = '#';
            else
                screen[i][j] = ' ';
        }
    }
    screen[HEIGHT - 2][playerX] = '|';  // Coloca o jogador na posição inicial
}

// Desenha a tela do jogo
void draw() {
    system("clear || printf '\\033c'");  // Limpa a tela
    int i, j;
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            printf("%c", screen[i][j]);
        }
        printf("\n");
    }
}

// Atualiza o estado do jogo
void update() {
    if (kbhit()) {
        char key = getchar();
        // Movimento do jogador
        if (key == 'a' && playerX > 1) {
            screen[HEIGHT - 2][playerX] = ' ';
            playerX--;
            screen[HEIGHT - 2][playerX] = '|';
        } else if (key == 'd' && playerX < WIDTH - 2) {
            screen[HEIGHT - 2][playerX] = ' ';
            playerX++;
            screen[HEIGHT - 2][playerX] = '|';
        } else if (key == ' ' && bulletY == -1) {
            // Disparo do tiro
            bulletX = playerX;
            bulletY = HEIGHT - 3;
        }
    }
    // Atualização do tiro
    if (bulletY >= 1) {
        screen[bulletY][bulletX] = ' ';
        bulletY--;
        // Checa se o tiro atingiu o inimigo
        if (bulletY == enemyY && bulletX == enemyX) {
            bulletY = -1;
            enemyX = rand() % (WIDTH - 2) + 1;
            enemyY = 1;
        } else {
            screen[bulletY][bulletX] = '*';
        }
    } else if (bulletY == 0) {
        bulletY = -1;
        screen[bulletY][bulletX] = ' ';
    }

    // Movimento do inimigo
    if (enemyY < HEIGHT - 2) {
        screen[enemyY][enemyX] = ' ';
        enemyY++;
        screen[enemyY][enemyX] = 'Z';

        // Checa colisão com o jogador
        if (enemyY == HEIGHT - 2 && enemyX == playerX) {
            gameOver = 1;
        }
    } else {
        // Reinicia a posição do inimigo ao atingir o chão
        enemyX = rand() % (WIDTH - 2) + 1;
        enemyY = 1;
    }
}

// Mostra a tela de fim de jogo
void gameOverScreen() {
    system("clear || printf '\\033c'");
    printf("Game Over!\n");
}

// Mostra o menu inicial
void menu() {
    printf("Pressione a tecla enter para jogar.\n");
    getchar(); 
}

int main() {
    int playAgain = 1;
    while (playAgain) {
        menu(); 
        initialize();
        while (!gameOver) {
            draw();
            update();
            usleep(100000);  // Espera 100 ms
        }
        gameOverScreen();
        printf("Selecione Enter para jogar novamente ou 's' para sair.\n");
        char input = getchar();
        if (input == 's' || input == 'S')
            playAgain = 0;
        else {
            gameOver = 0;
            getchar(); // Consumir o caractere de nova linha
        }
    }
    return 0;
}

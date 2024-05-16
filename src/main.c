#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>

#define WIDTH 100
#define HEIGHT 50

int playerX, bulletX, bulletY, enemyX, enemyY, gameOver;
char screen[HEIGHT][WIDTH];

int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void initialize() {
    int i, j;
    playerX = WIDTH / 2;
    bulletX = -1;
    bulletY = -1;
    enemyX = rand() % (WIDTH - 2) + 1;
    enemyY = 1;
    gameOver = 0;
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            if (i == 0 || i == HEIGHT - 1 || j == 0 || j == WIDTH - 1)
                screen[i][j] = '#';
            else
                screen[i][j] = ' ';
        }
    }
    screen[HEIGHT - 2][playerX] = '|';
}

void draw() {
    system("clear || printf '\\033c'");
    int i, j;
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            printf("%c", screen[i][j]);
        }
        printf("\n");
    }
}

void update() {
    if (kbhit()) {
        char key = getchar();
        if (key == 'a' && playerX > 1) {
            screen[HEIGHT - 2][playerX] = ' ';
            playerX--;
            screen[HEIGHT - 2][playerX] = '|';
        } else if (key == 'd' && playerX < WIDTH - 2) {
            screen[HEIGHT - 2][playerX] = ' ';
            playerX++;
            screen[HEIGHT - 2][playerX] = '|';
        } else if (key == ' ' && bulletY == -1) {
            bulletX = playerX;
            bulletY = HEIGHT - 3;
        }
    }
    if (bulletY >= 1) {
        screen[bulletY][bulletX] = ' ';
        bulletY--;
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
        // Se o inimigo atingir o chão, reinicia sua posição
        enemyX = rand() % (WIDTH - 2) + 1;
        enemyY = 1;
    }
}

void gameOverScreen() {
    system("clear || printf '\\033c'");
    printf("Game Over!\n");
}

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
            usleep(100000); 
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
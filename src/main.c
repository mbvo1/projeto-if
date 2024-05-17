#include "screen.h"
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>

// Estruturas para os objetos do jogo
typedef struct {
    int x, y;
} GameObject;

typedef struct {
    GameObject invaders[5];
    GameObject player;
    GameObject bullets[5];
    int numInvaders;
    int numBullets;
    int score;
} Game;

// Funções auxiliares
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

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

// Inicializar o jogo
void initGame(Game *game) {
    game->numInvaders = 0;  // Inicia sem invasores
    game->numBullets = 0;
    game->score = 0;

    game->player.x = MAXX / 2;
    game->player.y = MAXY - 2;

    srand(time(NULL));
}

// Desenhar o jogo na tela
void drawGame(Game *game) {
    screenClear();
    
    // Desenhar invasores
    for (int i = 0; i < game->numInvaders; i++) {
        screenGotoxy(game->invaders[i].x, game->invaders[i].y);
        printf("W");
    }
    
    // Desenhar jogador
    screenGotoxy(game->player.x, game->player.y);
    printf("^");
    
    // Desenhar tiros
    for (int i = 0; i < game->numBullets; i++) {
        screenGotoxy(game->bullets[i].x, game->bullets[i].y);
        printf("|");
    }

    // Exibir pontuação
    screenGotoxy(1, MAXY + 1);
    printf("Score: %d", game->score);
    
    screenUpdate();
}

// Atualizar a lógica do jogo
void updateGame(Game *game) {
    // Mover tiros
    for (int i = 0; i < game->numBullets; i++) {
        game->bullets[i].y--;
        if (game->bullets[i].y < MINY) {
            // Remover tiro fora da tela
            for (int j = i; j < game->numBullets - 1; j++) {
                game->bullets[j] = game->bullets[j + 1];
            }
            game->numBullets--;
            i--;
        }
    }

    // Mover invasores
    for (int i = 0; i < game->numInvaders; i++) {
        game->invaders[i].y++;
        if (game->invaders[i].y >= MAXY) {
            // Remover invasor que passou do jogador
            for (int j = i; j < game->numInvaders - 1; j++) {
                game->invaders[j] = game->invaders[j + 1];
            }
            game->numInvaders--;
            i--;
        } else if (game->invaders[i].x == game->player.x && game->invaders[i].y == game->player.y) {
            // Jogo termina quando um invasor toca o jogador
            game->numInvaders = 0;
            game->numBullets = 0;
            printf("\nGAME OVER!\n");
            screenDestroy();
            exit(0);
        }
    }
    
    // Checar colisões (simples)
    for (int i = 0; i < game->numBullets; i++) {
        for (int j = 0; j < game->numInvaders; j++) {
            if (game->bullets[i].x == game->invaders[j].x && game->bullets[i].y == game->invaders[j].y) {
                // Remover invasor
                for (int k = j; k < game->numInvaders - 1; k++) {
                    game->invaders[k] = game->invaders[k + 1];
                }
                game->numInvaders--;
                game->score += 10; // Aumentar pontuação por invasor eliminado
                j--;
                
                // Remover tiro
                for (int k = i; k < game->numBullets - 1; k++) {
                    game->bullets[k] = game->bullets[k + 1];
                }
                game->numBullets--;
                i--;
                break;
            }
        }
    }

    // Gerar novos invasores aleatoriamente
    if (game->numInvaders < 5) {
        if (rand() % 20 == 0) {  // Probabilidade de gerar um novo invasor
            game->invaders[game->numInvaders].x = rand() % (MAXX - MINX + 1) + MINX;
            game->invaders[game->numInvaders].y = MINY;
            game->numInvaders++;
        }
    }
}

// Controlar a entrada do usuário
void handleInput(Game *game) {
    if (kbhit()) {
        char ch = getchar();
        
        if (ch == 'a' && game->player.x > MINX) {
            game->player.x--;
        } else if (ch == 'd' && game->player.x < MAXX - 1) {
            game->player.x++;
        } else if (ch == ' ') {
            // Disparar tiro
            if (game->numBullets < 5) {
                game->bullets[game->numBullets].x = game->player.x;
                game->bullets[game->numBullets].y = game->player.y - 1;
                game->numBullets++;
            }
        }
    }
}

int main() {
    Game game;
    
    screenInit(1);
    initGame(&game);
    
    while (1) {
        handleInput(&game);
        updateGame(&game);
        drawGame(&game);
        usleep(100000); // Pequena pausa para controle da velocidade do jogo
    }
    
    screenDestroy();
    return 0;
}

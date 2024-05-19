#include <stdio.h>
#include "screen.h"
#include "timer.h"
#include "keyboard.h"

#define PLAYER_SYMBOL 'A'
#define INVADER_SYMBOL 'W'
#define BULLET_SYMBOL '|'
#define MAX_INVADERS 10
#define MAX_BULLETS 5

typedef struct {
    int x, y;
} Object;

Object player;
Object invaders[MAX_INVADERS];
Object bullets[MAX_BULLETS];

int numInvaders = MAX_INVADERS;
int numBullets = 0;
int score = 0;  // Variável para armazenar a pontuação

void initGame() {
    screenInit(0);  // Não desenhar bordas
    keyboardInit();
    timerInit(100); // 100 ms de intervalo para o temporizador

    // Inicializar o jogador
    player.x = MAXX / 2;
    player.y = MAXY - 2;

    // Inicializar os invasores
    for (int i = 0; i < MAX_INVADERS; i++) {
        invaders[i].x = (i % 5) * 10 + 5;
        invaders[i].y = (i / 5) * 2 + 1;
    }
}

void destroyGame() {
    keyboardDestroy();
    screenDestroy();
    timerDestroy();
}

void drawObject(Object obj, char symbol) {
    screenGotoxy(obj.x, obj.y);
    printf("%c", symbol);
}

void drawScore() {
    screenGotoxy(MINX, MAXY);  // Desenhar na linha inferior da tela
    printf("Score: %d", score);
}

void drawGame() {
    screenClear();

    // Desenhar o jogador
    drawObject(player, PLAYER_SYMBOL);

    // Desenhar os invasores
    for (int i = 0; i < numInvaders; i++) {
        drawObject(invaders[i], INVADER_SYMBOL);
    }

    // Desenhar os tiros
    for (int i = 0; i < numBullets; i++) {
        drawObject(bullets[i], BULLET_SYMBOL);
    }

    // Desenhar a pontuação
    drawScore();

    screenUpdate();
}

void updateBullets() {
    for (int i = 0; i < numBullets; i++) {
        bullets[i].y--;
        if (bullets[i].y < MINY) {
            // Remover a bala
            for (int j = i; j < numBullets - 1; j++) {
                bullets[j] = bullets[j + 1];
            }
            numBullets--;
            i--;
        }
    }
}

void shootBullet() {
    if (numBullets < MAX_BULLETS) {
        bullets[numBullets].x = player.x;
        bullets[numBullets].y = player.y - 1;
        numBullets++;
    }
}

void updateGame() {
    if (keyhit()) {
        int ch = readch();
        if (ch == 'a' && player.x > MINX + 1) {
            player.x--;
        } else if (ch == 'd' && player.x < MAXX - 1) {
            player.x++;
        } else if (ch == ' ') {
            shootBullet();
        }
    }

    updateBullets();

    // Verificar colisões com os invasores
    for (int i = 0; i < numInvaders; i++) {
        for (int j = 0; j < numBullets; j++) {
            if (invaders[i].x == bullets[j].x && invaders[i].y == bullets[j].y) {
                // Remover o invasor
                for (int k = i; k < numInvaders - 1; k++) {
                    invaders[k] = invaders[k + 1];
                }
                numInvaders--;

                // Atualizar a pontuação
                score += 10;

                // Remover a bala
                for (int k = j; k < numBullets - 1; k++) {
                    bullets[k] = bullets[k + 1];
                }
                numBullets--;

                i--;
                break;
            }
        }
    }
}

int main() {
    initGame();

    while (numInvaders > 0) {
        if (timerTimeOver()) {
            updateGame();
            drawGame();
        }
    }

    destroyGame();
    printf("Game Over! Final Score: %d\n", score);

    return 0;
}

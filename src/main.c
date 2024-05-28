#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "screen.h"
#include "timer.h"
#include "keyboard.h"

#define PLAYER_SYMBOL 'H'
#define INVADER_SYMBOL 'Z'
#define BULLET_SYMBOL '|'
#define MAX_INVADERS 20
#define MAX_BULLETS 3

typedef struct Object {
    int x, y;
    struct Object* next;
} Object;

typedef struct {
    Object* head;
    int size;
} LinkedList;

Object player;
LinkedList invaders;
LinkedList bullets;
int score;
int gameOver;
char playerName[50];

void initLinkedList(LinkedList* list) {
    list->head = NULL;
    list->size = 0;
}

void addObject(LinkedList* list, int x, int y) {
    Object* newObject = (Object*)malloc(sizeof(Object));
    newObject->x = x;
    newObject->y = y;
    newObject->next = list->head;
    list->head = newObject;
    list->size++;
}

void removeObject(LinkedList* list, Object* prev, Object* toRemove) {
    if (prev == NULL) {
        list->head = toRemove->next;
    } else {
        prev->next = toRemove->next;
    }
    free(toRemove);
    list->size--;
}

void clearLinkedList(LinkedList* list) {
    Object* current = list->head;
    while (current != NULL) {
        Object* toRemove = current;
        current = current->next;
        free(toRemove);
    }
    list->head = NULL;
    list->size = 0;
}

void initGame() {
    screenInit(0);
    keyboardInit();
    timerInit(100);
    srand(time(NULL));

    initLinkedList(&invaders);
    initLinkedList(&bullets);

    score = 0;
    gameOver = 0;

    player.x = MAXX / 2;
    player.y = MAXY - 2;

    for (int i = 0; i < MAX_INVADERS; i++) {
        addObject(&invaders, (i % 5) * 10 + 5, (i / 5) * 2 + 1);
    }
}

void destroyGame() {
    clearLinkedList(&invaders);
    clearLinkedList(&bullets);
    keyboardDestroy();
    screenDestroy();
    timerDestroy();
}

void drawObject(Object* obj, char symbol) {
    screenGotoxy(obj->x, obj->y);
    printf("%c", symbol);
}

void drawScore() {
    screenGotoxy(MINX, MAXY);
    printf("Score: %d", score);
}

void drawGame() {
    screenClear();
    drawObject(&player, PLAYER_SYMBOL);

    Object* current = invaders.head;
    while (current != NULL) {
        drawObject(current, INVADER_SYMBOL);
        current = current->next;
    }

    current = bullets.head;
    while (current != NULL) {
        drawObject(current, BULLET_SYMBOL);
        current = current->next;
    }

    drawScore();
    screenUpdate();
}

void updateBullets() {
    Object* current = bullets.head;
    Object* prev = NULL;

    while (current != NULL) {
        current->y--;

        if (current->y < MINY) {
            Object* toRemove = current;
            current = current->next;
            removeObject(&bullets, prev, toRemove);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

void shootBullet() {
    if (bullets.size < MAX_BULLETS) {
        addObject(&bullets, player.x, player.y - 1);
    }
}

void updateInvaders() {
    for (int i = 0; i < 2; i++) {
        if (invaders.size > 0) {
            int invaderIndex = rand() % invaders.size;
            Object* current = invaders.head;
            for (int j = 0; j < invaderIndex; j++) {
                current = current->next;
            }
            current->y++;
            if (current->y >= player.y && current->x == player.x) {
                gameOver = 1;
            }
            if (current->y > MAXY) {
                current->y = 1;
            }
        }
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
    updateInvaders();

    Object* invader = invaders.head;
    Object* prevInvader = NULL;

    while (invader != NULL) {
        Object* bullet = bullets.head;
        Object* prevBullet = NULL;

        while (bullet != NULL) {
            if (invader->x == bullet->x && invader->y == bullet->y) {
                removeObject(&invaders, prevInvader, invader);
                removeObject(&bullets, prevBullet, bullet);
                score += 10;
                break;
            }
            prevBullet = bullet;
            bullet = bullet->next;
        }
        prevInvader = invader;
        invader = invader->next;
    }
}

void showScores() {
    FILE *file = fopen("score.txt", "r");
    if (file != NULL) {
        char line[100];
        while (fgets(line, sizeof(line), file)) {
            printf("%s", line);
        }
        fclose(file);
    } else {
        printf("Nenhum score disponível.\n");
    }
}

int main() {
    int choice;

    do {
        printf("Menu:\n");
        printf("1: Ver score\n");
        printf("2: Jogar o jogo\n");
        printf("3: Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                showScores();
                break;
            case 2:
                printf("Digite seu nome: ");
                fgets(playerName, 50, stdin);
                playerName[strcspn(playerName, "\n")] = 0;

                initGame();

                while (invaders.size > 0 && !gameOver) {
                    if (timerTimeOver()) {
                        updateGame();
                        drawGame();
                    }
                }

                destroyGame();
                printf("Game Over! Final Score: %d\n", score);

                FILE *file = fopen("score.txt", "a");
                if (file != NULL) {
                    fprintf(file, "Nome: %s, Score: %d\n", playerName, score);
                    fclose(file);
                } else {
                    printf("Erro ao abrir o arquivo score.txt\n");
                }
                break;
            case 3:
                printf("Jogo encerrado, OBRIGADO!\n");
                break;
            default:
                printf("Opção inválida. Tente novamente.\n");
        }
    } while (choice != 3);

    return 0;
}

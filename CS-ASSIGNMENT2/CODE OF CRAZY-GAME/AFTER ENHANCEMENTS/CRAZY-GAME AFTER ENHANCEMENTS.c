// CRAZY-GAME CODE AFTER ENHANCEMENTS
// Features: Score, Lives (3), Difficulty (speed increases with score), Pause (P), Quit (Q)

#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand((unsigned)time(0));

    int x = 1;                  // player lane (0 to 2)
    int step = 0;               // obstacle vertical position (0..10)
    int obstaclePos = rand() % 3; // obstacle lane
    int lives = 3;              // player lives
    int score = 0;              // score counter
    int level = 1;              // difficulty level
    int base_sleep = 120;       // base speed (ms)
    int sleep_ms = base_sleep;  // current speed
    const int max_level = 10;   // just a cap for display
    int running = 1;            // game loop flag
    int paused = 0;

    // helper to compute level/speed from score
    void update_difficulty() {
        level = 1 + score / 5; // level up every 5 points
        if (level < 1) level = 1;
        if (level > max_level) level = max_level;
        // decrease sleep as level increases (faster = smaller sleep)
        sleep_ms = base_sleep - (level - 1) * 10;
        if (sleep_ms < 40) sleep_ms = 40; // cap minimum delay
    }

    update_difficulty();

    while (running) {

        // ---- INPUT ----
        if (_kbhit()) {
            int ch = getch();
            // arrow keys return 0 or 224 first, then code
            if (ch == 0 || ch == 224) {
                ch = getch();
                if (ch == 75 && x > 0)        // LEFT arrow
                    x--;
                else if (ch == 77 && x < 2)   // RIGHT arrow
                    x++;
            } else {
                // regular keys: P to pause, Q to quit
                if (ch == 'p' || ch == 'P') {
                    paused = !paused;
                } else if (ch == 'q' || ch == 'Q') {
                    running = 0;
                    break;
                }
            }
        }

        if (paused) {
            // Draw pause screen (simple)
            system("cls");
            printf("=== CRAZY GAME (PAUSED) ===\n");
            printf("Score : %d    Lives : %d    Level : %d\n\n", score, lives, level);
            printf("Controls: ? ? to move, P to pause/resume, Q to quit\n");
            Sleep(100);
            continue;
        }

        // ---- DRAW HUD AND PLAYFIELD ----
        system("cls");
        printf("=== CRAZY GAME ===\n");
        printf("Score : %d    Lives : %d    Level : %d\n", score, lives, level);
        printf("|--- --- ---|\n");

        for (int i = 0; i < 10; i++) {
            if (i == step) {
                if (obstaclePos == 0)
                    printf("| %c        |\n", 1);
                else if (obstaclePos == 1)
                    printf("|     %c    |\n", 1);
                else if (obstaclePos == 2)
                    printf("|        %c |\n", 1);
            } else {
                printf("|           |\n");
            }
        }

        // ---- PLAYER ----
        if (x == 0)
            printf("| %c        |\n", 6);
        else if (x == 1)
            printf("|     %c    |\n", 6);
        else if (x == 2)
            printf("|        %c |\n", 6);

        // ---- COLLISION or PASS CHECK ----
        // When obstacle reaches the player row (step == 10) we resolve it
        if (step == 10) {
            if (x == obstaclePos) {
                // collision
                lives--;
                // small feedback
                printf("\n!!! HIT !!!  Lives left: %d\n", lives);
                // brief pause to show the hit
                Sleep(600);

                if (lives <= 0) {
                    system("cls");
                    printf("=== CRAZY GAME ===\n");
                    printf("Score : %d    Lives : 0    Level : %d\n", score, level);
                    printf("\nGAME OVER! You lost all lives.\n");
                    printf("Final Score: %d   Final Level: %d\n", score, level);
                    running = 0;
                    break;
                }
            } else {
                // successfully avoided the obstacle -> increase score
                score++;
                update_difficulty();
            }
        }

        // Delay according to current difficulty
        Sleep(sleep_ms);

        // Move obstacle down
        step++;

        // Reset when reaches bottom (step > 10)
        if (step > 10) {
            step = 0;
            obstaclePos = rand() % 3; // new lane
        }
    }

    printf("\nThanks for playing!\n");
    return 0;
}

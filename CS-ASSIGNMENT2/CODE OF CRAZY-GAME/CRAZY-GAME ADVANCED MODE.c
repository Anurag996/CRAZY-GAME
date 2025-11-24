// CRAZY GAME ON ADVANCED MODE !!!!1
/*
 Rapid Roll Ball - Console Game (Windows)
 - Move the ball left/right with ? and ?.
 - Avoid falling obstacles (#).
 - Score increases for each avoided obstacle.
 - You start with 3 lives. Game over when lives == 0.
 - Difficulty increases as score rises.
 - P pauses/resumes. Q quits.
 
 Compile/Run: Visual Studio, MinGW (Windows only).
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

#define WIDTH 11        // playfield width (columns)
#define HEIGHT 14       // playfield height (rows including HUD area)
#define PLAY_HEIGHT 11  // number of rows used for falling obstacles (visual)
#define HUD_ROWS 3      // HUD lines above the playfield

// Characters
#define BALL_CHAR 'O'
#define OBST_CHAR '#'
#define EMPTY_CHAR ' '

// Game parameters (tweakable)
int initial_lives = 3;
int base_delay_ms = 140;   // base frame delay (lower -> faster)
int min_delay_ms = 40;     // cap minimum delay
int score_per_level = 6;   // how many points per level-up

// Console helpers ---------------------------------------------------------
HANDLE hConsole;

void set_color(int attr) {
    SetConsoleTextAttribute(hConsole, attr);
}

void clear_screen() {
    // faster-ish clear: fill console buffer with spaces
    system("cls");
}

// Sleep wrapper
void wait_ms(int ms) {
    Sleep(ms);
}

// Game structures ---------------------------------------------------------
typedef struct {
    int x; // column 0..WIDTH-1
    int y; // row 0..PLAY_HEIGHT-1 (0 top)
    int active;
} Obstacle;

// Game state --------------------------------------------------------------
int player_x;
int score;
int lives;
int level;
int delay_ms;         // current frame delay
int spawn_chance;     // probability weight for spawning (lower -> more spawn)
Obstacle obstacles[50]; // pool of obstacles (max concurrent)
int max_obstacles = 12; // safe number to use from pool
int running = 1;
int paused = 0;

// Utility functions ------------------------------------------------------
void init_game() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    srand((unsigned)time(NULL));
    player_x = WIDTH / 2;
    score = 0;
    lives = initial_lives;
    level = 1;
    delay_ms = base_delay_ms;
    spawn_chance = 22; // initial spawn threshold (bigger means rarer spawn)
    for (int i = 0; i < 50; ++i) {
        obstacles[i].active = 0;
    }
}

// Get free obstacle slot from pool
int get_free_obstacle() {
    for (int i = 0; i < max_obstacles; ++i) {
        if (!obstacles[i].active) return i;
    }
    return -1;
}

// Spawn a new obstacle at random column, optionally with chance to create double
void spawn_obstacle() {
    // spawn probability: reduce spawn_chance as level increases
    int roll = rand() % (spawn_chance);
    if (roll != 0) return; // mostly no spawn; 1/spawn_chance chance to spawn

    int idx = get_free_obstacle();
    if (idx == -1) return;

    obstacles[idx].active = 1;
    obstacles[idx].y = 0;
    obstacles[idx].x = rand() % WIDTH;
}

// Move all active obstacles down; return collisions with player (1 if collided)
int move_obstacles_and_check_collision() {
    int collided = 0;
    for (int i = 0; i < max_obstacles; ++i) {
        if (!obstacles[i].active) continue;
        obstacles[i].y += 1;
        // If obstacle passed bottom without hitting player, award score and deactivate
        if (obstacles[i].y > PLAY_HEIGHT - 1) {
            // obstacle escaped -> score
            score++;
            obstacles[i].active = 0;
        } else {
            // Check collision with player (player is always at bottom row PLAY_HEIGHT-1)
            if (obstacles[i].y == PLAY_HEIGHT - 1 && obstacles[i].x == player_x) {
                collided = 1;
                obstacles[i].active = 0; // remove obstacle on collision
            }
        }
    }
    return collided;
}

// Adjust difficulty based on score (increase level, speed, spawn rate)
void update_difficulty() {
    int new_level = 1 + score / score_per_level;
    if (new_level != level) {
        level = new_level;
    }
    // speed up slightly with level
    delay_ms = base_delay_ms - (level - 1) * 8;
    if (delay_ms < min_delay_ms) delay_ms = min_delay_ms;
    // spawn becomes more frequent as level increases
    // make spawn_chance smaller as level rises (but not lower than 6)
    spawn_chance = 22 - (level - 1) * 2;
    if (spawn_chance < 6) spawn_chance = 6;
}

// Drawing functions ------------------------------------------------------
void draw_hud() {
    set_color(7);
    printf(" Rapid Roll Ball  -  Score: %d  Lives: %d  Level: %d\n", score, lives, level);
    printf("+");
    for (int i = 0; i < WIDTH; ++i) printf("-");
    printf("+\n");
}

void draw_playfield() {
    // Build a buffer for the playfield rows
    char grid[PLAY_HEIGHT][WIDTH];
    // Initialize empty
    for (int r = 0; r < PLAY_HEIGHT; ++r)
        for (int c = 0; c < WIDTH; ++c)
            grid[r][c] = EMPTY_CHAR;

    // Place obstacles
    for (int i = 0; i < max_obstacles; ++i) {
        if (!obstacles[i].active) continue;
        int ox = obstacles[i].x;
        int oy = obstacles[i].y;
        if (oy >= 0 && oy < PLAY_HEIGHT && ox >= 0 && ox < WIDTH) {
            grid[oy][ox] = OBST_CHAR;
        }
    }

    // Place player on bottom row
    grid[PLAY_HEIGHT - 1][player_x] = BALL_CHAR;

    // Render rows top -> bottom
    for (int r = 0; r < PLAY_HEIGHT; ++r) {
        printf("|");
        for (int c = 0; c < WIDTH; ++c) {
            char ch = grid[r][c];
            if (ch == OBST_CHAR) {
                set_color(12); // red for obstacles
                printf("%c", ch);
                set_color(7);
            } else if (ch == BALL_CHAR) {
                set_color(10); // green ball
                printf("%c", ch);
                set_color(7);
            } else {
                printf("%c", ch);
            }
        }
        printf("|\n");
    }

    // bottom border
    printf("+");
    for (int i = 0; i < WIDTH; ++i) printf("-");
    printf("+\n");
    // controls hint
    printf(" Controls: ? ? to move   P = Pause   Q = Quit\n");
}

// Input processing (non-blocking)
void process_input() {
    if (!_kbhit()) return;
    int ch = getch();
    if (ch == 0 || ch == 224) { // arrow or special key
        ch = getch();
        if (ch == 75) { // left arrow
            if (player_x > 0) player_x--;
        } else if (ch == 77) { // right arrow
            if (player_x < WIDTH - 1) player_x++;
        }
    } else {
        if (ch == 'p' || ch == 'P') {
            paused = !paused;
        } else if (ch == 'q' || ch == 'Q') {
            running = 0;
        }
    }
}

// Main loop ---------------------------------------------------------------
int main() {
    init_game();

    // Limit console buffer and window size for a stable appearance (optional)
    // (This is simple and may not work in all terminals, but it's fine for typical Windows cmd)
    SMALL_RECT windowSize = {0, 0, WIDTH + 1, PLAY_HEIGHT + HUD_ROWS + 1};
    COORD bufferSize = {WIDTH + 2, PLAY_HEIGHT + HUD_ROWS + 2};
    SetConsoleScreenBufferSize(hConsole, bufferSize);
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

    while (running) {
        if (paused) {
            clear_screen();
            set_color(11);
            printf("=== PAUSED ===\n");
            set_color(7);
            printf("Score: %d   Lives: %d   Level: %d\n", score, lives, level);
            printf("Press P to resume or Q to quit.\n");
            wait_ms(120);
            process_input();
            continue;
        }

        // Input
        process_input();

        // Spawn new obstacles (chance-based)
        spawn_obstacle();

        // Move obstacles and check collision
        int hit = move_obstacles_and_check_collision();
        if (hit) {
            // collision: lose a life and show brief feedback
            lives--;
            clear_screen();
            set_color(12);
            printf("!!! HIT !!!  Lives left: %d\n", lives);
            set_color(7);
            wait_ms(420);
            if (lives <= 0) {
                clear_screen();
                set_color(12);
                printf("=== GAME OVER ===\n");
                set_color(7);
                printf("Final Score: %d   Final Level: %d\n", score, level);
                break;
            }
        }

        // Difficulty updates
        update_difficulty();

        // Draw everything
        clear_screen();
        draw_hud();
        draw_playfield();

        // Wait
        wait_ms(delay_ms);
    }

    printf("\nThanks for playing Rapid Roll Ball! Final Score: %d\n", score);
    return 0;
}

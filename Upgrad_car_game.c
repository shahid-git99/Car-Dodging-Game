#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <windows.h>

#define SCREEN_HEIGHT 18
#define NUM_LANES 5
#define LANE_WIDTH 5
#define TOTAL_WIDTH (NUM_LANES * LANE_WIDTH + 2)

// ANSI Color Codes
#define COLOR_RESET   "\033[0m"
#define COLOR_PLAYER  "\033[1;32m" // Bright Green
#define COLOR_ENEMY   "\033[1;31m" // Bright Red
#define COLOR_ROAD    "\033[1;30m" // Dark Gray
#define COLOR_TEXT    "\033[1;36m" // Cyan

int playerLane = 2; // Middle lane (0 to 4)
int enemyLane = -1;
int enemyY = -6;    // Start above the screen (since cars are 3 lines tall)
int score = 0;
int highScore = 0;
int gameSpeed = 90; // Milliseconds per frame

// Reset cursor position to (0,0) to prevent screen flickering
void resetCursor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = {0, 0};
    SetConsoleCursorPosition(hOut, coord);
}

// Hide console cursor for a cleaner look
void hideCursor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

// High Score Persistence
void loadHighScore() {
    FILE *file = fopen("highscore.txt", "r");
    if (file != NULL) {
        fscanf(file, "%d", &highScore);
        fclose(file);
    }
}

void saveHighScore() {
    if (score > highScore) {
        highScore = score;
        FILE *file = fopen("highscore.txt", "w");
        if (file != NULL) {
            fprintf(file, "%d", highScore);
            fclose(file);
        }
    }
}

// Render the 3-line tall car ASCII model into screen buffer
void drawCar(char grid[SCREEN_HEIGHT][TOTAL_WIDTH + 1], int lane, int y, int isPlayer) {
    int startX = 1 + lane * LANE_WIDTH;
    const char *sprite[3];

    if (isPlayer) {
        sprite[0] = " /\\ ";
        sprite[1] = "[||]";
        sprite[2] = "/  \\";
    } else {
        sprite[0] = "//\\\\";
        sprite[1] = "|==|";
        sprite[2] = "\\\\//";
    }

    for (int r = 0; r < 3; r++) {
        int targetY = y + r;
        if (targetY >= 0 && targetY < SCREEN_HEIGHT) {
            for (int c = 0; c < 4; c++) {
                grid[targetY][startX + c] = sprite[r][c];
            }
        }
    }
}

void drawGame() {
    resetCursor();

    char grid[SCREEN_HEIGHT][TOTAL_WIDTH + 1];

    // 1. Clear grid to blank road space
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < TOTAL_WIDTH; x++) {
            if (x == 0 || x == TOTAL_WIDTH - 1) {
                grid[y][x] = '|'; // Outer track border
            } else if (x % LANE_WIDTH == 0) {
                grid[y][x] = '.'; // Lane divider lines
            } else {
                grid[y][x] = ' ';
            }
        }
        grid[y][TOTAL_WIDTH] = '\0';
    }

    // 2. Overlay Cars
    if (enemyLane != -1) {
        drawCar(grid, enemyLane, enemyY, 0);
    }
    drawCar(grid, playerLane, SCREEN_HEIGHT - 3, 1);

    // 3. Render frame to terminal with colors
    printf(COLOR_TEXT "  === SPACIOUS HIGHWAY RACER ===" COLOR_RESET "\n");
    printf("  Score: " COLOR_TEXT "%-5d" COLOR_RESET "  High Score: " COLOR_TEXT "%-5d" COLOR_RESET "\n\n", score, highScore);

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < TOTAL_WIDTH; x++) {
            char ch = grid[y][x];

            // Check if position belongs to player car
            int playerX = 1 + playerLane * LANE_WIDTH;
            int isPlayerChar = (y >= SCREEN_HEIGHT - 3 && y < SCREEN_HEIGHT && x >= playerX && x < playerX + 4 && ch != ' ' && ch != '.');

            // Check if position belongs to enemy car
            int enemyX = 1 + enemyLane * LANE_WIDTH;
            int isEnemyChar = (enemyY >= 0 ? (y >= enemyY && y < enemyY + 3) : (y >= 0 && y < enemyY + 3)) && (x >= enemyX && x < enemyX + 4 && ch != ' ' && ch != '.');

            if (isPlayerChar) {
                printf(COLOR_PLAYER "%c" COLOR_RESET, ch);
            } else if (isEnemyChar) {
                printf(COLOR_ENEMY "%c" COLOR_RESET, ch);
            } else if (ch == '|' || ch == '.') {
                printf(COLOR_ROAD "%c" COLOR_RESET, ch);
            } else {
                putchar(ch);
            }
        }
        printf("\n");
    }
    printf("\nControls: " COLOR_TEXT "'a'" COLOR_RESET " (Left), " COLOR_TEXT "'d'" COLOR_RESET " (Right), " COLOR_TEXT "'q'" COLOR_RESET " (Quit)\n");
}

int checkCollision() {
    // Both cars occupy 3 rows; collision happens if enemy overlaps player lane
    if (enemyLane == playerLane && (enemyY + 2 >= SCREEN_HEIGHT - 3)) {
        return 1;
    }
    return 0;
}

void runGame() {
    srand((unsigned int)time(NULL));
    score = 0;
    playerLane = 2;
    enemyLane = rand() % NUM_LANES;
    enemyY = -6;
    gameSpeed = 90;

    system("cls");

    while (1) {
        // Non-blocking key handling
        if (kbhit()) {
            char ch = getch();
            if ((ch == 'a' || ch == 'A') && playerLane > 0) {
                playerLane--;
                Beep(1200,30);
            } else if ((ch == 'd' || ch == 'D') && playerLane < NUM_LANES - 1) {
                playerLane++;
                Beep(1200,30);
            } else if (ch == 'q' || ch == 'Q') {
                break;
            }
        }

        // Enemy movement step
        enemyY++;

        // Reset enemy when it leaves screen
        if (enemyY >= SCREEN_HEIGHT) {
            score += 10;
            saveHighScore();
            
            enemyY = 0;
            enemyLane = rand() % NUM_LANES;

            // Increase speed as score grows
            if (gameSpeed > 30) {
                gameSpeed -= 2;
            }
        }

        drawGame();

        if (checkCollision()) {
            Beep(200,400);
            printf("\n" COLOR_ENEMY "💥 CRASH! Game Over! 💥" COLOR_RESET "\n");
            printf("Final Score: %d\n", score);
            saveHighScore();
            break;
        }

        Sleep(gameSpeed);
    }
}

int main() {
    // Enable ANSI Color support in Windows Command Console
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    hideCursor();
    loadHighScore();

    printf("==========================================\n");
    printf("   WELCOME TO SPACIOUS HIGHWAY RACER!     \n");
    printf("==========================================\n");
    printf("High Score to beat: %d\n\n", highScore);
    printf("Press any key to start...");
    getch();

    runGame();

    printf("\nPress any key to exit.\n");
    getch();
    return 0;
}
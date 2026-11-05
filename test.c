#include <stdio.h>
#include <stdlib.h>
#include <conio.h> // For kbhit() and getch() - Common on Windows/MinGW
#include <time.h>
#include <windows.h> // For Sleep() - Use unistd.h and usleep() on Linux/macOS

#define SCREEN_HEIGHT 20
#define ROAD_WIDTH 3
#define CAR_CHAR 'C'
#define OBSTACLE_CHAR 'X'
#define ROAD_CHAR '|'

int carPosition = 1; // 0, 1, or 2 (three lanes)
int obstaclePosition = -1; // -1 means no obstacle, 0, 1, or 2 for a lane
int obstacleY = 1;
int score = 0;

// Function to clear the console screen
void clearScreen() {
    // system("cls") is common on Windows. Use system("clear") for Linux/macOS.
    system("cls"); 
}

// Function to draw the current game state
void drawGame() {
    clearScreen();
    printf("🚗 Console Car Game\n");
    printf("Score: %d\n\n", score);

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < ROAD_WIDTH; x++) {
            if (y == SCREEN_HEIGHT - 2 && x == carPosition) {
                printf("%c", CAR_CHAR); // Draw the player car
            } else if (y == obstacleY && x == obstaclePosition) {
                printf("%c", OBSTACLE_CHAR); // Draw the obstacle
            } else {
                printf(" "); // Empty space
            }
        }
        printf("%c\n", ROAD_CHAR); // Draw right road edge
    }

    printf("\nControls: 'a' (Left), 'd' (Right), 'q' (Quit)\n");
}

// Function to check for collision
int checkCollision() {
    // Collision happens if the obstacle is on the same row as the car
    // AND in the same column/lane.
    if (obstacleY == SCREEN_HEIGHT - 2 && obstaclePosition == carPosition) {
        return 1; // Collision detected
    }
    return 0; // No collision
}

// Main game logic function
void runGame() {
    srand(time(NULL)); // Seed the random number generator
    score = 0;
    carPosition = 1;
    obstaclePosition = -1;
    obstacleY = 1;

    // Game Loop
    while (1) {
        // --- Input Handling ---
        if (kbhit()) {
            char input = getch();
            if (input == 'a' && carPosition > 0) {
                carPosition--; // Move left
            } else if (input == 'd' && carPosition < ROAD_WIDTH - 1) {
                carPosition++; // Move right
            } else if (input == 'q') {
                break; // Quit the game
            }
        }

        // --- Game State Update (Obstacle Movement) ---
        obstacleY++; // Move obstacle down
        
        // Obstacle passed the screen or was newly generated
        if (obstacleY >= SCREEN_HEIGHT) {
            if (obstaclePosition != -1) {
                score += 10; // Only add score if an obstacle was successfully dodged
            }
            
            // Generate a new obstacle
            obstacleY = 1;
            // Generate a random lane (0, 1, or 2)
            obstaclePosition = rand() % ROAD_WIDTH; 
        }

        // --- Draw Game ---
        drawGame();

        // --- Collision Check ---
        if (checkCollision()) {
            printf("\n💥 Game Over! You crashed! 💥\n");
            printf("Final Score: %d\n", score);
            break;
        }

        // --- Game Speed Control ---
        Sleep(100); // Pause for 100 milliseconds (adjust for speed)
    }
}

int main() {
    printf("Press any key to start the car dodging game...\n");
    getch();
    
    runGame();

    printf("\nPress any key to exit.\n");
    getch();
    return 0;
}
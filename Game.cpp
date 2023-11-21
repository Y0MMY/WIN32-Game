#include "Engine.h"
#include <stdlib.h>
#include <memory.h>
#include <windows.h>
#include <random>
#include <algorithm>

// Colors
constexpr uint32_t COLORS[] =
{
	0xFFFFFF, 0x00FF00, 0xFF0000, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF, 0x800000, 0x008000, 0x000080,
	0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080, 0xFF8080, 0xFFFF80, 0x80FF80, 0x80FFFF, 0x0080FF,
	0x800000, 0x800080, 0x008080, 0x808000, 0x808080, 0xFF0000, 0xFF00FF, 0x00FF00, 0x00FFFF, 0x0000FF,
	0x800000, 0x800080, 0x008080, 0x808000, 0x808080, 0xFF8000, 0xFF80FF, 0x80FF00, 0x80FF80, 0x80FFFF,
};

enum COLORS_TYPE
{
	White = 0,
	Green = 1,
	Red = 2,
	Blue = 3,
	Yellow = 4,
	Magenta = 5,
	Cyan = 6,
	DarkRed = 7,
	DarkGreen = 8,
	DarkBlue = 9,
	Olive = 10,
	Purple = 11,
	Teal = 12,
	Silver = 13,
	Gray = 14,
	LightRed = 15,
	LightYellow = 16,
	LightGreen = 17,
	LightCyan = 18,
	LightBlue = 19,
	
};

// Data
constexpr int PLATFORM_WIDTH = 100;
constexpr int PLATFORM_HEIGHT = 10;
constexpr int BALL_RADIUS = 10;
constexpr int BRICK_WIDTH = 30;
constexpr int BRICK_HEIGHT = 30;
constexpr int NUM_BRICKS = 15;
constexpr int PLATFORM_SPEED = 1;

uint32_t ColorIndex = 0;
uint32_t ActualColorBrick = -1;

unsigned BricksAlive = 0;

bool isRestart = false;

struct Platform {
	int x;
	int y;
};

struct Ball {
	float x;
	float y;
	float dx;
	float dy;
};

struct Brick {
	int x;
	int y;
	bool destroyed;
};

Platform platform;
Ball ball;
Brick bricks[NUM_BRICKS];


// initialize game data in this function
void initialize()
{
	// Initialize platform
	platform.x = SCREEN_WIDTH / 2 - PLATFORM_WIDTH / 2;
	platform.y = SCREEN_HEIGHT - PLATFORM_HEIGHT - 10;

	// Initialize ball
	ball.x = SCREEN_WIDTH / 2;
	ball.y = SCREEN_HEIGHT / 2;
	ball.dx = 0.3f;  
	ball.dy = -0.3f; 

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> xDist(0, SCREEN_WIDTH - BRICK_WIDTH);
	std::uniform_int_distribution<int> yDist(0, SCREEN_HEIGHT / 2);

	for (int i = 0; i < NUM_BRICKS; i++) {
		bricks[i].x = xDist(gen);
		bricks[i].y = yDist(gen);
		bricks[i].destroyed = false;
	}
	
	BricksAlive = NUM_BRICKS;
	ActualColorBrick = COLORS[Green];
	ColorIndex = 0;
}

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt)
{
	if (ColorIndex >= sizeof(COLORS) / sizeof(uint32_t))
		ColorIndex = 0;
	if (isRestart)
	{
		isRestart = false;
		Sleep(2000);
	}
	
	if (!BricksAlive)
	{
		isRestart = true;
		initialize();
	}

	if (is_key_pressed(VK_ESCAPE))
	schedule_quit_game();

	// Move the platform
	if (is_key_pressed(VK_LEFT) && platform.x > 0) {
		platform.x -= PLATFORM_SPEED;
	}
	if (is_key_pressed(VK_RIGHT) && platform.x < SCREEN_WIDTH - PLATFORM_WIDTH) {
		platform.x += PLATFORM_SPEED;
	}

	// Move the ball
	ball.x += ball.dx;
	ball.y += ball.dy;

	if (ball.x <= 0 || ball.x + BALL_RADIUS >= SCREEN_WIDTH )
	{
		ball.dx = -ball.dx;
	}
	if (ball.y <= 0) {
		ball.dy = -ball.dy;
	}

	// Check collision with platform
	if (ball.y + BALL_RADIUS >= platform.y && ball.y - BALL_RADIUS <= platform.y + PLATFORM_HEIGHT &&
		ball.x + BALL_RADIUS >= platform.x && ball.x - BALL_RADIUS <= platform.x + PLATFORM_WIDTH) {
		// Calculate collision depth on each axis
		float collisionX = (std::abs(ball.x - platform.x - PLATFORM_WIDTH / 2) < (float)(PLATFORM_WIDTH / 2)) ? std::abs(ball.x - platform.x - PLATFORM_WIDTH / 2) : (float)(PLATFORM_WIDTH / 2);
		float collisionY = (std::abs(ball.y - platform.y - PLATFORM_HEIGHT / 2) < (float)(PLATFORM_HEIGHT / 2)) ? std::abs(ball.y - platform.y - PLATFORM_HEIGHT / 2) : (float)(PLATFORM_HEIGHT / 2);

		// Determine the side of collision
		if (collisionX < collisionY) {
			// Collided from the left or right side
			ball.dx = -ball.dx;
		}
		else {
			// Collided from the top or bottom side
			ball.dy = -ball.dy;
		}
	}

	// Check collisions with bricks
	for (int i = 0; i < NUM_BRICKS; i++) {
		Brick& brick = bricks[i];
		if (!brick.destroyed &&
			ball.y + BALL_RADIUS >= brick.y && ball.y - BALL_RADIUS <= brick.y + BRICK_HEIGHT &&
			ball.x + BALL_RADIUS >= brick.x && ball.x - BALL_RADIUS <= brick.x + BRICK_WIDTH) {
			// Calculate collision depth on each axis
			float collisionX = (std::abs(ball.x - brick.x - BRICK_WIDTH / 2) < BRICK_WIDTH / 2) ? std::abs(ball.x - brick.x - BRICK_WIDTH / 2) : BRICK_WIDTH / 2;
			float collisionY = (std::abs(ball.y - brick.y - BRICK_HEIGHT / 2) < BRICK_HEIGHT / 2) ? std::abs(ball.y - brick.y - BRICK_HEIGHT / 2) : BRICK_HEIGHT / 2;

			// Determine the side of collision
			if (collisionX < collisionY) {
				// Collided from the left or right side
				ball.dx = -ball.dx;
			}
			else {
				// Collided from the top or bottom side
				ball.dy = -ball.dy;
			}

			brick.destroyed = true;
			ActualColorBrick = COLORS[++ColorIndex];
			BricksAlive--;
		}
	}

	// Check if the ball falls off the bottom side
	if (ball.y > SCREEN_HEIGHT)
	{
		isRestart = true;
		initialize();
	}
}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw()
{
	// clear backbuffer
	memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));

	// Draw the platform
	for (int y = 0; y < PLATFORM_HEIGHT; y++) 
	{
		for (int x = 0; x < PLATFORM_WIDTH; x++) 
		{
			buffer[platform.y + y][platform.x + x] = COLORS[White];
		}
	}
	
	// Draw the bricks
	for (int i = 0; i < NUM_BRICKS; i++)
	{
		if (!bricks[i].destroyed)
		{
			for (int y = 0; y < BRICK_HEIGHT; y++)
			{
				for (int x = 0; x < BRICK_WIDTH; x++)
				{
					buffer[bricks[i].y + y][bricks[i].x + x] = ActualColorBrick;
				}
			}
		}
	}

	// Draw the ball
	for (int y = -BALL_RADIUS; y <= BALL_RADIUS; y++) 
	{
		for (int x = -BALL_RADIUS; x <= BALL_RADIUS; x++) 
		{
			if (x * x + y * y <= BALL_RADIUS * BALL_RADIUS) 
			{
				int pixelX = (int)ball.x + x;
				int pixelY = (int)ball.y + y;
				if (pixelX >= 0 && pixelX < SCREEN_WIDTH && pixelY >= 0 && pixelY < SCREEN_HEIGHT)
				{
					buffer[pixelY][pixelX] = COLORS[Red];
				}
			}
		}
	}
}

// free game data in this function
void finalize()
{
}


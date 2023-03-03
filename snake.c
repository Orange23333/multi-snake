#include <conio.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define __WINDOWS__
// #define __LINUX__

#define MAP_WIDTH 16
#define MAP_HEIGHT 8
#define MAP_HAS_WALL false

#define STEP_PER_SECOND 1

// Map backgrund filling character.
#define MAP_BG_CHAR '+'

#if defined(__WINDOWS__)
#if defined(__LINUX__)
#error "Can not understand platform."
#endif
#elif defined(__LINUX__)
// ...
#elif
#error "Can not understand platform."
#endif

#if defined(__WINDOWS__)
#include <windows.h>
#elif
#include <unistd.h>
#endif

// Get 3D pointer.
#define G3D(a, w, h, x, y, z) (a + ((z * h + y) * w + x))
// Get 2D pointer.
#define G2D(a, w, x, y) (a + y * w + x)
// Get random number.
#define GRN(min, max) ((rand() + min) % (max - min + 1))

// Map transparent character.
#define MAP_TSPRT_CHAR '\0'

// Diretions:
#define DRCT_UP '^'
#define DRCT_DOWN 'v'
#define DRCT_LEFT '<'
#define DRCT_RIGHT '>'
#define DRCT_NONE '\0'

// Null snake body index.
#define SBI_NULL 0

int mapW = 0, mapH = 0; // Map width and height.
char *bg = NULL;		// Background.
char *sbd = NULL;		// Snake body direcions.
int *sbi = NULL;		// Snake body indexes.
int score = 0, frameCount = 0;
int foodX = -1, foodY = -1;
int snakeX = -1, snakeY = -1, snakeL = 0;

// Get background pointer.
inline char *gbg(const int x, const int y);
// Get snake body direction pointer.
inline char *gsbd(const int x, const int y);
// Get snake body index pointer.
inline int *gsbi(const int x, const int y);
inline int GetMapSize();
void ResizeMap(const int width, const int height, const char background);
void ResetGame(int width, int height, const char background);
void ResetSnake();
void RandomFood();
void PrintMap();
void PrintFrameInfo();

int main()
{
	const int stepPS = STEP_PER_SECOND;
#if defined(__WINDOWS__)
	const int delayMs = 50;
	const int delayTimes = (1000.0 / stepPS) / delayMs;
#elif
	const int delayUs = 50000;
	const int delayTimes = (1000000.0 / stepPS) / delayUs;
#endif

	int delayCount;
	char input;
	char lastDrct;
	bool deathFlag;

	while (true)
	{
		delayCount = 0;
		input = '\0';
		lastDrct = DRCT_NONE;
		deathFlag = false;

		ResetGame(MAP_WIDTH, MAP_HEIGHT, MAP_BG_CHAR);

		// When snakeL>mapSize the game will be end.
		// That also means although the snake fills screen, the game wont' end.

		PrintMap();
		PrintFrameInfo();

		while (true)
		{
			if (kbhit())
			{
				input = getch();
				putchar(input);
			}

			if (delayCount >= delayTimes)
			{
				if (input != '\0')
				{
					if (input == 'a')
					{
						*gsbd(snakeX, snakeY) = DRCT_LEFT;
					}
					else if (input == 'd')
					{
						*gsbd(snakeX, snakeY) = DRCT_RIGHT;
					}
					else if (input == 'w')
					{
						*gsbd(snakeX, snakeY) = DRCT_UP;
					}
					else if (input == 's')
					{
						*gsbd(snakeX, snakeY) = DRCT_DOWN;
					}
					input = '\0';
					putchar('\n');
				}

				lastDrct = *gsbd(snakeX, snakeY);
				if (lastDrct == DRCT_LEFT)
				{
					snakeX--;
				}
				else if (lastDrct == DRCT_RIGHT)
				{
					snakeX++;
				}
				else if (lastDrct == DRCT_UP)
				{
					snakeY--;
				}
				else if (lastDrct == DRCT_DOWN)
				{
					snakeY++;
				}

				if (MAP_HAS_WALL)
				{
					//Hit wall!
					if (snakeX < 0 || snakeX > mapW - 1 || snakeY < 0 || snakeY > mapH - 1)
					{
						deathFlag=true;
					}
				}
				else
				{
					if (snakeX < 0)
					{
						snakeX = mapW - 1;
					}
					else if (snakeX > mapW - 1)
					{
						snakeX = 0;
					}
					else if (snakeY < 0)
					{
						snakeY = mapH - 1;
					}
					else if (snakeY > mapH - 1)
					{
						snakeY = 0;
					}
				}

				if(*gsbi(snakeX,snakeY)!=SBI_NULL){
					//Bit itself.
					deathFlag=true;
				}

				if (snakeX == foodX && snakeY == foodY)
				{
					score++;
					snakeL++;
					RandomFood();
				}

				delayCount = 0;

				PrintMap();
				PrintFrameInfo();
				frameCount++;

				if(deathFlag){
					break;
				}
			}
#if defined(__WINDOWS__)
			Sleep(delayMs);
#elif
			Sleep(delayUs);
#endif
			delayCount++;
		}
		printf("Game Over! (score=%d)\n", score);
		fflush(stdin);
		printf("Press ENTER to start a new game...");
		getchar();
		fflush(stdin);
	}

	return 0;
}

// Get background pointer.
char *gbg(const int x, const int y)
{
	return G2D(bg, mapW, x, y);
}

// Get snake body direction pointer.
char *gsbd(const int x, const int y)
{
	return G2D(sbd, mapW, x, y);
}

// Get snake body index pointer.
int *gsbi(const int x, const int y)
{
	return G2D(sbi, mapW, x, y);
}

int GetMapSize()
{
	return mapW * mapH;
}

void ResizeMap(const int width, const int height, const char background)
{
	int newSize = width * height;

	if (newSize != GetMapSize())
	{
		if (bg != NULL)
		{
			free(bg);
		}
		if (sbd != NULL)
		{
			free(sbd);
		}
		if (sbi != NULL)
		{
			free(sbi);
		}

		bg = (char *)malloc(sizeof(char) * newSize);
		sbd = (char *)malloc(sizeof(char) * newSize);
		sbi = (int *)malloc(sizeof(int) * newSize);

		mapW = width;
		mapH = height;
	}

	memset(bg, background, sizeof(char) * newSize);
	// memset(sbd,DRCT_NONE,sizeof(char)*newSize);
	// memset(sbi,SBI_NULL,sizeof(int)*newSize);
}

void ResetGame(int width, int height, const char background)
{
	score = 0;
	frameCount = 0;

	ResizeMap(width, height, background);

	srand(time(NULL));
	ResetSnake();
	RandomFood();
}

void ResetSnake()
{
	int mapSize = GetMapSize();

	snakeX = GRN(0, mapW - 1);
	snakeY = GRN(0, mapH);

	memset(sbd, DRCT_NONE, sizeof(char) * mapSize);
	memset(sbi, SBI_NULL, sizeof(int) * mapSize);

	*gsbi(snakeX, snakeY) = 1;
	snakeL = 1;
}

void RandomFood()
{
	do
	{
		// Never mind food will generate at last postion because the snake head will take it up.
		foodX = GRN(0, mapW - 1);
		foodY = GRN(0, mapH - 1);
	} while (*gsbi(foodX, foodY) != SBI_NULL);
}

void PrintMap()
{
	int x, y;
	int bodyIndex;

	for (y = 0; y < mapH; y++)
	{
		for (x = 0; x < mapW; x++)
		{
			bodyIndex = *gsbi(x, y);
			if (bodyIndex != SBI_NULL)
			{
				if (bodyIndex == 1)
				{
					putchar('X'); // Or snakeX,snakeY = x,y.
				}
				else
				{
					putchar(bodyIndex % 2 == 0 ? '/' : '\\');
				}
			}
			else if (x == foodX && y == foodY)
			{
				putchar('O');
			}
			else
			{
				putchar(*gbg(x, y));
			}
		}
		putchar('\n');
	}
}

void PrintFrameInfo()
{
	printf(
		"{map{w=%d,h=%d},snake{x=%d,y=%d,d='%d'},food{x=%d,y=%d},score=%d,frameCount=%d}\n",
		mapW, mapH,
		snakeX, snakeY, *gsbd(snakeX, snakeY),
		foodX, foodY,
		score,
		frameCount);
}
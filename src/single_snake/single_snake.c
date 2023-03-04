#include <conio.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//vvvvvvvvEDITABLE ZONEvvvvvvvv
#define MAP_WIDTH 16
#define MAP_HEIGHT 8
#define MAP_HAS_WALL true

#define STEP_PER_SECOND 4
#define STEP_PER_MIN 1
#define STEP_PER_MAX 8

// Map backgrund filling character.
#define MAP_BG_CHAR '+'
//^^^^^^^^EDITABLE ZONE^^^^^^^^

#define SCAN_DELAY_MS 50

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

// struct timespec delayCache={0,0};

int stepPS = STEP_PER_SECOND;
int mapW = 0, mapH = 0; // Map width and height.
char *bg = NULL;		// Background.
char *sbd = NULL;		// Snake body direcions.
int *sbi = NULL;		// Snake body indexes. This use performance to get better animation.
int score = 0, frameCount = 0;
struct timespec lastFrameTime = {0, 0}, deltaFrameTime = {0, 0};
int foodX = -1, foodY = -1;
int snakeX = -1, snakeY = -1, snakeL = 0;
// int tailX=-1, tailY=-1; //Use tail to acclerate remove tail of snake body.

// Get background pointer.
inline char *gbg(const int x, const int y);
// Get snake body direction pointer.
inline char *gsbd(const int x, const int y);
// Get snake body index pointer.
inline int *gsbi(const int x, const int y);
void UpdateSBD(const int x, const int y, const bool eatenFood, const char lastDrct);
void UpdateSBI(const int x, const int y, const bool eatenFood);
inline int GetMapSize();
void SetGameOptions();
void SaveRecord();
void ResetGame(int width, int height, const char background);
void ResizeMap(const int width, const int height, const char background);
void ResetSnake();
void RandomFood();
void PrintMap();
void PrintFrameInfo();
void Delay(const int delayMs, const bool resetDelayCache);
struct timespec DiffTimespec(const struct timespec x, const struct timespec y); // return y-x.
struct timespec AddTimespec(const struct timespec x, const struct timespec y);
struct timespec RealTimespec(const struct timespec x);
bool IsOverTimespec(const struct timespec last, const struct timespec now, const int delayMs);

int main()
{
	int delayMs;
	int delayTimes;

	int delayCount;
	char input;
	char lastDrct;
	bool deathFlag, eatenFoodFlag;

	struct timespec ts_temp;

	while (true)
	{
		SetGameOptions();
		delayMs = SCAN_DELAY_MS;
		delayTimes = (1000.0 / stepPS) / delayMs;

		delayCount = 0;
		input = '\0';
		lastDrct = DRCT_NONE;
		deathFlag = false;
		eatenFoodFlag = false;

		ResetGame(MAP_WIDTH, MAP_HEIGHT, MAP_BG_CHAR);

		// When snakeL>mapSize the game will be end.
		// That also means although the snake fills screen, the game wont' end.

		PrintMap();
		PrintFrameInfo();
		puts("Press WASD to change your direction.");

		fflush(stdin);
		do
		{
			input = getch();
			// putchar(input);
		} while (strchr("wasd", input) == NULL);
		// putchar('\n');

		Delay(0, true);

		while (true)
		{
			if (kbhit())
			{
				input = getch();
				// putchar(input);
			}

			if (delayCount >= delayTimes)
			{
				if (input != '\0')
				{
					if (input == 'a' && lastDrct != DRCT_RIGHT)
					{
						*gsbd(snakeX, snakeY) = lastDrct = DRCT_LEFT;
					}
					else if (input == 'd' && lastDrct != DRCT_LEFT)
					{
						*gsbd(snakeX, snakeY) = lastDrct = DRCT_RIGHT;
					}
					else if (input == 'w' && lastDrct != DRCT_DOWN)
					{
						*gsbd(snakeX, snakeY) = lastDrct = DRCT_UP;
					}
					else if (input == 's' && lastDrct != DRCT_UP)
					{
						*gsbd(snakeX, snakeY) = lastDrct = DRCT_DOWN;
					}
					input = '\0';
					// putchar('\n');
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
					// Hit wall!
					if (snakeX < 0 || snakeX > mapW - 1 || snakeY < 0 || snakeY > mapH - 1)
					{
						deathFlag = true;
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

				if (snakeX == foodX && snakeY == foodY)
				{
					eatenFoodFlag = true;
				}

				if (*gsbi(snakeX, snakeY) != SBI_NULL && *gsbi(snakeX, snakeY) != snakeL)
				{
					// Bit itself.
					deathFlag = true;
				}

				if (eatenFoodFlag)
				{
					score++;
					UpdateSBI(snakeX, snakeY, true);
					UpdateSBD(snakeX, snakeY, true, lastDrct);
					snakeL++;
					RandomFood();

					eatenFoodFlag = false;
				}
				else
				{
					UpdateSBI(snakeX, snakeY, false);
					UpdateSBD(snakeX, snakeY, false, lastDrct);
				}

				delayCount = 0;

				clock_gettime(CLOCK_REALTIME, &ts_temp);
				deltaFrameTime = DiffTimespec(lastFrameTime, ts_temp);
				lastFrameTime = ts_temp;
				//clrscr();
				//system("cls");
				PrintMap();
				PrintFrameInfo();
				frameCount++;

				if (deathFlag)
				{
					break;
				}
			}

			Delay(delayMs, false);

			delayCount++;
		}
		SaveRecord();
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

void UpdateSBD(const int x, const int y, const bool eatenFood, const char lastDrct)
{
	char *p, *pEnd;

	// No matter with whether p could add to INT_MAX.
	for (p = sbd, pEnd = sbd + GetMapSize(); p < pEnd; p++)
	{
		if (*p != MAP_TSPRT_CHAR)
		{
			if (!eatenFood && *p == snakeL)
			{
				*p = MAP_TSPRT_CHAR;
			}
		}
	}

	*gsbd(x, y) = lastDrct;
}

void UpdateSBI(const int x, const int y, const bool eatenFood)
{
	int *p, *pEnd;

	// No matter with whether p could add to INT_MAX.
	for (p = sbi, pEnd = sbi + GetMapSize(); p < pEnd; p++)
	{
		if (*p != SBI_NULL)
		{
			if (!eatenFood && *p == snakeL)
			{
				*p = SBI_NULL;
			}
			else
			{
				(*p)++;
			}
		}
	}

	*gsbi(x, y) = 1;
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
		if (bg == NULL)
		{
			printf("Out of memory!");
			exit(-1);
		}
		sbd = (char *)malloc(sizeof(char) * newSize);
		if (sbd == NULL)
		{
			printf("Out of memory!");
			exit(-1);
		}
		sbi = (int *)malloc(sizeof(int) * newSize);
		if (sbi == NULL)
		{
			printf("Out of memory!");
			exit(-1);
		}

		mapW = width;
		mapH = height;
	}

	memset(bg, background, sizeof(char) * newSize);
	// memset(sbd,DRCT_NONE,sizeof(char)*newSize);
	// memset(sbi,SBI_NULL,sizeof(int)*newSize);
}

void SetGameOptions(){
	int _stepPs;

	do{
		printf("Step of snake per second (%d~%d): ",STEP_PER_MIN,STEP_PER_MAX);
		fflush(stdin);
		scanf("%d",&_stepPs);
	}while(_stepPs<STEP_PER_MIN||_stepPs>STEP_PER_MAX);
}

void SaveRecord(){
	FILE* fp = fopen(".single_snake_record.txt", "a");
	if(fp==NULL){
		printf("Can't open or create record file.");
		exit(-1);
	}
	fprintf(fp,"name=\"\",map(w=%d,h=%d),sps=%d,l=%d,score=%d",mapW,mapH,stepPS,snakeL,score);
	fclose(fp);
}

void ResetGame(int width, int height, const char background)
{
	score = 0;
	frameCount = 0;
	clock_gettime(CLOCK_REALTIME, &lastFrameTime);
	deltaFrameTime.tv_sec = 0;
	deltaFrameTime.tv_nsec = 0;

	ResizeMap(width, height, background);

	srand(time(NULL));
	ResetSnake();
	RandomFood();
}

void ResetSnake()
{
	int mapSize = GetMapSize();

	snakeX = GRN(0, mapW - 1);
	snakeY = GRN(0, mapH - 1);

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
	static int anim_count = 0;
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
					putchar((bodyIndex + anim_count) % 2 == 0 ? '/' : '\\');
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

	anim_count++;
	if (anim_count >= 2)
	{
		anim_count = 0;
	}
}

void PrintFrameInfo()
{
	printf(
		"{map{w=%d,h=%d},snake{x=%d,y=%d,l=%d,d='%c'},food{x=%d,y=%d},score=%d,frameCount=%d,deltaFrameTime{sec=%lld,ns=%d}}\n",
		mapW, mapH,
		snakeX, snakeY, snakeL, *gsbd(snakeX, snakeY),
		foodX, foodY,
		score,
		frameCount,
		deltaFrameTime.tv_sec, deltaFrameTime.tv_nsec);
}

void Delay(const int delayMs, const bool resetDelayCache)
{
	static struct timespec delayCache = {0, 0};
	struct timespec ts_now, ts_delay;

	// timespec_get(&ts, TIME_UTC); //https://en.cppreference.com/w/c/chrono/timespec
	clock_gettime(CLOCK_REALTIME, &ts_now);
	if (resetDelayCache)
	{
		delayCache = ts_now;
	}

	while (!IsOverTimespec(delayCache, ts_now, delayMs))
	{
		clock_gettime(CLOCK_REALTIME, &ts_now);
		// printf("{last{sec=%lld,ns=%d},now{sec=%lld,ns=%d}}\n",delayCache.tv_sec,delayCache.tv_nsec,ts_now.tv_sec,ts_now.tv_nsec);

		// If use sleep function to save CPU time need to specify OS type.
	}

	ts_delay.tv_sec = delayMs / 1000;
	ts_delay.tv_nsec = delayMs % 1000 * 1000000;

	delayCache = AddTimespec(delayCache, ts_delay);
}

struct timespec DiffTimespec(const struct timespec x, const struct timespec y)
{ // return y-x.
	struct timespec diff;

	diff.tv_nsec = y.tv_nsec - x.tv_nsec;
	diff.tv_sec = y.tv_sec - x.tv_sec;

	return RealTimespec(diff);
}

struct timespec AddTimespec(const struct timespec x, const struct timespec y)
{
	struct timespec result;

	result.tv_nsec = y.tv_nsec + x.tv_nsec;
	result.tv_sec = y.tv_sec + x.tv_sec;

	return RealTimespec(result);
}

struct timespec RealTimespec(const struct timespec x)
{
	struct timespec r = x;

	if (r.tv_sec > 0)
	{
		if (r.tv_nsec >= 1000000000)
		{
			do
			{
				r.tv_sec++;
				r.tv_nsec -= 1000000000;
			} while (r.tv_nsec >= 1000000000);
		}
		else if (r.tv_nsec < 0)
		{
			do
			{
				r.tv_sec--;
				r.tv_nsec += 1000000000;
			} while (r.tv_nsec < 0 && r.tv_sec > 0);
		}
	}
	else if (r.tv_sec < 0)
	{
		if (r.tv_nsec <= -1000000000)
		{
			do
			{
				r.tv_sec--;
				r.tv_nsec += 1000000000;
			} while (r.tv_nsec <= -1000000000);
		}
		else if (r.tv_nsec > 0)
		{
			do
			{
				r.tv_sec++;
				r.tv_nsec -= 1000000000;
			} while (r.tv_nsec > 0 && r.tv_sec < 0);
		}
	}
	else
	{
		if (r.tv_nsec >= 1000000000)
		{
			do
			{
				r.tv_sec++;
				r.tv_nsec -= 1000000000;
			} while (r.tv_nsec >= 1000000000);
		}
		else if (r.tv_nsec <= -1000000000)
		{
			do
			{
				r.tv_sec--;
				r.tv_nsec += 1000000000;
			} while (r.tv_nsec <= -1000000000);
		}
	}

	return r;
}

bool IsOverTimespec(const struct timespec last, const struct timespec now, const int delayMs)
{
	struct timespec ts_delay = {delayMs / 1000, delayMs % 1000 * 1000000};
	struct timespec diffTimespec = DiffTimespec(ts_delay, DiffTimespec(last, now));

	return diffTimespec.tv_sec > 0 || (diffTimespec.tv_sec == 0 && diffTimespec.tv_nsec >= 0);
}
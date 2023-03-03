#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

//Get 3D pointer.
#define G3D(a,w,h,x,y,z) (a+((z*h+y)*w+x))
//Get 2D pointer.
#define G2D(a,w,x,y) (a+y*w+x)
//Get random number.
#define GRN(min,max) ((rand()+min)%(max-min+1))

#define MAP_WIDTH	16
#define MAP_HEIGHT	8

//Map transparent character.
#define MAP_TSPRT_CHAR	'\0'
//Map backgrund filling character.
#define MAP_BG_CHAR		'+'

//Diretions:
#define DRCT_UP		'^'
#define DRCT_DOWN	'v'
#define DRCT_LEFT	'<'
#define DRCT_RIGHT	'>'
#define DRCT_NONE	'\0'

//Null snake body index.
#define SBI_NULL	0

int mapW=0, mapH=0; //Map width and height.
char *bg=NULL; //Background.
char *sbd=NULL; //Snake body direcions.
int *sbi=NULL; //Snake body indexes.
int score=0, frameCount=0;
int foodX=-1, foodY=-1;
int snakeX=-1, snakeY=-1, snakeL=0;

//Get background pointer.
inline char* gbg(const int x, const int y);
//Get snake body direction pointer.
inline char* gsbd(const int x, const int y);
//Get snake body index pointer.
inline int* gsbi(const int x, const int y);
inline int GetMapSize();
void ResizeMap(const int width, const int height, const char background);
void ResetGame(int width, int height, const char background);
void ResetSnake();
void RandomFood();
void PrintMap();
void PrintFrameInfo();

int main(){
	ResetGame(MAP_WIDTH,MAP_HEIGHT,MAP_BG_CHAR);

	// When snakeL>mapSize the game will be end.
	// That also means although the snake fills screen, the game wont' end.

	return 0;
}

//Get background pointer.
inline char* gbg(const int x, const int y){
	return G2D(bg,mapW,x,y);
}

//Get snake body direction pointer.
inline char* gsbd(const int x, const int y){
	return G2D(sbd,mapW,x,y);
}

//Get snake body index pointer.
inline int* gsbi(const int x, const int y){
	return G2D(sbi,mapW,x,y);
}

inline int GetMapSize(){
	return mapW*mapH;
}

void ResizeMap(const int width, const int height, const char background){
	int newSize=width*height;

	if(newSize!=GetMapSize()){
		if(bg!=NULL){
			free(bg);
		}
		if(sbd!=NULL){
			free(sbd);
		}
		if(sbi!=NULL){
			free(sbi);
		}

		bg=(char*)malloc(sizeof(char)*newSize);
		sbd=(char*)malloc(sizeof(char)*newSize);
		sbi=(int*)malloc(sizeof(int)*newSize);

		mapW=width;
		mapH=height;
	}

	memset(bg,background,sizeof(char)*newSize);
	//memset(sbd,DRCT_NONE,sizeof(char)*newSize);
	//memset(sbi,SBI_NULL,sizeof(int)*newSize);
}


void ResetGame(int width, int height, const char background){
	score = 0;
	frameCount = 0;

	ResizeMap(width,height,background);

	srand(time(NULL));
	ResetSnake();
	RandomFood();
}

void ResetSnake(){
	int mapSize=GetMapSize();

	snakeX=GRN(0,mapW-1);
	snakeY=GRN(0,mapH);

	memset(sbd,DRCT_NONE,sizeof(char)*mapSize);
	memset(sbi,SBI_NULL,sizeof(int)*mapSize);

	*gsbi(snakeX,snakeY)=1;
	snakeL=1;
}

void RandomFood(){
	do{
		//Never mind food will generate at last postion because the snake head will take it up.
		foodX=GRN(0,mapW-1);
		foodY=GRN(0,mapH-1);
	}while(*gsbi(foodX,foodY)!=SBI_NULL);
}

void PrintMap(){
	int x,y;
	int bodyIndex;

	for(y=0;y<mapH;y++){
		for(x=0;x<mapW;x++){
			bodyIndex=*gsbi(x,y);
			if(bodyIndex != SBI_NULL) {
				if(bodyIndex == 1){
					putchar('X'); //Or snakeX,snakeY = x,y.
				}else{
					putchar(bodyIndex % 2 == 0 ? '/' : '\\');
				}
			}else if(x==foodX && y==foodY){
				putchar('O');
			}else{
				putchar(*gbg(x,y));
			}
		}
	}
}

void PrintFrameInfo(){
	printf(
		"{map{w=%d,h=%d},snake{x=%d,y=%d,d='%d'},food{x=%d,y=%d},score=%d,frameCount=%d}",
		mapW,mapH,
		snakeX,snakeY,*gsbd(snakeX,snakeY),
		foodX,foodY,
		score,
		frameCount
	);
}
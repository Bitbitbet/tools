#include<cstdlib>
#include<cstdio>
#include<cmath>

#define UP 0
#define DOWN 1
#define LEFT 0
#define RIGHT 1

#define PI 3.141592653

int x=0,y=0,direction[2]{DOWN,RIGHT};

int degree=0,radius=200;

int xo=0,yo=0;

void way_first();
void way_second();
void way_third();

int main(){
	int choice;
	char cmd[101]="xdotool mousemove ";
	void (*method[3])()={way_first,way_second,way_third};
	printf("选择移动方式0直线1圆圈2结合:");
	scanf("%d",&choice);if(choice==2)radius=50;
	while(true){
		sprintf(cmd+18,"%d %d",x>-1?x:0,y>-1?y:0);
		printf("\033[30D\033[1A\033[KMouse Position:%d %d\n",x,y);
		system(cmd);
		method[choice]();
	}
}

void way_first(){
	if(direction[1])x++;else x--;
	if(direction[0])y++;else y--;
	if(x==1366){
		x--;direction[1]=LEFT;
	}else if(x==-1){
		x++;direction[1]=RIGHT;
	}
	if(y==768){
		y--;direction[0]=UP;
	}else if(y==-1){
		y++;direction[0]=DOWN;
	}
}

void way_second(){
	x=(int)(cos(degree*PI/180)*(double)radius);
	y=(int)(sin(degree*PI/180)*(double)radius);
	x+=683;
	y+=384;
	degree++;
	if(degree==360)degree=0;
}

void way_third(){
	if(direction[1])xo++;else xo--;
	if(direction[0])yo++;else yo--;
	if(xo==1366){
		xo--;direction[1]=LEFT;
	}else if(xo==-1){
		xo++;direction[1]=RIGHT;
	}
	if(yo==768){
		yo--;direction[0]=UP;
	}else if(yo==-1){
		yo++;direction[0]=DOWN;
	}
	x=(int)(cos(degree*PI/180)*(double)radius);
	y=(int)(sin(degree*PI/180)*(double)radius);
	x+=xo;
	y+=yo;
	degree++;
	if(degree==360)degree=0;
}

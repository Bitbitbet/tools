#include<cstdio>
#include<cstdlib>
#include<unistd.h>

void start();
void stopwatch();

const int STOPWATCH=1;
const int EXIT=127;

int main(){
	printf("\033[3mClock tools\033[0m");
	while(true){
		start();
	}
}

void start(){
	int mode=0;
	bool vaild=true;
	char c;
	printf("\nSelect mode:\n-\t1.Stopwatch\n-\t127.Exit\n");
	while((c=getchar())!='\n'){
		if('0'<=c&&c<='9'){
			mode=mode*10+c-48;
		}else{
			vaild=false;
		}
	}
	if(!vaild){
		printf("Not a vaild option");
		return;
	}
	switch(mode){
		case STOPWATCH:
			stopwatch();
			break;
		case EXIT:
			exit(0);
			break;
		default:
			printf("%d is not a valid option",mode);
			return;
	}
}

void stopwatch(){
	int h=0,min=0,sec=0;
	putchar('\n');
	while(true){
		printf("\033[1A\033[100D%d:",h);
		if(min<10)putchar('0');
		printf("%d:",min);
		if(sec<10)putchar('0');
		printf("%d\n",sec);
		usleep(1000000);
		sec++;
		if(sec==60){
			sec=0;
			min++;
			if(min==60){
				min=0;
				h++;
			}
		}
	}
}

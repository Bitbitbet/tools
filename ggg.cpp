#include<cstdio>
#include<conio.h>

char c;
int stat;
bool output;

int main(int argc,char **argv){
	if(argc!=1){
		char *path=new char[10001];
		int index=0;
		for(int i=1;i<argc;i++){
			int pos=0;
			while(argv[i][pos]){
				path[index]=argv[i][pos];
				index++;
				pos++;
			}
			path[index]=' ';
			index++;
		}
		path[index-1]='\0';
		printf("Out put: %s\n",path);
		freopen(path,"w",stdout);
	}
	while(true){
		c=getch();
		output=true;
		switch(stat){
			case 0:
				if(c==27){
					stat++;
					output=false;
				}else if(c==127){
					printf("\033[1D \033[1D");
					output=false;
				}
				break;
			case 1:if(c==91){stat++;output=false;}else stat=0;break;
			case 2:
				switch(c){
					case 'A':printf("\033[1A");output=false;break;
					case 'B':printf("\033[1B");output=false;break;
					case 'C':printf("\033[1C");output=false;break;
					case 'D':printf("\033[1D");output=false;break;
					case 'F':printf("\033[200C");output=false;break;
					case 'H':printf("\033[200D");output=false;break;
					default:stat=0;
				}
				break;
		}
		if(output){
			putchar(c);
		}
	}
	return 0;
}

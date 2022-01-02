#include<cstdio>

int main(int argc,char **argv){
	for(int i=1;i<argc;i++){
		int num=0;//convert to integer
		bool nan=false;
		for(int index=0;argv[i][index];index++){
			if(argv[i][index]>='0'&&argv[i][index]<='9'){
				num+=argv[i][index]-'0';
				num*=10;
			}else{
				fprintf(stderr,"%s:\tNot a number\n",argv[i]);
				nan=true;
				break;
			}
		}
		num/=10;
		if(nan)continue;
		

	}
}

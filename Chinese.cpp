#include<cstdio>
#include<cstring>

int main(){
	char *a=new char[1000];
	scanf("%s",a);
	int count=0;
	for(int i=0;a[i];i++){
		int re=0xe0^a[i];
		if(re<=0xf&&re>=0x0){
			count++;
			printf("%c%c%c",a[i],a[i+1],a[i+2]);
			i+=2;
		}
	}
	printf("\n%d\n",count);
}

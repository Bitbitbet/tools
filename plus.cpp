#include<cstdio>
#include<string.h>

int main(){
	char a[200],b[200],c[201];
	memset(a,0,sizeof(a));
	memset(b,0,sizeof(b));
	memset(c,0,sizeof(c));
	int indexa,indexb,indexc=200;
	scanf("%s\n%s",a,b);
	for(indexa=199;!a[indexa];indexa--);
	for(indexb=199;!b[indexb];indexb--);
	while(indexa>=0||indexb>=0){
		if(indexa>=0)c[indexc]+=a[indexa]-'0';
		if(indexb>=0)c[indexc]+=b[indexb]-'0';
		if(c[indexc]>9){
			c[indexc]-=10;
			c[indexc-1]=1;
		}
		indexa--;indexb--;indexc--;
	}
	
	for(indexc=0;!c[indexc];indexc++);
	for(;indexc<201;indexc++)putchar(c[indexc]+'0');
	putchar('\n');
}

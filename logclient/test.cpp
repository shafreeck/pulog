#include <stdio.h>
#include <iostream>
using namespace std;

static void dump(void *msg , int len)
{
	FILE *dump = fopen("dump","w");
	fwrite(msg,len,1,dump);
	fclose(dump);

}
int main(int argc,char *argv[])
{
	time_t t = time(NULL);
	char stime[20]={0};
	int size = strftime(stime,sizeof(stime),"%Y-%m-%d %H:%M:%S",localtime(&t));
	cout<<stime<<endl;
	cout<<size <<endl;
	/*cout<<sizeof(int8_t)<<endl;
	uint32_t test = atoi(argv[1]);
	char buff[10]={0};
	memcpy(buff,&test,sizeof(uint32_t));
	dump(buff,sizeof(uint32_t ));*/
}

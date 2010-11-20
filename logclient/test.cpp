#include <stdio.h>
#include <iostream>
using namespace std;

static void dump(void *msg , int len)
{
	FILE *dump = fopen("dump","w");
	fwrite(msg,len,1,dump);
	fclose(dump);

}
void teststr(string msg)
{
	cout<<"string:"<<msg<<endl;
}
void teststr(char *msg)
{
	cout<<"char *:"<<msg<<endl;
}
int main(int argc,char *argv[])
{
/*	time_t t = time(NULL);
	char stime[20]={0};
	int size = strftime(stime,sizeof(stime),"%Y-%m-%d %H:%M:%S",localtime(&t));
	cout<<stime<<endl;
	cout<<size <<endl;*/
	teststr("hello");
	string s = "world";
	teststr(s);
}

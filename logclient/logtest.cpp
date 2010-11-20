#include <iostream>
#include <string>
#include "logclient.h"
using namespace std;

int main(int argc,char *argv[])
{
	LogClient lger("log.conf");
	int count = 1000000;
	/*while(count--)
		lger.logWarning("有敌人接近");*/
	lger.logError("error happend %d",count);
	string hello = "hello world";
	lger.logInfo(hello);
	return 0;
}

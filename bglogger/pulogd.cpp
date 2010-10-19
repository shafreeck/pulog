#include <iostream>
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <libgearman/gearman.h>

#define ROOT_LOG "./"
#define MAX_PATH_LEN 512
/*packet format
 *-------------------------------------------------
 *|CMD_VER | NAME_LEN | FILE_NAME | MSG_LEN | MSG |
 *-------------------------------------------------
 * */
enum ErrorCode{
	LOG_SUCCESS = -1,
	LOG_ERROR = 0
};
char * get_log_path(char *log_name)
{
	char full_path_name[MAX_PATH_LEN] = {0};
	char *root_path = ROOT_LOG;
	time_t t = time(NULL);
	char stime[20] = {0};
	int stime_size = strftime(stime,sizeof(stime),"%Y%m%d",localtime(&t));
	sprintf(full_path_name,"%s/%s/%s.%s.log",root_path,log_name,log_name,stime);
	//cout<<full_path_name<<endl;
	return strdup(full_path_name);
}
void parse_cmd(char *data,int data_size,char * *log_file,char **msg)
{
	int cur = 0;
	int size = sizeof(uint32_t);
	uint32_t cmd_ver = 0;
	memcpy(&cmd_ver,data+cur,size);
//	cout<<"cmd_ver:"<<cmd_ver<<endl;

	cur += size;
	uint32_t name_len = 0;
	memcpy(&name_len,data+cur,size);
	cur += size;
//	cout<<"name_len:"<<name_len<<endl;

	*log_file = new char[name_len+1];
	memcpy(*log_file,data+cur,name_len);
	(*log_file)[name_len]='\0';         // NOTICE :!!!!  (*log_file) ,() is requied!
	*log_file = get_log_path(*log_file);
	
	cur+=name_len;
	uint32_t msg_len = 0;
	memcpy(&msg_len,data+cur,sizeof(msg_len));
//	cout<<"msg_len:"<<msg_len<<endl;

	*msg = new char[msg_len+1];
	cur+=size;
	memcpy(*msg,data+cur,msg_len);
	(*msg)[msg_len] = '\0'; // NOTICE : !!!! , as above
}
static void dump(void * msg ,int len)
{
	FILE *dump = fopen("dump","w");
	fwrite(msg,len,1,dump);
	fclose(dump);
}

void* write_log(gearman_job_st *job,void *args ,size_t *result_size,gearman_return_t *pRet)
{
	char *data = (char *)gearman_job_workload(job);
	*pRet = GEARMAN_SUCCESS;
	*result_size = 0;
	if(!data)
	{
		*pRet = GEARMAN_WORK_ERROR;
		return NULL;
	}
	int data_size = gearman_job_workload_size(job);
	dump(data,data_size);
	char *log_file=NULL;
	char *msg=NULL;
	parse_cmd(data,data_size,&log_file,&msg);

	char * dir_name = dirname(strdup(log_file)); // dirname will change its paramter
	if(access(dir_name,0) == -1)
	{
		mkdir(dir_name,0777);
	}

	FILE *fd;
	if(!(fd=fopen(log_file,"a")))
	{
		perror("open file");
		*pRet = GEARMAN_WORK_ERROR;
		return NULL;
	}
	fprintf(fd,"%s",msg);

	delete []msg;
	delete []log_file;
	fclose(fd);
	return NULL;
}
int compress_log()
{

}


int main(int argc,char *argv[])
{
	char *shost = NULL;
	int iport = 0;
	gearman_worker_st logger;
	gearman_worker_create(&logger);
	gearman_worker_add_server(&logger,shost,iport);

	gearman_worker_add_function(&logger,"pulog",0,write_log,NULL);

	printf("do work ... \n");
	while(1)
	{
		gearman_worker_work(&logger);
	}
	gearman_worker_free(&logger);
	return 0;
}

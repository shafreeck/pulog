#include "logclient.h"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/unistd.h>
static void dump(void *msg , int len)
{
	FILE *dump = fopen("dump","w");
	fwrite(msg,len,1,dump);
	fclose(dump);

}
LogClient::LogClient(const char *config):m_config(config),m_log_name(NULL),m_log_path(NULL),m_host(NULL),m_port(0),m_output(STDOUT),m_log_level(DEBUG),m_cmd_ver(1)
{
	loadConfig();

#ifdef LOG_GEARMAN
	gearman_client_create(&m_client);
	gearman_client_add_server(&m_client,m_host,m_port);
#endif
}
LogClient::~LogClient()
{
#ifdef LOG_GEARMAN
	gearman_client_free(&m_client);
#endif
	if(m_log_name)
		free(m_log_name);
	if(m_host)
		free(m_host);
	if(m_log_path)
		free(m_log_path);
}
int LogClient::formatMsg(char *buff,const char* slevel, const char *msg, int len)
{
	if(!buff || !msg || !len)return LOG_ERROR;
	int pid = getpid();
	if(pid==-1)return LOG_ERROR;
	char stime[20]={0};
	time_t t = time(NULL);
	int stime_size = strftime(stime,sizeof(stime),"%Y-%m-%d %H:%M:%S",localtime(&t));
	sprintf(buff,"[%s] [%d] [%s] %s\n",stime,pid,slevel,msg);
	return LOG_SUCCESS;
}
int LogClient::log(int level,const char *msg,va_list ap)
{
	if(!msg)return LOG_ERROR;
	if(level > m_log_level)
		return LOG_SUCCESS;
	char buff[512]={0}; //magic num ,FIXME
	vsprintf(buff,msg,ap);
	switch (level)
	{
		case DEBUG:
			writeLog("DEBUG",buff);
			break;
		case INFO:
			writeLog("INFO",buff);
			break;
		case WARNING:
			writeLog("WARNING",buff);
			break;
		case ERROR:
			writeLog("ERROR",buff);
			break;
		default:break;
	}
	return LOG_SUCCESS;
}
int LogClient::logError(const char *msg,...)
{
	int errcode = LOG_SUCCESS;
	va_list ap;
	va_start(ap,msg);
	errcode = log(ERROR,msg,ap);
	va_end(ap);
	return errcode;

}
int LogClient::logWarning(const char *msg,...)
{
	int errcode = LOG_SUCCESS;
	va_list ap;
	va_start(ap,msg);
	errcode = log(WARNING,msg,ap);
	va_end(ap);
	return errcode;
	
}
int LogClient::logInfo(const char *msg,...)
{
	int errcode = LOG_SUCCESS;
	va_list ap;
	va_start(ap,msg);
	errcode = log(INFO,msg,ap);
	va_end(ap);
	return errcode;
	
}
int LogClient::logDebug(const char *msg,...)
{
	int errcode = LOG_SUCCESS;
	va_list ap;
	va_start(ap,msg);
	errcode = log(DEBUG,msg,ap);
	va_end(ap);
	return errcode;
	
}
int LogClient::writeLog(const char* slevel,const char * data)// where to write msg 
{
	if(!data)return LOG_ERROR;

	char msg[MSG_SIZE] = {0};
	if(formatMsg(msg,slevel,data,strlen(data))!=LOG_SUCCESS) // format message string
		return LOG_ERROR;
	
	int errcode = LOG_SUCCESS;
	switch(m_output)
	{
#ifdef LOG_GEARMAN
		case SERVER:
			errcode = gearman_send(msg);
			break;
#endif
		case STDOUT:
			errcode = stdout_print(msg);
			break;
		case STDERR:
			errcode = stderr_print(msg);
			break;
		case WRITEFILE:
			errcode = file_write(msg);
			break;
		default:
			break;
	}
	return errcode;
}

#ifdef LOG_GEARMAN
int LogClient::gearman_send(const char *msg)
{
	if(!msg)return LOG_ERROR;
	int errcode = LOG_SUCCESS;
	
	//allocate memory
	uint32_t msg_len = strlen(msg);
	char *buff = new char[msg_len+3*sizeof(uint32_t)+sizeof("pulog")];
	
	//set command version
	int cur = 0;
	memcpy(buff+cur,&m_cmd_ver,sizeof(uint32_t));

	//set log name length
	cur += sizeof(uint32_t);
	uint32_t log_len = strlen(m_log_name);
	memcpy(buff+cur,&log_len,sizeof(uint32_t));

	//set log name
	cur += sizeof(uint32_t);
	memcpy(buff+cur,m_log_name,log_len);
	
	//set transmited msg length
	cur+= log_len;
	memcpy(buff+cur,&msg_len,sizeof(uint32_t));
	
	// set message data
	cur+= sizeof(uint32_t);
	memcpy(buff+cur,msg,msg_len);

	cur += msg_len;
	

	size_t result_size;
	gearman_return_t ret;
	gearman_client_do(&m_client,"pulog",NULL,buff,cur,&result_size,&ret);
//	gearman_client_add_task(&m_client,NULL,NULL,"pulog",NULL,buff,cur,&ret);
//	gearman_client_run_tasks(&m_client);

	//dump(buff,cur);
	if(ret != GEARMAN_SUCCESS)
		printf("%s\n",gearman_client_error(&m_client));
	delete []buff;

	return errcode;
}
#endif
int LogClient::stdout_print(const char *msg)
{
	if(!msg)return LOG_ERROR;
	fprintf(stdout,"%s",msg);
	return LOG_SUCCESS;
}
int LogClient::stderr_print(const char *msg)
{
	if(!msg)return LOG_ERROR;
	fprintf(stderr,"%s",msg);
	return LOG_SUCCESS;
}
int LogClient::file_write(const char *msg)
{
	if(!msg)return LOG_ERROR;
	FILE *file = fopen(m_log_path,"a");
	if(!file)
	{
		perror("open m_log_path failed");
		return LOG_ERROR;
	}
	fprintf(file,"%s",msg);
	fclose(file);

	return LOG_SUCCESS;
}
int LogClient::loadConfig()
{

	FILE *fd = NULL;
	fd = fopen(m_config,"r");
	if(!fd)return LOG_ERROR;
	int errcode = LOG_SUCCESS;
	char line[128] = {0}; //magic num ,FIXME
	while(fgets(line,sizeof(line),fd))
	{
		if(!line)continue;
		if(line[0]=='#')continue; //skip comment lines
		int j = 0;
		while(isspace(line[j++]));// space is : space ,tab ,enter 
		if(line[j-1]=='\0')continue; // skip blank lines

		int i = 0;
		int len = strlen(line);
		char next_char = ':';
		char *key=NULL,*value=NULL;
		while(line[++i]!=next_char && line[i]!='\n' && line[i]!='\0');
		if(line[i]==next_char)
		{
			line[i] = '\0';
			key = line;
			line[len-1] = '\0'; // overwrite '\n' to '\0'
			value = line+i+1;
		}
		else
		{
			errcode = LOG_ERROR;
			continue;
		}
		if(!strcmp(key,"host"))
		{
			m_host =strdup(value);
		}
		else if(!strcmp(key,"port"))
		{
			m_port = atoi(value);
		}
		else if(!strcmp(key,"level"))
		{
			if(!strcmp(value,"ERROR"))	
				m_log_level = 1;
			else if(!strcmp(value,"WARNING"))	
				m_log_level = 2;
			else if(!strcmp(value,"INFO"))	
				m_log_level = 3;
			else if(!strcmp(value,"DEBUG"))	
				m_log_level = 4;
			else {errcode = LOG_ERROR;continue;}
		}
		else if(!strcmp(key,"output"))
		{
			if(!strcmp(value,"SERVER"))	
				m_output = 0;
			else if(!strcmp(value,"STDOUT"))	
				m_output = 1;
			else if(!strcmp(value,"STDERR"))	
				m_output = 2;
			else {
				m_output = 3;
				m_log_path = strdup(value);
			}

		}
		else if(!strcmp(key,"format"))
		{

		}
		else if(!strcmp(key,"logname"))
		{
			m_log_name = strdup(value);
		}
	}
	fclose(fd);
	return errcode;
}

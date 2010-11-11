#ifndef LOG_CLIENT__H
#define LOG_CLIENT__H
#include <cctype>
#include <cstdlib>
#include <cstdarg>
#include <libgearman/gearman.h>
#define MSG_SIZE 512
enum LOG_LEVEL{// 0 :no log
	DEBUG = 4,
	INFO = 3,
	WARNING = 2,
	ERROR = 1
};
enum OUTPUT{
	SERVER = 0,
	STDOUT = 1,
	STDERR = 2,
	WRITEFILE = 3,
};
enum ERROR_CODE{
	LOG_SUCCESS = 0,
	LOG_ERROR = -1
};
class LogClient
{
	private:
		int m_log_level;
		char *m_host;
		size_t m_port;
		int m_output;
		const char *m_config;
		char * m_log_path;
		char * m_log_name;
		uint32_t m_cmd_ver;
		gearman_client_st m_client;

	public:
		LogClient(const char* config=NULL);
		~LogClient();
		int logError(const char *msg,...);
		int logWarning(const char *msg,...);
		int logInfo(const char *msg,...);
		int logDebug(const char *msg,...);
	private: //private utility func
		int log(int level,const char *msg, va_list ap);
		int writeLog(const char *slevel,const char *msg);
		int gearman_send(const char *msg);
		int stdout_print(const char *msg);
		int stderr_print(const char *msg);
		int file_write(const char *msg);

		int loadConfig();
		int formatMsg(char* buff,const char *level,const char *msg, int msg_len);
};
#endif

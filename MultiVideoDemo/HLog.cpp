#include "include\HLog.h"
#include "log4cplus.h"
#include "include\log4cplus\loglevel.h"

static log4cplus::Logger logger;

HLog::HLog()
{
}


HLog::~HLog()
{
	logger.shutdown();
}

void HLog::init()
{
	log4cplus::initialize();

	//加载配置文件  
	log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("./log4cplus.conf"));

	//设置日志级别  
	log4cplus::Logger::getRoot().setLogLevel(log4cplus::ALL_LOG_LEVEL);

	//初始化logger  
	logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("filelogger"));

	std::locale::global(std::locale("chs"));
}

void HLog::fatal(QString s, char *file, int line, char* function)
{
	logger.log(log4cplus::FATAL_LOG_LEVEL, (wchar_t *)s.utf16(), file, line, function);
}

void HLog::error(QString s, char *file, int line, char* function)
{
	logger.log(log4cplus::ERROR_LOG_LEVEL, (wchar_t *)s.utf16(), file, line, function);
}

void HLog::warn(QString s, char *file, int line, char* function)
{
	logger.log(log4cplus::WARN_LOG_LEVEL, (wchar_t *)s.utf16(), file, line, function);
}

void HLog::info(QString s, char *file, int line, char* function)
{
	logger.log(log4cplus::INFO_LOG_LEVEL, (wchar_t *)s.utf16(), file, line, function);
}

void HLog::debug(QString s, char *file, int line, char* function)
{
	logger.log(log4cplus::DEBUG_LOG_LEVEL, (wchar_t *)s.utf16(), file, line, function);
}

void HLog::trace(QString s, char *file, int line, char* function)
{
	logger.log(log4cplus::TRACE_LOG_LEVEL, (wchar_t *)s.utf16(), file, line, function);
}

void HLog::close()
{
	logger.shutdown();
}


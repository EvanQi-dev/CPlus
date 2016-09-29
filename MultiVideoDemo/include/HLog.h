#pragma once

#include <QtCore>

#define LOG_FATAL(s) \
	HLog::fatal(s, __FILE__, __LINE__, __FUNCTION__)

#define LOG_ERROR(s) \
	HLog::error(s, __FILE__, __LINE__, __FUNCTION__)

#define LOG_WARN(s) \
	HLog::warn(s, __FILE__, __LINE__, __FUNCTION__)

#define LOG_INFO(s) \
	HLog::info(s, __FILE__, __LINE__, __FUNCTION__)

#define LOG_DEBUG(s) \
	HLog::debug(s, __FILE__, __LINE__, __FUNCTION__)

#define LOG_TRACE(s) \
	HLog::trace(s, __FILE__, __LINE__, __FUNCTION__)

class HLog
{
public:
	HLog();
	~HLog();

	static void init();

	static void fatal(QString s, char *file, int line, char* function);
	static void error(QString s, char *file, int line, char* function);
	static void warn(QString s, char *file, int line, char* function);
	static void info(QString s, char *file, int line, char* function);
	static void debug(QString s, char *file, int line, char* function);
	static void trace(QString s, char *file, int line, char* function);

	static void close();
};


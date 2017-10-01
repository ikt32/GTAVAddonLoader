#include <Windows.h>
#include <fstream>
#include <iomanip>

#include "Logger.hpp"

Logger::Logger() {}

void Logger::Clear() const {
	std::ofstream logFile;
	logFile.open(file, std::ofstream::out | std::ofstream::trunc);
	logFile.close();
}

void Logger::Write(const std::string& text) const {
	std::ofstream logFile(file, std::ios_base::out | std::ios_base::app);
	SYSTEMTIME currTimeLog;
	GetLocalTime(&currTimeLog);
	logFile << "[" <<
	           std::setw(2) << std::setfill('0') << currTimeLog.wHour << ":" <<
	           std::setw(2) << std::setfill('0') << currTimeLog.wMinute << ":" <<
	           std::setw(2) << std::setfill('0') << currTimeLog.wSecond << "." <<
	           std::setw(3) << std::setfill('0') << currTimeLog.wMilliseconds << "] " <<
	           text << "\n";
}

//#define CHARS_FOR_BUFF 4096
//#define CHARS_FOR_PARAMS 3500
//void Logger::Write(const char* fmt, ...) {
//	va_list va_alist;
//	char chLogBuff[CHARS_FOR_BUFF];
//	char chParameters[CHARS_FOR_PARAMS];
//	char szTimestamp[30];
//	struct tm current_tm;
//	time_t current_time = time(NULL);
//
//	localtime_s(&current_tm, &current_time);
//	sprintf_s(szTimestamp, "[%02d:%02d:%02d] %%s\n", current_tm.tm_hour, current_tm.tm_min, current_tm.tm_sec);
//
//	va_start(va_alist, fmt);
//	_vsnprintf_s(chParameters, sizeof(chParameters), fmt, va_alist);
//	va_end(va_alist);
//	sprintf_s(chLogBuff, szTimestamp, chParameters);
//	Write(chLogBuff);
//}

void Logger::SetFile(const std::string &fileName) {
	file = fileName;
}

// Everything's gonna use this instance.
Logger logger;

#ifndef COMMONLOG_H
#define COMMONLOG_H

#include <stdio.h>
/**
 * @brief submodule to log
 *
 */

extern int gsc_log_level;

enum LogLevel {
    LogLevel_None = 0x00,
    LogLevel_Error = 0x01,
    LogLevel_Notice = 0x02,
    LogLevel_Debug = 0x03,
    LogLevel_Verbose = 0x04,
};

void set_log_level(enum LogLevel level);

#define DASSERT(expr) do { \
	if (!(expr)) { \
		printf("assertion failed: %s\n\tAt file: %s\n\tfunction: %s, line %d\n", #expr, __FILE__, __FUNCTION__, __LINE__); \
	} \
} while (0)

#define D_LOG_PRINT_BY_LEVEL(level, tag, format, args...)  do { \
		if (gsc_log_level >= level) { \
			printf("%s", tag); \
			printf(format, ##args);  \
		} \
	} while (0)

#define D_LOG_PRINT_TRACE_BY_LEVEL(level, tag, format, args...)  do { \
		if (gsc_log_level >= level) { \
			printf("%s", tag); \
			printf(format, ##args);  \
			printf("            [trace] file %s.\n            function: %s: line %d\n", __FILE__, __FUNCTION__, __LINE__); \
		} \
	} while (0)

#define DPRINT_VERBOSE(format, args...)   D_LOG_PRINT_BY_LEVEL(LogLevel_Verbose, "            [Verbose]: ", format, ##args)
#define DPRINT_DEBUG(format, args...)   D_LOG_PRINT_BY_LEVEL(LogLevel_Debug, "        [Debug]: ", format, ##args)
#define DPRINT_NOTICE(format, args...)   D_LOG_PRINT_BY_LEVEL(LogLevel_Notice, "    [Notice]: ", format, ##args)
#define DPRINT_ERROR(format, args...)   D_LOG_PRINT_TRACE_BY_LEVEL(LogLevel_Error, "[Error]: ", format, ##args)

#endif /* COMMONLOG_H */
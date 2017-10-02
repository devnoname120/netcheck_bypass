#ifndef LOG_H
#define LOG_H

#include <psp2/paf.h>


#define LOG_PATH "ux0:data/"
#define LOG_FILE LOG_PATH "netcheck_log.txt"

void log_reset();
void log_write(const char *buffer, size_t length);

#if 0
#define LOG(...) \
	do { \
		char buffer[256]; \
		sce_paf_private_snprintf(buffer, sizeof(buffer), ##__VA_ARGS__); \
		log_write(buffer, sce_paf_private_strlen(buffer)); \
	} while (0)
#else
#define LOG(...)
#endif
#endif

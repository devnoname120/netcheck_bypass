#include "log.h"
#include <psp2/io/fcntl.h>

extern int sceIoMkdir(const char *, int);

void log_reset()
{
	sceIoMkdir(LOG_PATH, 6);

	SceUID fd = sceIoOpen(LOG_FILE,
		SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fd < 0)
		return;

	sceIoClose(fd);
}

void log_write(const char *buffer, size_t length)
{
	SceUID fd = sceIoOpen(LOG_FILE,
		SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
	if (fd < 0)
		return;

	sceIoWrite(fd, buffer, length);
	sceIoClose(fd);
}

/**
 ******************************************************************************
 * @file      syscalls.c
 * @author    Auto-generated by STM32CubeIDE
 * @brief     STM32CubeIDE Minimal System calls file
 *
 *            For more information about which c-functions
 *            need which of these lowlevel functions
 *            please consult the Newlib libc-manual
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes */
#include <sys/stat.h>
#include <errno.h>
#include <sys/times.h>


extern "C" {
extern int _kill(int pid, int sig);
extern int _getpid(void);
extern void _exit(int status);
extern int _read(int file, char *ptr, int len);
extern int _write(int file, char *ptr, int len);
extern int _close(int file);
};

/* Variables */

char *__env[1] = {0};
char **environ = __env;


/* Functions */
void initialise_monitor_handles()
{
}

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

void _exit(int status)
{
    _kill(status, -1);
    while (1)
    {}        /* Make sure we hang here */
}

int read(int file, char *ptr, int len)
{
    int DataIdx;

//    unsigned read = 0;
//    while(read < len)
//    {
//        read += kernel::serial.Read(ptr, len);
//    }

    return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    (void) file;

//    unsigned written = 0;
//    while(written < len)
//    {
//        written += kernel::serial.Write(ptr, len);
//    }

    return len;
}

int _close(int file)
{
    return -1;
}


extern int _fstat(int file, struct stat *st);
int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

extern int _isatty(int file);
int _isatty(int file)
{
    return 1;
}

extern int _lseek(int file, int ptr, int dir);
int _lseek(int file, int ptr, int dir)
{
    return 0;
}

extern int _open(char *path, int flags, ...);
int _open(char *path, int flags, ...)
{
    /* Pretend like we always fail */
    return -1;
}

extern int _wait(int *status);
int _wait(int *status)
{
    errno = ECHILD;
    return -1;
}

extern int _unlink(char *name);
int _unlink(char *name)
{
    errno = ENOENT;
    return -1;
}

extern int _times(struct tms *buf);
int _times(struct tms *buf)
{
    return -1;
}

extern int _stat(char *file, struct stat *st);
int _stat(char *file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

extern int _link(char *old, char *new_);
int _link(char *old, char *new_)
{
    errno = EMLINK;
    return -1;
}

extern int _fork(void);
int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

extern int _execve(char *name, char **argv, char **env);
int _execve(char *name, char **argv, char **env)
{
    errno = ENOMEM;
    return -1;
}

extern int _sbrk(void);
int _sbrk(void)
{
    return -1;
}

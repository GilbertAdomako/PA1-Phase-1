#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>

/* Structure for Phase 2 pipe communication */
typedef struct {
    int corrupted_chars;
    int corrupted_lines;
} PipeMessage;

#endif /* LOGGER_H */
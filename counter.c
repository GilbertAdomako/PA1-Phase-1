#define _GNU_SOURCE
#include "logger.h"

/* Function prototypes */
void process_file(const char *file_path, int pipe_fd);
void write_results_to_file(const char *file_path, int corrupted_chars, int corrupted_lines);
void send_results_via_pipe(int pipe_fd, int corrupted_chars, int corrupted_lines);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return 1;
    }
    
    const char *file_path = argv[1];
    int pipe_fd = -1; /* -1 indicates Phase 1 (no pipe) */

    process_file(file_path, pipe_fd);

    return 0;
}

void process_file(const char *file_path, int pipe_fd) {
    int corrupted_chars = 0;
    int corrupted_lines = 0;

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror(file_path);
        return;
    }

    char *line = NULL;
    size_t capacity = 0;
    ssize_t line_len;

    while ((line_len = getline(&line, &capacity, file)) != -1) {
        int line_corrupted = 0;

        for (ssize_t i = 0; i < line_len; i++) {
            if (!isascii((unsigned char)line[i])) {
                corrupted_chars++;
                line_corrupted = 1;
            }
        }
        corrupted_lines += line_corrupted;
    }

    free(line);
    fclose(file);

    if (pipe_fd == -1) {
        write_results_to_file(file_path, corrupted_chars, corrupted_lines);
    } else {
        send_results_via_pipe(pipe_fd, corrupted_chars, corrupted_lines);
    }
}

void write_results_to_file(const char *file_path, int corrupted_chars, int corrupted_lines) {
    const char *slash = strrchr(file_path, '/');
    const char *dot = strrchr(file_path, '.');
    
    /* Safely isolate the base path without extension if it exists */
    size_t base_len = (dot != NULL && (slash == NULL || dot > slash))
                          ? (size_t)(dot - file_path)
                          : strlen(file_path);
                          
    char *log_path = malloc(base_len + sizeof(".log"));
    if (log_path == NULL) {
        perror("malloc");
        return;
    }

    memcpy(log_path, file_path, base_len);
    memcpy(log_path + base_len, ".log", sizeof(".log"));

    FILE *log_file = fopen(log_path, "w");
    if (log_file == NULL) {
        perror(log_path);
        free(log_path);
        return;
    }

    fprintf(log_file, "%d\n", corrupted_chars);
    fprintf(log_file, "%d\n", corrupted_lines);
    
    fclose(log_file);
    free(log_path);
}

void send_results_via_pipe(int pipe_fd, int corrupted_chars, int corrupted_lines) {
    (void)pipe_fd; (void)corrupted_chars; (void)corrupted_lines;
}
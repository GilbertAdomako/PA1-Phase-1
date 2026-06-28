#define _GNU_SOURCE
#include "logger.h"

/* Function prototypes */
void process_file(const char *file_path, int pipe_fd);
void write_results_to_file(const char *file_path, int corrupted_chars, int corrupted_lines);
void send_results_via_pipe(int pipe_fd, int corrupted_chars, int corrupted_lines);

int main(int argc, char *argv[]) {
    int pipe_fd = -1;
    const char *file_path;

    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Usage: %s <file_path> [pipe_fd]\n", argv[0]);
        return 1;
    }

    file_path = argv[1];
    if (argc == 3) {
        pipe_fd = atoi(argv[2]);
    }

    process_file(file_path, pipe_fd);

    return 0;
}

void process_file(const char *file_path, int pipe_fd) {
    int corrupted_chars = 0;
    int corrupted_lines = 0;

    FILE *file = fopen(file_path, "r");
    char *line = NULL;
    size_t capacity = 0;
    ssize_t line_len;

    if (file == NULL) {
        perror(file_path);
        return;
    }

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
    
    if (pipe_fd < 0) {
        write_results_to_file(file_path, corrupted_chars, corrupted_lines);
    } else {
        send_results_via_pipe(pipe_fd, corrupted_chars, corrupted_lines);
    }
}

void write_results_to_file(const char *file_path, int corrupted_chars, int corrupted_lines) {
    const char *slash = strrchr(file_path, '/');
    const char *dot = strrchr(file_path, '.');
    size_t base_len = (dot != NULL && (slash == NULL || dot > slash))
                          ? (size_t)(dot - file_path)
                          : strlen(file_path);
    char *log_path = malloc(base_len + sizeof(".log"));
    FILE *log_file;

    if (log_path == NULL) {
        perror("malloc");
        return;
    }

    memcpy(log_path, file_path, base_len);
    memcpy(log_path + base_len, ".log", sizeof(".log"));

    log_file = fopen(log_path, "w");
    if (log_file == NULL) {
        perror(log_path);
        free(log_path);
        return;
    }

    fprintf(log_file, "%d %d\n", corrupted_chars, corrupted_lines);
    fclose(log_file);
    free(log_path);
}

void send_results_via_pipe(int pipe_fd, int corrupted_chars, int corrupted_lines) {
    PipeMessage message = {corrupted_chars, corrupted_lines};

    write(pipe_fd, &message, sizeof(message));
}

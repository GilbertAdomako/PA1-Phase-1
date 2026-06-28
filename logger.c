#include "logger.h"

/* Function prototypes */
void process_directory(const char *dir_path);

typedef struct {
    pid_t pid;
    char *file_path;
    int pipe_fd;
} ChildInfo;

static ChildInfo *children = NULL;
static size_t child_count = 0;
static size_t child_capacity = 0;

static int add_child(pid_t pid, const char *file_path, int pipe_fd) {
    ChildInfo *new_children;

    if (child_count == child_capacity) {
        child_capacity = child_capacity == 0 ? 16 : child_capacity * 2;
        new_children = realloc(children, child_capacity * sizeof(*children));
        if (new_children == NULL) {
            perror("realloc");
            return -1;
        }
        children = new_children;
    }

    children[child_count].file_path = strdup(file_path);
    if (children[child_count].file_path == NULL) {
        perror("strdup");
        return -1;
    }

    children[child_count].pid = pid;
    children[child_count].pipe_fd = pipe_fd;
    child_count++;
    return 0;
}

static int is_log_file(const char *file_path) {
    const char *dot = strrchr(file_path, '.');

    return dot != NULL && strcmp(dot, ".log") == 0;
}

int main(int argc, char *argv[]) {
    const char *dir_path;
    int total_corrupted_chars = 0;
    int total_corrupted_lines = 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory_path>\n", argv[0]);
        return 1;
    }

    dir_path = argv[1];
    process_directory(dir_path);

    for (size_t i = 0; i < child_count; i++) {
        PipeMessage message = {0, 0};

        waitpid(children[i].pid, NULL, 0);
        if (read(children[i].pipe_fd, &message, sizeof(message)) == sizeof(message)) {
            printf("%s: %d, %d\n",
                   children[i].file_path,
                   message.corrupted_chars,
                   message.corrupted_lines);
            total_corrupted_chars += message.corrupted_chars;
            total_corrupted_lines += message.corrupted_lines;
        }
        close(children[i].pipe_fd);
        free(children[i].file_path);
    }

    free(children);
    printf("Total Corrupted Characters: %d\n", total_corrupted_chars);
    printf("Total Corrupted Lines: %d\n", total_corrupted_lines);
    
    return 0;
}

void process_directory(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    struct dirent *entry;

    if (dir == NULL) {
        perror(dir_path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char *entry_path;
        struct stat entry_stat;
        size_t path_len;

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        path_len = strlen(dir_path) + strlen(entry->d_name) + 2;
        entry_path = malloc(path_len);
        if (entry_path == NULL) {
            perror("malloc");
            continue;
        }

        snprintf(entry_path, path_len, "%s/%s", dir_path, entry->d_name);

        if (lstat(entry_path, &entry_stat) == -1) {
            perror(entry_path);
            free(entry_path);
            continue;
        }

        if (S_ISDIR(entry_stat.st_mode)) {
            process_directory(entry_path);
        } else if (S_ISREG(entry_stat.st_mode) && !is_log_file(entry_path)) {
            int pipe_fds[2];
            pid_t pid;

            if (pipe(pipe_fds) == -1) {
                perror("pipe");
                free(entry_path);
                continue;
            }

            pid = fork();
            if (pid == -1) {
                perror("fork");
                close(pipe_fds[0]);
                close(pipe_fds[1]);
                free(entry_path);
                continue;
            }

            if (pid == 0) {
                char pipe_fd_arg[32];

                close(pipe_fds[0]);
                snprintf(pipe_fd_arg, sizeof(pipe_fd_arg), "%d", pipe_fds[1]);
                execl("./counter", "counter", entry_path, pipe_fd_arg, (char *)NULL);
                perror("./counter");
                _exit(1);
            }

            close(pipe_fds[1]);
            if (add_child(pid, entry_path, pipe_fds[0]) == -1) {
                close(pipe_fds[0]);
            }
        }

        free(entry_path);
    }

    closedir(dir);
}

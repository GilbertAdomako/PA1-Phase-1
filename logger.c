#include "logger.h"

/* Function prototypes */
void process_directory(const char *dir_path);

/* Dynamic list structure to track child PIDs and paths for shallow verification */
typedef struct {
    pid_t pid;
    char *file_path;
    char *log_path;
} ChildTracker;

static ChildTracker *tracked_children = NULL;
static size_t child_count = 0;
static size_t child_capacity = 0;

static void track_child(pid_t pid, const char *file_path) {
    if (child_count == child_capacity) {
        child_capacity = child_capacity == 0 ? 16 : child_capacity * 2;
        tracked_children = realloc(tracked_children, child_capacity * sizeof(ChildTracker));
    }
    
    tracked_children[child_count].pid = pid;
    tracked_children[child_count].file_path = strdup(file_path);
    
    /* Calculate and cache matching log path location */
    const char *slash = strrchr(file_path, '/');
    const char *dot = strrchr(file_path, '.');
    size_t base_len = (dot != NULL && (slash == NULL || dot > slash)) ? (size_t)(dot - file_path) : strlen(file_path);
    
    char *log = malloc(base_len + sizeof(".log"));
    memcpy(log, file_path, base_len);
    memcpy(log + base_len, ".log", sizeof(".log"));
    
    tracked_children[child_count].log_path = log;
    child_count++;
}

static int is_log_extension(const char *file_path) {
    const char *dot = strrchr(file_path, '.');
    return dot != NULL && strcmp(dot, ".log") == 0;
}

int main(int argc, char *argv[]) {
    /* Task 1 - Parse command line arguments to get the directory path */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PATH_TO_DIR>\n", argv[0]);
        return 1;
    }
    
    process_directory(argv[1]);
    
    return 0;
}

void process_directory(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        perror(dir_path);
        return;
    }

    struct dirent *entry;
    
    /* Task 1 - Open the directory and read its entries */
    while ((entry = readdir(dir)) != NULL) {
        /* Skip "." and ".." to avoid circular loops */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        size_t path_len = strlen(dir_path) + strlen(entry->d_name) + 2;
        char *entry_path = malloc(path_len);
        snprintf(entry_path, path_len, "%s/%s", dir_path, entry->d_name);

        struct stat entry_stat;
        if (lstat(entry_path, &entry_stat) == -1) {
            perror(entry_path);
            free(entry_path);
            continue;
        }

        /* Phase 1: Shallow parsing ONLY. Ignore subdirectories. */
        if (S_ISREG(entry_stat.st_mode) && !is_log_extension(entry_path)) {
            /* Task 2 - Fork a child process */
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                free(entry_path);
                continue;
            }

            if (pid == 0) {
                /* In the child: execute the ./counter program using execl() */
                execl("./counter", "counter", entry_path, (char *)NULL);
                perror("execl failed");
                _exit(1);
            } else {
                /* In the parent: track child context */
                track_child(pid, entry_path);
            }
        }
        free(entry_path);
    }
    closedir(dir);

    /* Task 2 - Parent must wait for ALL child processes to terminate */
    for (size_t i = 0; i < child_count; i++) {
        waitpid(tracked_children[i].pid, NULL, 0);
    }

    /* Phase 1 Data Aggregation: Read log files and sum results */
    int total_corrupted_chars = 0;
    int total_corrupted_lines = 0;

    for (size_t i = 0; i < child_count; i++) {
        FILE *log_file = fopen(tracked_children[i].log_path, "r");
        int file_chars = 0;
        int file_lines = 0;

        if (log_file != NULL) {
            if (fscanf(log_file, "%d\n%d", &file_chars, &file_lines) == 2) {
                total_corrupted_chars += file_chars;
                total_corrupted_lines += file_lines;
            }
            fclose(log_file);
        }
        
        /* Print individual file metrics if expected by your submission validation environment */
        printf("%s: %d, %d\n", tracked_children[i].file_path, file_chars, file_lines);

        /* Clean allocations */
        free(tracked_children[i].file_path);
        free(tracked_children[i].log_path);
    }

    /* Print total calculations */
    printf("Total Corrupted Characters: %d\n", total_corrupted_chars);
    printf("Total Corrupted Lines: %d\n", total_corrupted_lines);

    free(tracked_children);
}
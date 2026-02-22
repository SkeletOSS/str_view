#include "str_view.h"
#include "test.h"

#ifdef __linux__
#    include <linux/limits.h>
#    define FILESYS_MAX_PATH PATH_MAX
#endif
#ifdef __APPLE__
#    include <sys/syslimits.h>
#    define FILESYS_MAX_PATH NAME_MAX
#endif

#include <dirent.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

struct Path_bin {
    SV_Str_view path;
    SV_Str_view bin;
};

static SV_Str_view const test_prefix = SV_from("test_");
static char const *const pass_msg = "⬤";
static char const *const fail_msg = "X";
static char const *const err_msg = "Test process failed abnormally:";

static int run(SV_Str_view);
static enum Test_result run_test_process(struct Path_bin);
static DIR *open_test_dir(SV_Str_view);
static bool fill_path(char *, SV_Str_view, SV_Str_view);

int
main(int argc, char **argv) {
    if (argc == 1) {
        return 0;
    }
    SV_Str_view arg_view = SV_from_terminated(argv[1]);
    return run(arg_view);
}

static int
run(SV_Str_view const tests_dir) {
    DIR *dir_ptr = open_test_dir(tests_dir);
    if (!dir_ptr) {
        return 1;
    }
    char absolute_path[FILESYS_MAX_PATH];
    size_t tests = 0;
    size_t passed = 0;
    struct dirent const *d;
    bool fail = false;
    while ((d = readdir(dir_ptr))) {
        SV_Str_view const entry = SV_from_terminated(d->d_name);
        if (!SV_starts_with(entry, test_prefix)) {
            continue;
        }
        if (!fill_path(absolute_path, tests_dir, entry)) {
            goto done;
        }
        printf("%s(%s%s\n", CYAN, SV_begin(entry), NONE);
        (void)fflush(stdout);
        enum Test_result const res = run_test_process(
            (struct Path_bin){SV_from_terminated(absolute_path), entry});
        switch (res) {
            case ERROR:
                (void)fprintf(stderr, "%s%s%s %s%s\n", RED, err_msg, CYAN,
                              SV_begin(entry), NONE);
                break;
            case PASS:
                (void)fprintf(stdout, "%s%s%s)%s\n", GREEN, pass_msg, CYAN,
                              NONE);
                break;
            case FAIL:
                (void)fprintf(stdout, "%s%s%s)%s\n", RED, fail_msg, CYAN, NONE);
                break;
        }
        passed += 1 - res;
        ++tests;
    }
    fail = passed != tests;
    fail
        ? printf("%sPASSED %zu/%zu T_T%s\n\n", RED, passed, tests, NONE)
        : printf("%sPASSED %zu/%zu \\(*.*)/%s\n\n", GREEN, passed, tests, NONE);
done:
    closedir(dir_ptr);
    return fail;
}

enum Test_result
run_test_process(struct Path_bin pb) {
    if (SV_is_empty(pb.path)) {
        (void)fprintf(stderr, "No test provided.\n");
        return ERROR;
    }
    pid_t const test_proc = fork();
    if (test_proc == 0) {
        execl(SV_begin(pb.path), SV_begin(pb.bin), NULL);
        (void)fprintf(stderr, "Child test process could not start.\n");
        exit(1);
    }
    int status = 0;
    if (waitpid(test_proc, &status, 0) < 0) {
        (void)fprintf(stderr, "Error running test: %s\n", SV_begin(pb.bin));
        return ERROR;
    }
    if (WIFSIGNALED(status)
        || (WIFEXITED(status) && WEXITSTATUS(status) == FAIL)) {
        return FAIL;
    }
    return PASS;
}

static DIR *
open_test_dir(SV_Str_view tests_folder) {
    if (SV_is_empty(tests_folder) || SV_len(tests_folder) > FILESYS_MAX_PATH) {
        (void)fprintf(stderr, "Invalid input to path to test executables %s\n",
                      SV_begin(tests_folder));
        return NULL;
    }
    DIR *dir_ptr = opendir(SV_begin(tests_folder));
    if (!dir_ptr) {
        (void)fprintf(stderr, "Could not open directory %s\n",
                      SV_begin(tests_folder));
        return NULL;
    }
    return dir_ptr;
}

static bool
fill_path(char *path_buf, SV_Str_view tests_dir, SV_Str_view entry) {
    size_t const dir_bytes = SV_fill(FILESYS_MAX_PATH, path_buf, tests_dir);
    if (FILESYS_MAX_PATH - dir_bytes < SV_bytes(entry)) {
        (void)fprintf(stderr, "Relative path exceeds FILESYS_MAX_PATH?\n%s",
                      path_buf);
        return false;
    }
    (void)SV_fill(FILESYS_MAX_PATH - dir_bytes, path_buf + SV_len(tests_dir),
                  entry);
    return true;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <seccomp.h>
#include <sys/prctl.h>
#include <linux/seccomp.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <program>\n", argv[0]);
        return 1;
    }

    // Initialize the libseccomp context
    scmp_filter_ctx ctx;
    ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (ctx == NULL) {
        fprintf(stderr, "seccomp_init failed\n");
        return 1;
    }

    // Intercept the getrandom syscall and return an error
    if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(getrandom), 0) < 0) {
        fprintf(stderr, "seccomp_rule_add failed\n");
        return 1;
    }

    // Load the seccomp policy into the kernel
    if (seccomp_load(ctx) < 0) {
        fprintf(stderr, "seccomp_load failed\n");
        return 1;
    }

    // Execute the given program
    execvp(argv[1], &argv[1]);

    fprintf(stderr, "execvp failed\n");
    return 1;
}


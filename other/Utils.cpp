/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#include <execinfo.h>

#include <sys/types.h>
#include <signal.h>

#include <other/Utils.h>
#include <other/Log.h>

namespace roo {

static void backtrace_info(int sig, siginfo_t* info, void* f) {
    int j, nptrs;
#define BT_SIZE 100
    char** strings;
    void* buffer[BT_SIZE];

    fprintf(stderr,       "\nSignal [%d] received.\n", sig);
    fprintf(stderr,       "======== Stack trace ========");

    nptrs = ::backtrace(buffer, BT_SIZE);
    fprintf(stderr,       "backtrace() returned %d addresses\n", nptrs);

    strings = ::backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    for (j = 0; j < nptrs; j++)
        fprintf(stderr, "%s\n", strings[j]);

    free(strings);

    fprintf(stderr,       "Stack Done!\n");

    ::kill(getpid(), sig);
    ::abort();

#undef BT_SIZE
}

void backtrace_init() {

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags     = SA_NODEFER | SA_ONSTACK | SA_RESETHAND | SA_SIGINFO;
    act.sa_sigaction = backtrace_info;
    sigaction(SIGABRT, &act, NULL);
    sigaction(SIGBUS,  &act, NULL);
    sigaction(SIGFPE,  &act, NULL);
    sigaction(SIGSEGV, &act, NULL);

    return;
}

} // roo

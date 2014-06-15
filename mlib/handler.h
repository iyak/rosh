#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

typedef void (*sighandler_t)(int);

extern void reg_signal_handler(int, sighandler_t);
extern void dfl_signal_handler(int);
extern void ign_signal(int);
extern void block_signal(int);
extern void unblock_signal(int);

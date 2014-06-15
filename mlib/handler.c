#include "handler.h"

void reg_signal_handler(int signum, sighandler_t hdl)
{
    struct sigaction act;
    sigset_t sigset;
    if (-1 == sigemptyset(&sigset))
    {
        perror("rosh: sigemptyset");
        return;
    }
    act.sa_handler = hdl;
    act.sa_mask = sigset;
    act.sa_flags = 0;
    if (-1 == sigaction(signum, &act, NULL))
    {
        perror("rosh: sigaction");
        return;
    }
}

void dfl_signal_handler(int signum)
{
    reg_signal_handler(signum, SIG_DFL);
}

void ign_signal(int signum)
{
    reg_signal_handler(signum, SIG_IGN);
}

void block_signal(int signum)
{
    sigset_t sigBlock;
    if (-1 == sigemptyset(&sigBlock))
    {
        perror("rosh: sigemptyset");
        return;
    }
    if (-1 == sigaddset(&sigBlock, signum))
    {
        perror("rosh: sigaddset");
        return;
    }
    if (-1 == sigprocmask(SIG_BLOCK, &sigBlock, NULL))
    {
        perror("rosh: sigprocmask");
        return;
    }
}

void unblock_signal(int signum)
{
    sigset_t sigBlock;
    if (-1 == sigemptyset(&sigBlock))
    {
        perror("rosh: sigemptyset");
        return;
    }
    if (-1 == sigaddset(&sigBlock, signum))
    {
        perror("rosh: sigaddset");
        return;
    }
    if (-1 == sigprocmask(SIG_UNBLOCK, &sigBlock, NULL))
    {
        perror("rosh: sigprocmask");
        return;
    }
}

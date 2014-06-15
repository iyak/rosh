#include "main.h"

char* envUser;
char* envPwd;
char* envHome;
char* envPath;

int shell_terminal;
pid_t shell_pgid;
job* first_job = NULL;

void set_pos_environ()
{
    int i;
    for (i = 0; !mystrbgn(environ[i], "USER="); ++i) ;
    envUser = environ[i] + 5;
    for (i = 0; !mystrbgn(environ[i], "PWD="); ++i) ;
    envPwd = environ[i]  + 4;
    for (i = 0; !mystrbgn(environ[i], "HOME="); ++i) ;
    envHome = environ[i] + 5;
    for (i = 0; !mystrbgn(environ[i], "PATH="); ++i) ;
    envPath = environ[i] + 5;
}

static void sigchld_hdl(int sig)
{
    int e = errno, status;
    sig = sig + 1 - 1; /* to make gcc happy */
    pid_t pid;
    while (0 < (pid = waitpid(-1, &status, WUNTRACED | WCONTINUED | WNOHANG)))
        if (-1 == mark_process_exit_status(pid, status)) {
            myerror("process was not found");
            exit(1);
        }
    errno = e;
    return;
}

int main(void)
{
    shell_terminal = STDIN_FILENO;

    /* wait for rosh come foreground */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
        kill(-shell_pgid, SIGTTIN);

    /* ignore job control signal of root shell */
    ign_signal(SIGTSTP);
    ign_signal(SIGINT);
    ign_signal(SIGQUIT);
    ign_signal(SIGTTOU);
    ign_signal(SIGTTIN);
    reg_signal_handler(SIGCHLD, sigchld_hdl);

    /* enter independent pg */
    if (-1 == setpgid(0, 0)) {
        perror("rosh");
        exit(1);
    }

    /* let system know rosh receive terminal control */
    if (-1 == tcsetpgrp(shell_terminal, shell_pgid)) {
        perror("rosh");
        exit(1);
    }
 
    char user_str[LINELEN];
    set_pos_environ(); 
    job *new_job, *last_job;

    /* show prompt and exec user commands */
    while (get_line(user_str, LINELEN)) {
        block_signal(SIGCHLD);
        if (NULL != (new_job = parse_line(user_str))) {
            /* add new job to job list */
            if (!first_job)
                first_job = new_job;
            else {
                for (last_job = first_job; last_job->next; last_job = last_job->next) ;
                last_job->next = new_job;
            }
            exec_job(new_job);
        }
        first_job = organize_job(first_job);
        unblock_signal(SIGCHLD);
    }
    return 0;
}

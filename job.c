#include "job.h"

int job_is_done(job *j)
{
    /* all process in job is COMPLETED or TERMINATED */
    process *p;
    for (p = j->first_process; NULL != p; p = p->next)
        if (COMPLETED != p->run_status && TERMINATED != p->run_status)
            return 0;
    return 1;
}

int job_is_suspended(job *j)
{
    /* at least one process in job is SUSPENDED */
    process *p;
    for (p = j->first_process; NULL != p; p = p->next)
        if (SUSPENDED == p->run_status)
            return 1;
    return 0;
}

process *detect_process(pid_t pid)
{
    job *j;
    process *p;
    for (j = first_job; NULL != j; j = j->next)
        for (p = j->first_process; NULL != p; p = p->next)
            if (pid == p->pid)
                return p;
    return NULL;
}

int mark_process_exit_status(pid_t pid, int status)
{
    process *p;
    if (NULL == (p = detect_process(pid)))
        return 0;

    /* if child exited normally */
    if (WIFEXITED(status)) {
        p->run_status = COMPLETED;
        p->exit_status = status;
        return 0;
    }

    /* if child was terminated by signal */
    else if (WIFSIGNALED(status)) {
        p->run_status = TERMINATED;
        p->exit_status = status;
        return 0;
    }

    /* if child was stopped by signal */
    else if (WIFSTOPPED(status)) {
        p->run_status = SUSPENDED;
        p->exit_status = status;
        return 0;
    }

    /* if child was resumed by siganl */
    else if (WIFCONTINUED(status)) {
        p->run_status = LAUNCHED;
        return 0;
    }

    return -1;
}

job *organize_job(job *j)
{
    if (!j)
        return NULL;
    if (job_is_done(j)) {
        if (BACKGROUND == j->mode)
            printf("\n[%d]\tDone\n", j->index);
        job *j_next = j->next;
        free_job(j);
        return organize_job(j_next);
    }
    j->next = organize_job(j->next);
    return j;
}

void display_jobs(void)
{
    job *j;
    process *p;
    for (j = first_job; NULL != j; j = j->next) {
        printf("\n+ %ld\n", (long int)j->pgid);
        for (p = j->first_process; NULL != p; p = p->next)
            printf("\t- [%p] %s(%p) - %d\n", p, p->program_name, p->program_name, p->run_status);
    }
}

void run_foreground_job(job *j, int cont)
{
    j->mode = FOREGROUND;

    /* put job in foreground */
    while (-1 == tcsetpgrp(shell_terminal, j->pgid)) ;

    if (cont) {
        if (-1 == kill(-j->pgid, SIGCONT))
            perror("kill (SIGCONT)");
        else {
            j->index = 0;
            process *p;
            for (p = j->first_process; NULL != p; p = p->next)
                if (SUSPENDED == p->run_status)
                    p->run_status = LAUNCHED;
        }
    }

    int status, s, c;
    do {
        pid_t pid = waitpid(-j->pgid, &status, WUNTRACED);
        if (-1 == mark_process_exit_status(pid, status)) {
            myerror("process was not found");
            break;
        }
    }
    while (
            0 == (s = job_is_suspended(j)) && 
            0 == (c = job_is_done(j))
          ) ;
    if (1 == s) {
        give_job_index(j);
        printf("\n[%d]\tStopped\n", j->index);
    }
    if (-1 == tcsetpgrp(shell_terminal, shell_pgid))
        perror("rosh");
}

void run_background_job(job *j, int cont)
{
    j->mode = BACKGROUND;

    if (cont) {
        if (-1 == kill(-j->pgid, SIGCONT))
            perror("kill (SIGCONT)");
        else {
            process *p;
            for (p = j->first_process; NULL != p; p = p->next)
                if (SUSPENDED == p->run_status)
                    p->run_status = LAUNCHED;
        }
    }
    printf("\n[%d]\t%d\n", give_job_index(j), j->pgid);
}

int give_job_index(job *j)
{
    int i = 0;
    job *jobTmp;
    for (jobTmp = first_job; NULL != jobTmp; jobTmp = jobTmp->next)
        if (i < jobTmp->index)
            i = jobTmp->index;
    ++ i;
    j->index = i;
    return i;
}

job *find_job_by_index(int index)
{
    /* if 0 is given, return most recent indexed job */
    job *j, *jobTarget = NULL;
    if (0 == index) {
        int i;
        for (i = 0, j = first_job; NULL != j; j = j->next) {
            if (i < j->index) {
                i = j->index;
                jobTarget = j;
            }
        }
        if (0 == i)
            return NULL;
        else
            return jobTarget;
    }
    else
        for (j = first_job; NULL != j; j = j->next)
            if (index == j->index)
                return j;
    return NULL;
}

#include "exec.h"

int execve_path(char *name, char *argp[], char *const *envp)
{
    /* append path to program name if necessary */
    const int lenArgWithPath = 128;
    int i, j, k;
    char *path, *argWithPath = NULL;
    if (-1 == access(name, F_OK)) {
        for (i = 0; NULL != envp[i] && !mystrbgn(envp[i], "PATH="); ++ i) ;
        if (NULL == envp[i]) {
            myerror("path not found");
            return -1;
        }
        path = envp[i];
        if (NULL == (argWithPath = (char *)malloc(lenArgWithPath)))
            return -1;
        j = k = 5;
        do {
            if ( '\0' == path[j] || ':' == path[j]) {
                argWithPath[j - k] = '/';
                argWithPath[j - k + 1] = '\0';
                mystrcat(argWithPath, name);
                if (-1 == access(argWithPath, F_OK)) {
                    k = j + 1;
                    continue;
                }
                mystrcpy(name, argWithPath);
                mystrcpy(argp[0], argWithPath);
                break;
            } else
                argWithPath[j - k] = path[j];
        } while ('\0' != path[j] && ++ j);
    }
    FREE(argWithPath);
    return execve(name, argp, envp);
}

int isBuiltin(char *name)
{
    const char **ic;
    for (ic = builtinCommands; NULL != *ic; ++ ic)
        if (0 == mystrcmp(name, *ic))
            return 1;
    return 0;
}

int exec_builtin(char *name, char *argp[])
{
    if (0 == mystrcmp(name, "exit")) {
        printf("exit\n");
        exit(0);
    }
    if (0 == mystrcmp(name, "cd")) {
        if (NULL == argp[1])
            return chdir(envHome);
        return chdir(argp[1]);
    }
    if (0 == mystrcmp(name, "fg")) {
        job *j;
        int index = argp[1]? atoi(argp[1]) : 0;
        if (NULL == (j = find_job_by_index(index))) {
            myerror("rosh: fg: no such job\n");
            return 0;
        }
        if (job_is_suspended(j))
            run_foreground_job(j, 1);
        else
            run_foreground_job(j, 0);
        return 0;
    }
    if (0 == mystrcmp(name, "bg")) {
        job *j;
        int index = argp[1]? atoi(argp[1]) : 0;
        if (NULL == (j = find_job_by_index(index))) {
            myerror("rosh: fg: no such job\n");
            return 0;
        }
        if (job_is_suspended(j))
            run_background_job(j, 1);
        return 0;
    }
    if (0 == mystrcmp(name, "jobs")) {
        job *j;
        for (j = first_job; NULL != j; j = j->next) {
            if (0 == j->index)
                continue;
            printf("[%d]\t", j->index);
            if (job_is_suspended(j))
                printf("SUSPENDED\t");
            else
                printf("RUNNING\t");
            process *p;
            for (p = j->first_process; NULL != p; p = p->next)
                printf("%s(%d) ", p->program_name, p->pid);
            printf("\n");
        }
        return 0;
    }
    return -1;
}

void exec_job(job *curr_job)
{
    job_mode modeJob = curr_job->mode;
    process *curr_process = curr_job->first_process;
    pid_t pidFork = -1;
    int fdPipe[2][2] = {{-1, -1}, {-1, -1}};
    volatile int newPipe, oldPipe, countFork;

    /* main-loop in processes in job */
    for (
            countFork = 0,
            newPipe = 0, oldPipe = 1;
            NULL != curr_process;
            curr_process = curr_process->next,
            newPipe = countFork % 2,
            oldPipe = (countFork + 1) % 2
        ) {
        /* modify process run_status */
        curr_process->run_status = LAUNCHED;

        /* continue if builtin command */
        if (isBuiltin(curr_process->program_name)) {
            if (-1 == exec_builtin (
                        curr_process->program_name,
                        curr_process->argument_list
                        ))
                perror("rosh");
            curr_process->run_status = COMPLETED;
            continue;
        }

        /* create pipes if necessary */
        if (
                (NULL != curr_process->next ||
                 !(fdPipe[newPipe][0] = fdPipe[newPipe][1] = -1)) &&
                -1 == pipe(fdPipe[newPipe])
           ) {
            perror("rosh: pipe");
            return;
        }

        /* fork here */
        if (-1 == (pidFork = fork())) {
            perror("rosh: fork");
            break;
        }
        else
            ++ countFork;

        /* child */
        if (0 == pidFork) {
            /* enter independent pg */
            if (1 == countFork) {
                if (-1 == setpgid(0, 0))
                    perror("rosh: child: setpgid");
            } else {
                if (-1 == setpgid(0, curr_job->pgid))
                    perror("rosh: child: setpgid");
            }
            if (FOREGROUND == modeJob && -1 == tcsetpgrp(shell_terminal, getpgid(0)))
                perror("");

            /* stop ignoring signals */
            dfl_signal_handler(SIGTSTP);
            dfl_signal_handler(SIGINT);
            dfl_signal_handler(SIGQUIT);
            dfl_signal_handler(SIGTTOU);
            dfl_signal_handler(SIGTTIN);
            dfl_signal_handler(SIGCHLD);

            /* connect pipe */
            if (
                    -1 != fdPipe[newPipe][0] && (
                        -1 == close(1) ||
                        -1 == dup2(fdPipe[newPipe][1], 1) ||
                        -1 == close(fdPipe[newPipe][0]))
               ) {
                perror("rosh: dup pipe");
                exit(1);
            }
            if (
                    -1 != fdPipe[oldPipe][0] && (
                        -1 == close(0) ||
                        -1 == dup2(fdPipe[oldPipe][0], 0) ||
                        -1 == close(fdPipe[oldPipe][1]))
               ) {
                perror("rosh: dup pipe");
                exit(1);
            }

            /* connect redirection */
            if (NULL != curr_process->input_redirection) {
                int fdIn;
                if (-1 == (fdIn = open (
                                curr_process->input_redirection,
                                O_RDONLY
                                ))) {
                    perror("rosh");
                    exit(1);
                }
                if (-1 == close(0) || -1 ==  dup2(fdIn, 0) || -1 == close(fdIn)) {
                    perror("rosh");
                    exit(1);
                }
            }
            if (NULL != curr_process->output_redirection) {
                int fdOut, modeWrite;
                switch (curr_process->output_option) {
                    case TRUNC:  modeWrite = O_TRUNC;  break;
                    case APPEND: modeWrite = O_APPEND; break;
                    default:
                                 printf("rosh: redirection error.");
                                 exit(1);
                }
                if (-1 == (fdOut = open (
                                curr_process->output_redirection,
                                O_CREAT | O_WRONLY | modeWrite,
                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
                                ))) {
                    perror("rosh");
                    exit(1);
                }
                if (-1 == close(1) || -1 ==  dup2(fdOut, 1) || -1 == close(fdOut)) {
                    perror("rosh");
                    exit(1);
                }
            }

            /* exec program */
            if (
                    -1 == execve_path (
                        curr_process->program_name,
                        curr_process->argument_list,
                        environ
                        )) {
                perror("rosh: execve");
                exit(1);
            }
            exit(1);
        }/* child */

        /* register process info */
        curr_process->pid = pidFork;

        /* shell process */
        /* put child into independent pg */
        if (1 == countFork) {
            if (-1 == setpgid(pidFork, 0))
                perror("rosh: setpgid");
            curr_job->pgid = pidFork;
        } else {
            if (-1 == setpgid(pidFork, curr_job->pgid))
                perror("rosh: setpgid");
        }
        if (FOREGROUND == modeJob && -1 == tcsetpgrp(shell_terminal, curr_job->pgid))
            perror("rosh: tcsetpgrp");

        /* close old pipe */
        if
            ( -1 != fdPipe[oldPipe][0] && (
                                           -1 == close(fdPipe[oldPipe][0]) ||
                                           -1 == close(fdPipe[oldPipe][1]))
            ) {
                perror("rosh: close pipe");
                return;
            }
    }

    /* builtin command job */
    if (-1 == pidFork)
        return;

    /* foreground job */
    if (FOREGROUND == modeJob)
        run_foreground_job(curr_job, 0);

    /* background job */
    if (BACKGROUND == modeJob)
        run_background_job(curr_job, 0);
}

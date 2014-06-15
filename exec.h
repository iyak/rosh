#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "mlib/str.h"
#include "mlib/handler.h"
#include "job.h"

extern char *const *environ;
extern char *envHome;
/* last member must be NULL */
const char *builtinCommands[] = {"cd", "exit", "fg", "bg", "jobs", NULL};
extern int shell_terminal; 
extern int shell_pgid;
extern job *first_job;
extern void unblock_signal(int);
extern void dfl_signal_handler(int);
extern int job_is_done(job *);
extern int job_is_suspended(job *);
extern int mark_process_exit_status(pid_t, int);
extern void run_foreground_job(job *, int);
extern void run_background_job(job *, int);
extern job *find_job_by_index(int);

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "jobdef.h"
#include "mlib/handler.h"
#include "mlib/str.h"

extern int shell_terminal;
extern int shell_pgid;
extern void free_job(job*);
extern job *first_job;
extern int give_job_index(job*);

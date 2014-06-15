#include <errno.h>
#include "jobdef.h"
#include "mlib/str.h"
#include "mlib/handler.h"

extern void exec_job(job*);
extern void print_job_list(job*);
extern char* const* environ;
extern void signal_setting();

extern void reg_signal_handler(int, sighandler_t);
extern void dfl_signal_handler(int);
extern void ign_signal(int);
extern void block_signal(int);
extern void unblock_signal(int);
extern job* parse_line(char*);
extern process* detect_process(job*, pid_t);
extern int mark_process_exit_status(pid_t, int);
extern int get_line(char*, int);
extern job* organize_job(job*);
extern void display_jobs(void);

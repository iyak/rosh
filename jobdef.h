#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#define NAMELEN 32      /* 各種名前の長さ */
#define ARGLSTLEN 16    /* 1つのプロセスがとる実行時引数の数 */
#define LINELEN 256     /* 入力コマンドの長さ */

#define FREE(x) free(x);x=NULL;

typedef enum write_option_
{
    TRUNC,
    APPEND,
} write_option;

typedef enum p_run_status_
{
    UNLAUNCH,
    LAUNCHED,
    COMPLETED,
    TERMINATED,
    SUSPENDED
} p_run_status;

typedef struct process_
{
    char            *program_name;
    char            **argument_list;
    char            *input_redirection;
    write_option    output_option;
    char            *output_redirection;
    struct process_ *next;
    p_run_status    run_status; 
    int             exit_status;
    pid_t           pid;
} process;

typedef enum job_mode_
{
    FOREGROUND,
    BACKGROUND,
} job_mode;

typedef struct job_
{
    job_mode        mode;
    process         *first_process;
    struct job_     *next;
    pid_t           pgid;
    int             index;
} job;

typedef enum parse_state_
{
    ARGUMENT,
    IN_REDIRCT,
    OUT_REDIRCT_TRUNC,
    OUT_REDIRCT_APPEND,
} parse_state;


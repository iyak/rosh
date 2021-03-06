#include "parse.h"

char *get_line(char *s, int size)
{
    char pwd[PWDLEN] = "";
    if (NULL == getcwd(pwd, PWDLEN)) {
        perror("rosh");
        return NULL;
    }
    if (NULL != envHome && mystrbgn(pwd, envHome)) {
        char pwd_home[PWDLEN] = "~";
        mystrcat(pwd_home, pwd + mystrlen(envHome));
        printf(PROMPT, pwd_home);
    }
    else
        printf(PROMPT, pwd);
    while(fgets(s, size, stdin) == NULL) {
        if(errno == EINTR)
            continue;
        return NULL;
    }
    return s;
}

static char *initialize_program_name(process *p)
{
    if(!(p->program_name = (char*)malloc(sizeof(char)*NAMELEN)))
        return NULL; 
    memset(p->program_name, 0, NAMELEN);
    return p->program_name;
}

static char **initialize_argument_list(process *p) {

    if(!(p->argument_list = (char **)malloc(sizeof(char *) * ARGLSTLEN)))
        return NULL;

    int i;
    for(i=0; i<ARGLSTLEN; i++) 
        p->argument_list[i] = NULL;

    return p->argument_list;
}

static char *initialize_argument_list_element(process *p, int n)
{
    if(!(p->argument_list[n] = (char *)malloc(sizeof(char) * NAMELEN)))
        return NULL; 
    memset(p->argument_list[n], 0, NAMELEN);
    return p->argument_list[n];
}

static char *initialize_input_redirection(process *p)
{
    if(!(p->input_redirection = (char *)malloc(sizeof(char) * NAMELEN)))
        return NULL; 
    memset(p->input_redirection, 0, NAMELEN);
    return p->input_redirection;
}

static char* initialize_output_redirection(process *p)
{
    if(!(p->output_redirection = (char *)malloc(sizeof(char) * NAMELEN)))
        return NULL; 
    memset(p->output_redirection, 0, NAMELEN);
    return p->output_redirection;
}

static process* initialize_process()
{
    process *p;
    if((p = (process *)malloc(sizeof(process))) == NULL)
        return NULL;
    initialize_program_name(p);
    initialize_argument_list(p);
    initialize_argument_list_element(p, 0);
    p->input_redirection = NULL;
    p->output_option = TRUNC;
    p->output_redirection = NULL;
    p->next = NULL;
    p->pid = -1;
    p->run_status = UNLAUNCH;
    p->exit_status = 0;
    return p;
}

static job *initialize_job()
{
    job *j;

    if((j = (job *)malloc(sizeof(job))) == NULL) 
        return NULL;

    j->mode = FOREGROUND;
    j->first_process = initialize_process();
    j->next = NULL;
    j->pgid = -1;
    j->index = 0;

    return j;
}

static void free_process(process *p)
{
    if(!p)
        return;
    free_process(p->next);

    if(p->program_name)
        FREE(p->program_name);
    if(p->input_redirection)
        FREE(p->input_redirection);
    if(p->output_redirection)
        FREE(p->output_redirection);

    if(p->argument_list) {
        int i;
        for(i = 0; NULL != p->argument_list[i]; ++ i)
            FREE(p->argument_list[i]);
        FREE(p->argument_list);
    }
    FREE(p);
}

void free_job(job *j)
{
    if(!j)
        return;
    free_process(j->first_process);
    FREE(j);
}

/* parser */
/* 受け付けた文字列を解析して結果をjob構造体に入れる関数 */
job* parse_line(char *buf)
{
    job *curr_job = NULL;
    process *curr_prc = NULL;
    parse_state state = ARGUMENT;
    int index=0, arg_index=0;

    /* 改行文字まで解析する */
    while(*buf != '\n') {
        /* 空白およびタブを読んだときの処理 */
        if(*buf == ' ' || *buf == '\t') { 
            buf++;
            if(index) {
                index = 0;
                state = ARGUMENT;
                ++arg_index;
            }
        } 
        /* 以下の3条件は、状態を遷移させる項目である */
        else if(*buf == '<') { 
            state = IN_REDIRCT; 
            buf++; 
            index = 0; 
        }
        else if(*buf == '>') {
            buf++; 
            index = 0;
            if(state == OUT_REDIRCT_TRUNC) {
                state = OUT_REDIRCT_APPEND;
                if(curr_prc)
                    curr_prc->output_option = APPEND;
            } else
                state = OUT_REDIRCT_TRUNC; 
        } else if(*buf == '|') { 
            state = ARGUMENT; 
            buf++; 
            index = 0; 
            arg_index = 0;
            if(curr_job) {
                mystrcpy (
                     curr_prc->program_name, 
                     curr_prc->argument_list[0]
                    );
                curr_prc->next = initialize_process();
                curr_prc = curr_prc->next;
            }
        }
        /* &を読めば、modeをBACKGROUNDに設定し、解析を終了する */
        else if(*buf == '&') {
            buf++;
            if(curr_job) {
                curr_job->mode = BACKGROUND;
                break;
            }
        }
        /* 以下の3条件は、各状態でprocess構造体の各メンバに文字を格納する */
        /* 状態ARGUMENTは、 リダイレクション対象ファイル名以外の文字(プログラム名、オプション)を読む状態 */
        /* 状態IN_REDIRCTは入力リダイレクション対象ファイル名を読む状態 */
        /* 状態OUT_REDIRCT_*は出力リダイレクション対象ファイル名を読む状態 */
        else if(state == ARGUMENT) {
            if(!curr_job) {
                curr_job = initialize_job();
                curr_prc = curr_job->first_process;
            }

            if(!curr_prc->argument_list[arg_index])
                initialize_argument_list_element(curr_prc, arg_index);
            curr_prc->argument_list[arg_index][index++] = *buf++;
        } else if(state == IN_REDIRCT) {
            if(!curr_prc->input_redirection) 
                initialize_input_redirection(curr_prc);
            curr_prc->input_redirection[index++] = *buf++;
        } else if(state == OUT_REDIRCT_TRUNC || state == OUT_REDIRCT_APPEND) {
            if(!curr_prc->output_redirection) 
                initialize_output_redirection(curr_prc);
            curr_prc->output_redirection[index++] = *buf++;
        }
    }

    /* 最後に、引数の0番要素をprogram_nameにコピーする */
    if(curr_prc)
        mystrcpy(curr_prc->program_name, curr_prc->argument_list[0]);
    return curr_job;
}

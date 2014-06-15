/* 
Headers for parse.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "mlib/str.h"
#include "jobdef.h"

#define PROMPT "ro[%s]%% " /* 入力ライン冒頭の文字列 */
#define PWDLEN 128

extern char *const *environ;
extern char *envUser;
extern char *envPwd;
extern char *envHome;
extern char *envPath;

#include <stdio.h>
#include <unistd.h>

/* self-defined functions for string */
extern int mystrbgn(const char*, const char*);
extern void mystrcpy(char*, const char*);
extern void mystrcat(char*, const char*);
extern void mystrini(char*, int);
extern int mystrcmp(const char*, const char*);
extern int mystrlen(const char*);
extern void myerror(const char*);

#include "str.h"

int mystrcmp(const char *str1, const char *str2)
{
    for (; '\0' != *str1 && '\0' != *str2 && *str1 == *str2; ++str1, ++str2);
    return *str1 - *str2;
}

void mystrcat(char *dst, const char *src)
{
    while (*dst++ != '\0');
    dst -= 1;
    while (*src != '\0')
        *dst++ = *src++;
    *dst = '\0';
}

int mystrbgn(const char *str, const char *ptrn)
{
    while ('\0' != *ptrn)
        if (*ptrn++ != *str++)
            return 0;
    return 1;
}

void mystrcpy(char *dst, const char *src)
{
    while (*src != '\0')
        *dst++ = *src++;
    *dst = '\0';
}

void mystrini(char *str, int l)
{
    while (l--)
        str[l] = '\0';
}

void myerror(const char *msg)
{
    int numOfCharsWrtn, numOfCharsOk;
    for (
            numOfCharsWrtn = numOfCharsOk = 0;
            msg[numOfCharsWrtn] != '\0';
            numOfCharsWrtn += numOfCharsOk
        )
        if (-1 == (numOfCharsOk = write(2, msg + numOfCharsWrtn, 1))) {
            perror("myerr");
            return;
        }
    return;
}

int mystrlen(const char* str)
{
    int i;
    for (i = 0; '\0' != str[i]; ++i) ;
    return i;
}

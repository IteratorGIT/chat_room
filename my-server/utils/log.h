#ifndef UTILS_LOG_H
#define UTILS_LOG_H

#include <stdio.h>
#include <stdarg.h>

#define LOG_ERROR(format, ...) my_log((format, ##__VA_ARGS__))

void my_log(const char * format, ...){
    //考虑加锁
    va_list valst;
    va_start(valst, format);

    vprintf(format, valst);

    va_end(valst);
}


#endif
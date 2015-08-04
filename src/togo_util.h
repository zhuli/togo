/*
 * util.h
 * Created on: 2014-2-20
 * Author: zhuli
 */
#ifndef TOGO_UTIL_H
#define TOGO_UTIL_H

#define togo_abs(value)       (((value) >= 0) ? (value) : - (value))
#define togo_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define togo_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))


int togo_get_time(void);
int togo_log(enum log_level level, const u_char *fmt, ...);
void togo_exit(void);


#endif

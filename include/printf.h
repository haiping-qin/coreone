#ifndef __PRINT_H__
#define __PRINT_H__

#include <stdarg.h>

#define PR_SOH		"\001"
#define PR_SOH_ASCII	'\001'

#define PR_ERR		PR_SOH "1"
#define PR_WARN		PR_SOH "2"
#define PR_INFO		PR_SOH "3"
#define PR_DEBUG	PR_SOH "4"

#define PRINT_LEVEL_ERR		1
#define PRINT_LEVEL_WARN	2
#define PRINT_LEVEL_INFO	3
#define PRINT_LEVEL_DEBUG	4

#if !defined(CONFIG_PRINT_LEVEL) || (CONFIG_PRINT_LEVEL > PRINT_LEVEL_DEBUG)
#define PRINT_LEVEL	PRINT_LEVEL_DEBUG
#else
#define PRINT_LEVEL	CONFIG_PRINT_LEVEL
#endif

static inline int print_get_level(const char *buffer)
{
	if (buffer[0] == PR_SOH_ASCII && buffer[1]) {
		switch (buffer[1]) {
		case '0' ... '4':
		return buffer[1] - '0';
		}   
	}   
	return 0;
}

static inline const char *print_skip_level(const char *buffer)
{
	if (buffer[0] == PR_SOH_ASCII && buffer[1]) {
		switch (buffer[1]) {
		case '0' ... '4':
		return buffer + 2;
		}
	}
	return buffer;
}

int vsnprintf(char *str, size_t len, const char *fmt, va_list ap);
int vprintf(const char *fmt, va_list ap);
int printf(const char *fmt, ...) __printf(1, 2);

/*
 * Dummy printk for disabled debugging statements to use whilst maintaining
 * gcc's format and side-effect checking.
 */
static inline int __printf(1, 2) no_printf(const char *fmt, ...)
{
	return 0;
}

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#define pr_err(fmt, ...) \
	printf(PR_ERR pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warn(fmt, ...) \
	printf(PR_WARN pr_fmt(fmt), ##__VA_ARGS__)
#define pr_info(fmt, ...) \
	printf(PR_INFO pr_fmt(fmt), ##__VA_ARGS__)

#if defined(DEBUG)
#define pr_debug(fmt, ...) \
	printf(PR_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...) \
	no_printf(PR_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#endif

#endif

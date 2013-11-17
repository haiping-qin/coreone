#ifndef __COMPILER_H__
#define __COMPILER_H__

#define __noreturn __attribute__((noreturn))
#define __nonnull __attribute__((nonnull))
#define __printf(a, b) __attribute__((format(printf, a, b)))

#endif

#ifndef __TYPES_H__
#define __TYPES_H__

#define false	0
#define true	1
#define TRUE	1
#define FALSE	0
#define NULL	(void *)0

typedef int bool;

typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;
typedef unsigned long long	uint64_t;

typedef signed char		int8_t;
typedef short			int16_t;
typedef int			int32_t;
typedef long long		int64_t;

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;

typedef unsigned long		size_t;
typedef long			ssize_t;
typedef long long		off_t;

typedef unsigned long	time_t;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#endif

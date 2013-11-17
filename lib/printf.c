#include <common.h>
#include <string.h>

#define PR_BUFSIZ	256

#define UART_PL01x_FR_TXFF              0x20
#define UART_FIFO_REG			0x101f1018
#define UART_DATA_REG			0x101f1000

static int print_level = PRINT_LEVEL;

static void putc(char c)
{
	while ((*((u32 *)UART_FIFO_REG)) & UART_PL01x_FR_TXFF)
		;
	(*((u32 *)UART_DATA_REG)) = c;
}

void _puts(const char *s)
{
	s = print_skip_level(s);

	while (*s) {
		if (*s == '\n')
			putc('\r');
		putc(*s);
		s++;
	}
}

void puts(const char *s)
{
	int level;

	level = print_get_level(s);
	if (level <= print_level)
		_puts(s);
}

static char *itoa(unsigned long long value, char *buf, int len,
					int base, int upper)
{
	static const char hextable[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	static const char hextable_upper[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	const char *table;
	int pos;

	if (len < 2)
		return buf;

	if (upper && base == 16)
		table = hextable_upper;
	else
		table = hextable;

	pos = len - 1;
	buf[pos] = 0;

	do {
		buf[--pos] = table[value % base];
		value /= base;
	} while (value && pos);

	return buf + pos;
}

#define _FLONG		0x00000001
#define _FLLONG		0x00000002
#define _FHALF		0x00000004
#define _FHHALF		0x00000008
#define _FALT		0x00000010
#define _FCAPS		0x00000020
#define _FSHOWSIGN	0x00000040
#define _FSIGNED	0x00000080
#define _FLALIGN	0x00000100
#define _FLZERO		0x00000200

int vsnprintf(char *str, size_t len, const char *fmt, va_list ap)
{
	unsigned long long n;
	size_t count = 0;
	char fmtbuf[32];
	int fmtnr;
	int l;
	int flags;
	int base;
	char *s;
	char *prefix;
	char c;

#define _PUTC(c) do { *str++ = c; count++; \
	if ((count + 1) >= len) goto done; } while (0)
#define _NEXT(f) do { flags |= (f); goto next_format; } while (0)

	while ((c = *fmt++) != 0) {
		if (c != '%') {
			_PUTC(c);
			continue;
		}

		flags = 0;
		fmtnr = 0;
		prefix = "";
next_format:
		c = *fmt++;
		if (c == 0)
			break;

		switch (c) {
		case '.': goto next_format;
		case '-': _NEXT(_FLALIGN);
		case '+': _NEXT(_FSHOWSIGN);
		case '#': _NEXT(_FALT);
		case 'l': _NEXT((flags & _FLONG)? _FLLONG : _FLONG);
		case 'h': _NEXT((flags & _FHALF)? _FHHALF : _FHALF);
		case 'c': c = (char)va_arg(ap, int);
		case '%': _PUTC(c); break;

		case '0':
			if (fmtnr == 0)
				flags |= _FLZERO;
		case '1' ... '9':
			fmtnr = 10 * fmtnr + c - '0';
			goto next_format;

		case 's':
			if (flags & _FLZERO)
				flags &= ~_FLZERO;
			s = (char *)va_arg(ap, unsigned int);
			goto output_string;

		case 'i':
		case 'd': flags |= _FSIGNED;
		case 'u': base = 10; goto convert_string;

		case 'p': flags |= _FLONG | _FALT; goto hex;
		case 'X': flags |= _FCAPS;
hex:
		case 'x':
			if (flags & _FALT)
				prefix = "0x";
			base = 16;
			goto convert_string;

		case 'o':
			if (flags & _FALT)
				prefix = "0";
			base = 8;
			goto convert_string;

		default:
			_PUTC('%');
			_PUTC(c);
			break;
		}

		continue;

convert_string:
		if (flags & _FSIGNED) {
			n = (flags & _FLLONG)? va_arg(ap, long long) :
				(flags & _FLONG)? va_arg(ap, long) :
				(flags & _FHHALF)? (char)va_arg(ap, int) :
				(flags & _FHALF)? (short)va_arg(ap, int) :
				va_arg(ap, int);
			if ((long long)n < 0) {
				n = -n;
				prefix = "-";
			} else if (flags & _FSHOWSIGN) {
				prefix = "+";
			}
		} else {
			n = (flags & _FLLONG)? va_arg(ap, unsigned long long) :
				(flags & _FLONG)? va_arg(ap, unsigned long) :
				(flags & _FHHALF)? (unsigned char)va_arg(ap, unsigned int) :
				(flags & _FHALF)? (unsigned short)va_arg(ap, unsigned int) :
				va_arg(ap, unsigned int);
		}

		s = itoa(n, fmtbuf, sizeof(fmtbuf), base, !!(flags & _FCAPS));

output_string:
		l = strlen(prefix) + strlen(s);
		c = (flags & _FLZERO)? '0' : ' ';

		if (!(flags & _FLALIGN) && c == ' ')
			while (fmtnr-- > l)
				_PUTC(c);
		while (*prefix != 0)
			_PUTC(*prefix++);
		if (!(flags & _FLALIGN) && c == '0')
			while (fmtnr-- > l)
				_PUTC(c);
		while (*s != 0)
			_PUTC(*s++);
		if (flags & _FLALIGN)
			while (fmtnr-- > l)
				_PUTC(' ');
	}

done:
	*str = '\0';

	return count;
}

int vprintf(const char *fmt, va_list ap)
{
	char buf[PR_BUFSIZ];
	int r;

	r = vsnprintf(buf, PR_BUFSIZ, fmt, ap);
	_puts(buf);

	return r;
}

int printf(const char *fmt, ...)
{
        va_list ap; 
	int level;
        int r;

	level = print_get_level(fmt);
	if (level > print_level)
		return 0;

        va_start(ap, fmt);
        r = vprintf(fmt, ap);
        va_end(ap);

	return r;
}

typedef unsigned int u32;

#define UART_PL01x_FR_TXFF              0x20
#define UART_FIFO_REG			0x101f1018
#define UART_DATA_REG			0x101f1000

void putc(char c)
{
	while ((*((u32 *)UART_FIFO_REG)) & UART_PL01x_FR_TXFF)
		;
	(*((u32 *)UART_DATA_REG)) = c;
}

void puts(const char *s)
{
	while (*s) {
		if (*s == '\n')
			putc('\r');
		putc(*s);
		s++;
	}
}

int kmain(void)
{
	puts("Hello Core One\n");
}

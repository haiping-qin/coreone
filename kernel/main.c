#include <common.h>
#include <sys/init.h>

void __noreturn kmain(void)
{
	printf("Hello Core One\n");

	heap_init();

	while (1);
}

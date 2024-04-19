#include <stdio.h>
#include <stdlib.h>

extern long __stack_chk_guard;

int main() {
	printf("canary 0x%08lx @%p", __stack_chk_guard, &__stack_chk_guard);
}

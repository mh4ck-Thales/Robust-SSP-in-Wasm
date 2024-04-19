/*
	CookieCrumbl0r: Smashing the Stack Protector for Fun and Profit
	Copyright (C) 2018 Bruno Bierbaumer, Julian Kirsch

	Usage: gcc -fstack-protector-all cookiecrumbl0r.c -o cookiecrumbl0r -pthread -std=c11
	Usage: cl.exe /GS cookiecrumbl0r.c
*/

#define _POSIX_C_SOURCE 199309 // wtf, sigaction
#define __XSI_VISIBLE   500    // wtf, sigaction + freebsd
#define _XOPEN_SOURCE  600     // wtf, sizeof(MCTXT_GREGS) + macos


#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>

#if !defined (_MSC_VER)
#include <pthread.h>
#endif


#ifdef __linux__

#include <sys/auxv.h>
#include <sys/wait.h>
#include <features.h>

// wtf, some includes don't have this constant
#define ARCH_GET_FS 0x1003
extern int arch_prctl(int code, unsigned long *addr);
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
#include <sys/ucontext.h> // such fuckup, this one needs stack_t
#endif

#define NUM_CHILDS 3

unsigned long * get_cookie_addr(void)
{

#if defined(__linux__) && defined(__x86_64__)
	unsigned long tmp;
	arch_prctl(ARCH_GET_FS, &tmp);
	return (unsigned long *)(tmp + 0x28);

#elif defined(__linux__) && defined(__i386__)
	__asm__ ("mov %gs:0, %eax");
	__asm__ ("add $0x14, %eax");

#elif defined(__linux__) && defined(__s390x__)
	{ uintptr_t x; __asm__ ("ear %0,%%a0; sllg %0,%0,32; ear %0,%%a1; lg %0,0x0(%0)" : "=a" (x)); return x + 0x28; }

#elif defined(__linux__) && defined(__PPC__)
	{ uintptr_t x; __asm__ ("li %0, -28680 ; add %0,2,%0" : "=r" (x)); return x; }

#elif defined( __OpenBSD__)
	extern long __guard_local;
	return (unsigned long *)&__guard_local;

#elif defined(__FreeBSD__) || defined(__linux__) || defined(__APPLE__)
	extern long __stack_chk_guard;
	return (unsigned long *)&__stack_chk_guard;

#elif defined(_WIN32) || defined(_WIN64)
	//extern long __stack_chk_guard;
	//return (unsigned long *)&__stack_chk_guard;
	//extern long __security_cookie;
	return (unsigned long *)&__security_cookie;
#endif

}

#ifdef __FreeBSD__
#define MCTXT_GREGS (__register_t *)(&context->uc_mcontext)
#define MCTXT_GREGS_NUM_ELEMS (sizeof(context->uc_mcontext) / sizeof(__register_t))
#elif defined(__APPLE__)
#define MCTXT_GREGS (long *)(context->uc_mcontext)
#define MCTXT_GREGS_NUM_ELEMS 64 //wtf
#elif defined(__OpenBSD__)
#define MCTXT_GREGS (long *)(context)
#define MCTXT_GREGS_NUM_ELEMS 64 //wtf
#elif defined(__dietlibc__) || defined(__MUSL__)
#define MCTXT_GREGS (long *)(&context->uc_mcontext)
#define MCTXT_GREGS_NUM_ELEMS (sizeof(context->uc_mcontext) / sizeof(long *))
#elif defined(__PPC__) || defined(__arm__)
#define MCTXT_GREGS (long *)(&context->uc_mcontext)
#define MCTXT_GREGS_NUM_ELEMS 64
#elif (__GLIBC__ == 2 ) && (__GLIBC_MINOR__ >= 26) 
#define MCTXT_GREGS (context->uc_mcontext.__gregs)
#define MCTXT_GREGS_NUM_ELEMS (sizeof(MCTXT_GREGS) / sizeof(MCTXT_GREGS[0]))
#else
#define MCTXT_GREGS (context->uc_mcontext.gregs)
#define MCTXT_GREGS_NUM_ELEMS (sizeof(MCTXT_GREGS) / sizeof(MCTXT_GREGS[0]))
#endif


#if defined(_WIN32) || defined(_WIN64)
size_t get_writeable_range(void *start, void *end)
{
	unsigned char *ptr;
	void *tmp;

	if (start > end) {
		tmp = end;
		end = start;
		start = tmp;
	}
	for (ptr = start; ptr < (unsigned char *)end && !IsBadWritePtr(ptr, 1); ptr++);

	return (size_t)(ptr - (unsigned char *)start);
}
#else
static int stop_write = 0;
unsigned char *write_ptr = 0;
static long writeable = 0;

static void handler(int sig_no, siginfo_t* si, void *vcontext)
{
  ucontext_t *context = (ucontext_t*)vcontext;
	unsigned int i = 0;

	//printf("Got SIGSEGV at address: 0x%lx %d\n",(long) si->si_addr, MCTXT_GREGS_NUM_ELEMS);

	/* Decrease pointer by one, as instruction is going to be re-executed */
	for (i = 0; i < MCTXT_GREGS_NUM_ELEMS; i++) {
		if ((void *)*(MCTXT_GREGS + i) == write_ptr) {
  		(*(MCTXT_GREGS + i)) = &writeable;
		}
	}
	write_ptr--;
	stop_write = 1;
}

size_t get_writeable_range(void *start, void *end)
{
	struct sigaction sa;
	unsigned char tmp;
	void *tmp_ptr;

	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = handler;
	if ((sigaction(SIGSEGV, &sa, NULL) == -1) || (sigaction(SIGBUS, &sa, NULL) == -1)) {
		fprintf(stderr, "sigaction");
		_exit(-1);
	}

	if (start > end) {
		// three xor variable swap does not work painlessly on void * :(
		tmp_ptr = end;
		end = start;
		start = tmp_ptr;
	}

	// is writeable!!!
	stop_write = 0;
	for (write_ptr = start; (write_ptr >= (unsigned char *)start) &&
			(write_ptr < (unsigned char *)end) && !stop_write; write_ptr++) {
		tmp = *write_ptr;
		if (stop_write) break;
		*write_ptr = tmp;
	}
	return (size_t) (write_ptr - (unsigned char *)start);
}
#endif


#if __STDC_VERSION__ >= 201100
char __thread tls_addr[128] = {0};
#elif defined(_MSC_VER)
__declspec(thread) tls_addr[128] = {0};
#endif


static long global_addr[128] = {0};

void * measure(void *type)
{
	char local_addr[128] = {0};
	long * heap_addr = malloc(128);

	printf("%-13s 0x%016lx 0x%016lx 0x%016lx 0x%016lx 0x%016lx 0x%016lx 0x%016lx 0x%016lx 0x%016lx 0x%016lx 0x%016lx 0x%016lx\n",
		(char *) type,
		(long unsigned int) local_addr,
		(long unsigned int) get_cookie_addr(),
		(long unsigned int) *get_cookie_addr(),
		(long unsigned int) get_writeable_range(get_cookie_addr(), (void *)get_cookie_addr() + sizeof(void *)),
		(long unsigned int) get_writeable_range(get_cookie_addr(), local_addr),
#if __STDC_VERSION__ >= 201100 || defined(_MSC_VER)
		(long unsigned int) tls_addr,
		(long unsigned int) get_writeable_range(get_cookie_addr(), tls_addr),
#else
		0xdeadbeef,
		0x0c0ffeee,
#endif
		(long) global_addr,
		(long unsigned int) get_writeable_range(get_cookie_addr(), global_addr),
		(long) heap_addr,
		(long unsigned int) get_writeable_range(get_cookie_addr(), heap_addr),
		(long) measure
	);

	free(heap_addr);

	return 0;
}

int main()
{
#if defined(_WIN32) || defined(_WIN64)
	HANDLE tid[NUM_CHILDS];
#else
	pthread_t tid[NUM_CHILDS];
#endif

	printf("%13s %18s %18s %18s %18s %18s %18s %18s %18s %18s %18s %18s %18s\n",
		"type",
		"local_addr",
		"cookie_addr",
		"cookie_val",
		"cookie_writeable",
		"cookie-local-w",
		"thread_addr",
		"cookie-thread-w",
		"global_addr",
		"cookie-global-w",
		"heap_addr",
		"cookie-heap-w",
		"code"
	);

	measure("main");
	for(int i = 0; i < NUM_CHILDS; i++){
#if defined(_WIN32) || defined(_WIN64)
		tid[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)measure, "thread", 0, NULL);
		WaitForSingleObject(tid[i], INFINITE);
#else
		pthread_create(tid + i, NULL, measure, "thread");
		pthread_join(tid[i], NULL);
#endif
	}

	exit(0);
}


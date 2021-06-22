#include <linux/kernel.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>


#include "testcases.h"

#define PAGE_SIZE	(4096)

#define HUGE_SIZE	(4096*512)

#define SIG_TEST 42

#define __NR_swap 443

#define __NR_sigballoon 442


#define BIT_VALUE(val, x)	(((val) & (1ull << x)) != 0)


void *buff;
unsigned long nr_signals = 0;

typedef unsigned long long u8;

u8 page_list[100];
int n = 100;

u8 *list = page_list;

enum
{
GET=0,
SET
};

u8 inactive_pages = 0,i=0;

/* Reference taken for idle_page_tracking from https://github.com/brendangregg/wss/blob/master/wss-v1.c */

int idle_page_track(u8 s, u8 e, int act)
{
	char path[256];
	int fd, fd_file;
	int fd_kfile;
	u8 buf;
	u8 buf1;
	u8 m,pg,k;
	u8 sp = 0;
	

	if (sprintf(path, "/proc/%d/pagemap", getpid()) < 0) 
		err(2, "error to get path");
		
		
	if ((fd_file = open(path, O_RDONLY)) < 0)
		err(2,"-ENXIO");
		
	if(act==GET)
		{	
			fd = open("/sys/kernel/mm/page_idle/bitmap", O_RDONLY);
			if (fd < 0)
				err(2, "can't open bitmap for read");
		}
	else
		{	
			fd = open("/sys/kernel/mm/page_idle/bitmap", O_WRONLY);
			if (fd < 0)
				err(2, "can't open bitmap for write");
		}	
		
	if ((fd_kfile = open("/proc/kpageflags", O_RDONLY)) < 0)
					err(2,"can't open kpageflags");
	
	for(m=s; m<e; m=m+k)
		{
			pg= 8*m/PAGE_SIZE;
				
			if(pread(fd_file, &buf, sizeof(buf), pg)!=sizeof(buf))
				err(2, "can't able to read pagemap");
					
			buf = buf & 0x7fffffffffffff;
			if(buf == 0)
				continue;
				
			if(pread(fd_kfile, &buf1, sizeof(buf1), buf*8)!=sizeof(buf1))
					err(2, "can't able to read kpageflags");
			
			if(act==GET)
				{ 
					
					if(pread(fd, &sp, sizeof(sp), buf / 64 * 8)!=sizeof(sp))
						err(2, "can't able to read bitmapfile");
						
					/*if(BIT_VALUE(buf1, 22))
						k = HUGE_SIZE;
					else*/
						k = PAGE_SIZE;
						
					if(BIT_VALUE(sp, buf % 64))
						{							
							inactive_pages++;
							if(i < n)
								page_list[i++] = buf;}
				}	
	  	else
				{
				
					sp = ~0ULL;
					if(pwrite(fd, &sp, sizeof(sp), buf / 64 * 8)!=sizeof(sp))
						err(2, "can't able to write on bitmapfile");
				}		
					
		}
		
	
	close(fd);
	close(fd_file);
	close(fd_kfile);
	
	return 0;	
		
}

int page_map(int bit_act)
{
	FILE *pmf;
	char path[256];
	char fl[256];
	u8 start;
	u8 end;
	u8 *pfns;
	int fd;
	
	
	if (sprintf(path, "/proc/%d/maps", getpid()) < 0){ 
		printf("error to get path\n");
       		exit(1);
		}
	
	if((pmf=fopen(path,"r"))==NULL)
		err(2,"can't able to open pagemapfile");
	
	while(fgets(fl,sizeof(fl),pmf)!=NULL)
	{
		sscanf(fl,"%llx-%llx", &start, &end);
			
		if(start <= 0xffff880000000000LLU)
			idle_page_track(start, end, bit_act);
			
		else
			continue;
	}
	
	
	fclose(pmf);
	
	if(bit_act == GET)
		printf("reclaim_pages = %llu\n",inactive_pages);
			
	inactive_pages = 0;
	return 0;
}


/*
 * 			placeholder-3
 * implement your page replacement policy here
 */

long swap_syscall(u8 *arg, int n1)
{
	return syscall(__NR_swap, arg, n1);
}

/*
 * 			placeholder-2
 * implement your signal handler here
 */

void signalFunction(int t, siginfo_t *info, void *unused) {
    nr_signals++;
    
    page_map(GET);
    	
    swap_syscall(list, n);
   
    page_map(SET);
    	
    i = 0;
}




long sigballoon_syscall(void)
{
	return syscall(__NR_sigballoon);
}

int main(int argc, char *argv[])
{
	int *ptr, nr_pages;
	long check;

  ptr = mmap(NULL, TOTAL_MEMORY_SIZE, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	if (ptr == MAP_FAILED) {
		printf("mmap failed\n");
       		exit(1);
	}
	buff = ptr;
	memset(buff, 0, TOTAL_MEMORY_SIZE);
	
	page_map(SET);
	
	struct sigaction sig;
	sig.sa_sigaction = signalFunction; // Callback function
	sig.sa_flags = SA_SIGINFO;

	
	sigaction(SIG_TEST, &sig, NULL);
	
	
	/*
	 * 		placeholder-1
	 * register me with the kernel ballooning subsystem
	 */
	check = sigballoon_syscall();
	
	/* test-case */
	test_case_main(buff, TOTAL_MEMORY_SIZE);

	munmap(ptr, TOTAL_MEMORY_SIZE);
	printf("I received SIGBALLOON %lu times\n", nr_signals);
}

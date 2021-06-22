#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <linux/kthread.h>
#include <linux/swap.h>
#include <linux/list.h>
#include <linux/page_idle.h>
#include <../mm/internal.h>


#define SIG_TEST 42


static struct task_struct *thread_t;
struct kernel_siginfo info;
struct task_struct *t;

static int thread_fn(void *unused)
	{
    		while (1)
    		{
    			if (si_mem_available() < (1UL * 1024 * 1024 * 1024))
    			{
        			if(send_sig_info(SIG_TEST, &info, t)<0) // send signal
        			{ 
        				goto exit;
        			}
        		}
        		ssleep(10);
    		}
    		exit:
    		do_exit(0);
    		return 0;
	}
	


SYSCALL_DEFINE0(sigballoon)

{
	
	
	
	clear_siginfo(&info); //clean the stack (help during corruption)
	
	
	
	info.si_signo = SIG_TEST;
	info.si_code = SI_USER;
	info.si_int = 12;
	t = current;
	
	
	thread_t = kthread_run(thread_fn, NULL, "sigball");
	
  return 0;
}

SYSCALL_DEFINE2(swap, unsigned long long __user *, arg, int, n)

{

	unsigned long long buf[100];
	unsigned long long *p;
	int i = 0;
	unsigned long long copied = 0;
	struct page *page = NULL;
	LIST_HEAD(page_list);
		
	copied = copy_from_user(buf, arg, sizeof(buf));
	if (copied < 0 || copied == sizeof(buf))
    return -EFAULT; 
  
  //printk("size of buf %ld", sizeof(buf)); 
  p = buf;
  
  while(i<n)
  {
  	
  	page = pfn_to_page(*p);
  	
  	ClearPageReferenced(page);
		test_and_clear_page_young(page);
		
		if (!isolate_lru_page(page)) {
				if (PageUnevictable(page))
					putback_lru_page(page);
				else
					list_add(&page->lru, &page_list);
			}
		
  	p++;
  	i++;
  }

  reclaim_pages(&page_list);

  return 0;
}

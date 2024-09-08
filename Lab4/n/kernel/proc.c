
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include <stdio.h>
#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

/*======================================================================*
                              schedule 进程调度
 *======================================================================*/
PUBLIC void schedule()
{
    // Read&Write
//    while (1) {
//        p_proc_ready++;
//
//        if (p_proc_ready >= proc_table + NR_TASKS + NR_PROCS) {
//            p_proc_ready = proc_table;
//        }
//        if (p_proc_ready->blocked == 0 && p_proc_ready->wake_tick <= get_ticks()) {
//            break; // 寻找到进程
//        }
//    }

    // Produce & Consume
    PROCESS *p = proc_table + NR_TASKS + NR_PROCS - 1;
    if (p->blocked == 0 && p->wake_tick <= get_ticks()) {
        p_proc_ready = p;
    }else {
        while (1) {
            p_proc_ready++;

            if (p_proc_ready >= proc_table + NR_TASKS + NR_PROCS - 1) {
                p_proc_ready = proc_table;
            }
            if (p_proc_ready->blocked == 0 && p_proc_ready->wake_tick <= get_ticks()) {
                break; // 寻找到进程
            }
        }
    }

}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

PUBLIC int sys_sleep(int milli_sec){
    p_proc_ready->wake_tick = get_ticks() + (milli_sec / (1000 / HZ));
    schedule();
    return 0;
}

PUBLIC int sys_print_str(char* str, int len)
{
    int i = 0;
    for (; i < len; i++) {
        out_char_color(console_table, *(str+i), out_color);
    }

    return 0;
}


PUBLIC int sys_P(SEMAPHORE* s){
    disable_int();//原語不能被中斷
    s->value = s->value - 1;
    if (s->value < 0){
        //进程将被阻塞，直到资源可用为止
        p_proc_ready->blocked = 1;
        s->p_list[s->size] = p_proc_ready;
        s->size++;
        schedule(); // 调度 other 进程
    }
    enable_int();
    return 0;
}



PUBLIC int sys_V(SEMAPHORE* s){
    disable_int();
    s->value++;
    if (s->value <= 0){
        s->p_list[0]->blocked = 0;
        int i = 0;
        for (; i < s->size-1; i++) {
            s->p_list[i] = s->p_list[i+1];
        }
        s->size --;
    }
    enable_int();
    return 0;
}

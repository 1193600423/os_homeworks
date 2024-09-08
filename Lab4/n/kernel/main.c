
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
// Stratagem
PUBLIC int stratagem = 0;
//R&W
PUBLIC int n2;
PUBLIC int t2;
PUBLIC int readCount;
PUBLIC int writeCount;

//P&C
PUBLIC int total;
PUBLIC int P1;
PUBLIC int P2;
PUBLIC int C1;
PUBLIC int C2;
PUBLIC int C3;

PRIVATE void read(int No, int take_time);
PRIVATE void write(int No, int take_time);
PRIVATE void wf_read(int No, int take_time);
PRIVATE void wf_write(int No, int take_time);
PRIVATE void rf_read(int No, int take_time);
PRIVATE void rf_write(int No, int take_time);

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
        u8              privilege;
        u8              rpl;
        int             eflags;
	for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
                if (i < NR_TASKS) {     /* 任务 */
                        p_task    = task_table + i;
                        privilege = PRIVILEGE_TASK;
                        rpl       = RPL_TASK;
                        eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
                }
                else {                  /* 用户进程 */
//                        p_task    = rw_proc_table + (i - NR_TASKS);
                        p_task    = pc_proc_table + (i - NR_TASKS);
                        privilege = PRIVILEGE_USER;
                        rpl       = RPL_USER;
                        eflags    = 0x202; /* IF=1, bit 2 is always 1 */
                }

		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

        p_proc->wake_tick=0;
        p_proc->blocked=0;

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

    // initialize
    // Stratagem
    stratagem = 1;

    //R&W
    n2 = 2;
    t2 = 0;
    readCount = 0;
    writeCount = 0;

    rmutex.value = 1; //只允许1个读者handle readCount
    wmutex.value = 1; //只允许1个写者写
    readTogether.value = n2; //允许𝑛个读者同时读⼀本书
    x.value = 1;
    y.value = 1;
    z.value = 1;
    
    rmutex.size = 0;
    wmutex.size = 0;
    readTogether.size = 0;
    x.size = 0;
    y.size = 0;
    z.size = 0;

    extra.size=0;
    extra.value=1;

    // P&C
    total = 3;
    P1 = 0;
    P2 = 0;
    C1 = 0;
    C2 = 0;
    C3 = 0;

    mutex.value = 1;
    sputTotal.value = total;
    sget1.value = 0;
    sget2.value = 0;

    mutex.size = 0;
    sputTotal.size = 0;
    sget1.size = 0;
    sget2.size = 0;

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
        init_keyboard();

	restart();

	while(1){}
}

/*======================================================================*
                               R&W 进程体
 *======================================================================*/
void RWA(){
    char* enter = "\n";
    char* tmp1 = " :";
    char* tmp2 = ":";
    while (1){
        //序号
        out_color = 0x07;
        int No = 1 + (get_ticks() / 100);
        if ( No <= 20) {
            disp_int(No);
            if (No<10)
                print_str(tmp1, 2);
            else
                print_str(tmp2, 1);

            for (int j = 1; j <= 5; j++) {
                char ch = *(proc_condition+j);
                if (ch == 'O') //正在读/写
                    out_color = 0x02;
                else if (ch == 'X') //等待读/写
                    out_color = 0x04;
                else if (ch == 'Z') //休息
                    out_color = 0x01;
                print_str(&ch, 1);
            }
            print_str(enter, 1);
            sleep(1*TIME_SLICE); //每个时间⽚输出每个读者写者的状态
        }
    }
}

void Reader1(){
    while (1) {
//        rf_read(1, 2);
//        wf_read(1, 2);
        read(1, 2);
    }
}
void Reader2(){
    while (1){
//        rf_read(2, 3);
//        wf_read(2, 3);
        read(2, 3);
    }
}
void Reader3(){
    while (1){
//        rf_read(3, 3);
//        wf_read(3, 3);
        read(3, 3);
    }
}
void Writer1(){
    while (1){
//        rf_write(4, 3);
//        wf_write(4, 3);
        write(4, 3);
    }
}
void Writer2(){
    while (1){
//        rf_write(5, 4);
//        wf_write(5, 4);
        write(5, 4);
    }
}

PRIVATE void rf_read(int No, int take_time){
    *(proc_condition+No) = 'X';

    P(&readTogether);
    P(&rmutex);
    if (readCount == 0) P(&wmutex);
    readCount++;

    V(&rmutex);

    // read
    *(proc_condition+No) = 'O';
    milli_delay(take_time* TIME_SLICE);

    P(&rmutex);
    readCount--;
    V(&readTogether);
    if (readCount == 0) V(&wmutex);
    V(&rmutex);

    *(proc_condition+No) = 'Z';
    sleep(t2*TIME_SLICE);
}
PRIVATE void rf_write(int No, int take_time){
    *(proc_condition+No) = 'X';
//    P(&extra);
    P(&wmutex);

    //write
    *(proc_condition+No) = 'O';
    milli_delay(take_time*TIME_SLICE);

    V(&wmutex);
//    V(&extra);
    *(proc_condition+No) = 'Z';
    sleep(t2*TIME_SLICE);
}
PRIVATE void wf_read(int No, int take_time){
    *(proc_condition+No) = 'X';
    P(&z);
    P(&readTogether);
    P(&rmutex);
    P(&x);
    readCount++;
    if (readCount==1) P(&wmutex);
    V(&x);
    V(&rmutex);
    V(&z);

    // read;
    *(proc_condition+No) = 'O';
    milli_delay(take_time*TIME_SLICE);

    P(&x);
    readCount--;
    V(&readTogether);
    if (readCount==0) V(&wmutex);
    V(&x);
    *(proc_condition+No) = 'Z';
    sleep(t2*TIME_SLICE);
}
PRIVATE void wf_write(int No, int take_time){
    *(proc_condition+No) = 'X';
    P(&y);
    writeCount++;
    if (writeCount==1) P(&rmutex);
    V(&y);

    P(&wmutex);
    //write;
    *(proc_condition+No) = 'O';
    milli_delay(take_time*TIME_SLICE);
    V(&wmutex);

    P(&y);
    writeCount--;
    if (writeCount==0) V(&rmutex);
    V(&y);
    *(proc_condition+No) = 'Z';
    sleep(t2*TIME_SLICE);
}
PRIVATE void read(int No, int take_time) {
    *(proc_condition+No) = 'X';
    P(&x);
    P(&rmutex);
    if (readCount==0) P(&wmutex);
    readCount++;
    V(&rmutex);
    V(&x);

    //读文件
    *(proc_condition+No) = 'O';
    milli_delay(take_time*TIME_SLICE);

    P(&rmutex);
    readCount--;
    if(readCount==0) V(&wmutex);
    V(&rmutex);

    *(proc_condition+No) = 'Z';
    sleep(t2*TIME_SLICE);
}
PRIVATE void write(int No, int take_time) {
    *(proc_condition+No) = 'X';
    P(&x);
    P(&wmutex);

    //写文件
    *(proc_condition+No) = 'O';
    milli_delay(take_time*TIME_SLICE);

    V(&wmutex);
    V(&x);

    *(proc_condition+No) = 'Z';
    sleep(t2*TIME_SLICE);
}

/*======================================================================*
                               P&C 进程体
 *======================================================================*/
void PCA(){
    char* enter = "\n";
    char* tmp1 = " :";
    char* tmp2 = ":";
    while (1){
        //序号
        int No = 1 + (get_ticks() / 100);
        if ( No <= 20) {
            disp_int(No);
            if (No<10)
                print_str(tmp1, 2);
            else
                print_str(tmp2, 1);

            disp_int(P1);
            disp_int(P2);
            disp_int(C1);
            disp_int(C2);
            disp_int(C3);

            print_str(enter, 1);
            sleep(TIME_SLICE); //每个时间⽚输出每个读者写者的状态
        }
    }
}
void Producer1(){
    while(1) {
        P(&sputTotal);
        P(&mutex);
        P1++;
        sleep(TIME_SLICE);
        V(&mutex);
        V(&sget1);
    }
}
void Producer2(){
    while(1) {
        P(&sputTotal);
        P(&mutex);
        P2++;
        sleep(TIME_SLICE);
        V(&mutex);
        V(&sget2);
    }
}
void Consumer1(){
    while(1) {
        P(&sget1);
        P(&mutex);
        C1++;
        sleep(TIME_SLICE);
        V(&mutex);
        V(&sputTotal);
    }
}
void Consumer2(){
    while(1) {
        P(&sget2);
        P(&mutex);
        C2++;
        sleep(TIME_SLICE);
        V(&mutex);
        V(&sputTotal);
    }
}
void Consumer3(){
    while(1) {
        P(&sget2);
        P(&mutex);
        C3++;
        sleep(TIME_SLICE);
        V(&mutex);
        V(&sputTotal);
    }
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	int i = 0;
    char *str = "ABC";
//    sleep(20*1000);
//    print_str(str, 3);
    while (1) {

//        print_str(str, 3);
		milli_delay(10);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	int i = 0x1000;
	while(1){
//		disp_str("B.");
		milli_delay(10);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestC()
{
	int i = 0x2000;
	while(1){
		/* disp_str("C."); */
		milli_delay(10);
	}
}

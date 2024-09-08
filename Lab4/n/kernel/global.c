
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "global.h"
#include "proto.h"


PUBLIC	PROCESS	proc_table[NR_TASKS + NR_PROCS];

PUBLIC	TASK	task_table[NR_TASKS] = {
	{task_tty, STACK_SIZE_TTY, "tty"}};

PUBLIC  TASK    test_proc_table[NR_PROCS] = {
	{TestA, STACK_SIZE_TESTA, "TestA"},
	{TestB, STACK_SIZE_TESTB, "TestB"},
	{TestC, STACK_SIZE_TESTC, "TestC"}};

PUBLIC  TASK    rw_proc_table[NR_PROCS] = {{Reader1, STACK_SIZE_PROC, "Reader1"},
                                           {Reader2, STACK_SIZE_PROC, "Reader2"},
                                           {Reader3, STACK_SIZE_PROC, "Reader3"},
                                           {Writer1, STACK_SIZE_PROC, "Writer1"},
                                           {Writer2, STACK_SIZE_PROC, "Writer2"},
                                           {RWA, STACK_SIZE_PROC, "RWA"}};

PUBLIC  TASK    pc_proc_table[NR_PROCS] = {{Producer1, STACK_SIZE_PROC, "Producer1"},
                                           {Producer2, STACK_SIZE_PROC, "Producer2"},
                                           {Consumer1, STACK_SIZE_PROC, "Consumer1"},
                                           {Consumer2, STACK_SIZE_PROC, "Consumer2"},
                                           {Consumer3, STACK_SIZE_PROC, "Consumer3"},
                                           {PCA, STACK_SIZE_PROC, "PCA"}};


PUBLIC	char		task_stack[STACK_SIZE_TOTAL];

PUBLIC	TTY		tty_table[NR_CONSOLES];
PUBLIC	CONSOLE		console_table[NR_CONSOLES];

PUBLIC	irq_handler	irq_table[NR_IRQ];

PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {sys_get_ticks,
                                                        sys_sleep,
                                                        sys_print_str,
                                                        sys_P,
                                                        sys_V};

PUBLIC SEMAPHORE rmutex;
PUBLIC SEMAPHORE wmutex;
PUBLIC SEMAPHORE readTogether;
PUBLIC SEMAPHORE x;
PUBLIC SEMAPHORE y;
PUBLIC SEMAPHORE z;

PUBLIC SEMAPHORE extra;

PUBLIC SEMAPHORE mutex;
PUBLIC SEMAPHORE sputTotal;
PUBLIC SEMAPHORE sget1;
PUBLIC SEMAPHORE sget2;

PUBLIC char proc_condition[NR_PROCS] = {'Z','Z','Z','Z','Z','Z'};
PUBLIC int out_color = 0x07;

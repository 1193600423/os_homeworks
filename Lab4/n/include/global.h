
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int		ticks;

EXTERN	int		disp_pos;
EXTERN	u8		gdt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	DESCRIPTOR	gdt[GDT_SIZE];
EXTERN	u8		idt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	GATE		idt[IDT_SIZE];

EXTERN	u32		k_reenter;

EXTERN	TSS		tss;
EXTERN	PROCESS*	p_proc_ready;

EXTERN	int		nr_current_console;

extern	PROCESS		proc_table[];
extern	char		task_stack[];
extern  TASK            task_table[];
extern  TASK            test_proc_table[];
extern  TASK            rw_proc_table[];
extern  TASK            pc_proc_table[];
extern	irq_handler	irq_table[];
extern	TTY		tty_table[];
extern  CONSOLE         console_table[];

extern SEMAPHORE rmutex;
extern SEMAPHORE wmutex;
extern SEMAPHORE readTogether;
extern SEMAPHORE x;
extern SEMAPHORE y;
extern SEMAPHORE z;

extern SEMAPHORE extra;

extern SEMAPHORE mutex;
extern SEMAPHORE sputTotal;
extern SEMAPHORE sget1;
extern SEMAPHORE sget2;

extern char  proc_condition[];
extern int out_color;




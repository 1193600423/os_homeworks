
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);
PUBLIC void disable_int();
PUBLIC void enable_int();

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);
PUBLIC void    disp_int(int input);
PUBLIC char * itoa(char * str, int num);

/* kernel.asm */
void restart();

/* main.c */
void TestA();
void TestB();
void TestC();
void RWA();
void Reader1();
void Reader2();
void Reader3();
void Writer1();
void Writer2();
void PCA();
void Producer1();
void Producer2();
void Consumer1();
void Consumer2();
void Consumer3();


/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);
PUBLIC void milli_delay(int milli_sec);
PUBLIC void init_clock();

/* keyboard.c */
PUBLIC void init_keyboard();
PUBLIC void keyboard_read(TTY* p_tty);

/* tty.c */
PUBLIC void task_tty();
PUBLIC void in_process(TTY* p_tty, u32 key);

/* console.c */
PUBLIC void out_char(CONSOLE* p_con, char ch);
PUBLIC void out_char_color(CONSOLE* p_con, char ch, int color);
PUBLIC void scroll_screen(CONSOLE* p_con, int direction);


/* 以下是系统调用相关 */

/* proc.c */
PUBLIC  void    schedule();
PUBLIC  int     sys_get_ticks();        /* sys_call */
PUBLIC  int     sys_sleep(int milli_sec);
PUBLIC  int     sys_print_str(char* str, int len);        /* sys_call */
PUBLIC  int     sys_P(SEMAPHORE* s);
PUBLIC  int     sys_V(SEMAPHORE* s);

/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();
PUBLIC  int     sleep(int milli_sec);
PUBLIC  int     print_str(char* str, int len);
PUBLIC  int     P(SEMAPHORE* s);
PUBLIC  int     V(SEMAPHORE* s);


//

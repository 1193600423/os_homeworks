
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE int OutCharColor = DEFAULT_CHAR_COLOR;
PRIVATE int TabColor = 0x02;
PRIVATE int searchWordStart;

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);
PRIVATE int check(CONSOLE * p_con, int dev, char* text, int pos);

/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
}

/*======================================================================*
			   init_searchWordStart
*======================================================================*/
PUBLIC void init_searchWordStart(CONSOLE* p_con)
{
    searchWordStart = p_con->cursor - p_con->original_addr;
}
/*======================================================================*
			   clear_search
*======================================================================*/
PUBLIC void clear_search(CONSOLE* p_con)
{
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    int i = 0;
    for (; i < p_con->cursor - p_con->original_addr - searchWordStart; i++) {
        *(p_vmem -2*i-2) = '\0';
        *(p_vmem -2*i-1) = OutCharColor;
    }
    p_con->cursor -= i;
}


PUBLIC void colorChange(int color){
	OutCharColor = color;
}

PUBLIC void colorRet(){
	OutCharColor = DEFAULT_CHAR_COLOR;
}

/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

	switch(ch) {
	case '\n':
		if (p_con->cursor < p_con->original_addr +
		    p_con->v_mem_limit - SCREEN_WIDTH) {
			for(int i=0; i<(SCREEN_WIDTH-(p_con->cursor - p_con->original_addr)%SCREEN_WIDTH); i++){
				*(p_vmem+2*i) = '\0';
			}
			p_con->cursor = p_con->original_addr + SCREEN_WIDTH * 
				((p_con->cursor - p_con->original_addr) /
				 SCREEN_WIDTH + 1);
		}
		break;
	case '\b':
		if (p_con->cursor > p_con->original_addr) {
			//edited by zhao
			
			//Tab
			if (p_con->cursor > p_con->original_addr+3) {
				if (*(p_vmem-2) == ' ' && *(p_vmem-4) == ' ' && *(p_vmem-6) == ' ' && *(p_vmem-8) == ' '
                  && *(p_vmem-1) == TabColor && *(p_vmem-3) == TabColor && *(p_vmem-5) == TabColor && *(p_vmem-7) == TabColor){
					p_con->cursor-=4;
                    *(p_vmem-2) = '\0';
                    *(p_vmem-4) = '\0';
                    *(p_vmem-6) = '\0';
                    *(p_vmem-8) = '\0';
					*(p_vmem-1) = OutCharColor;
					*(p_vmem-3) = OutCharColor;
					*(p_vmem-5) = OutCharColor;
					*(p_vmem-7) = OutCharColor;
					break;
				}
			}
			p_con->cursor--;
			*(p_vmem-2) = '\0';
			*(p_vmem-1) = OutCharColor;
			int t = 0;
            // TODO
			while(*(p_vmem-2*t-2) == '\0' && t < SCREEN_WIDTH) {
				t++;
			}
			p_con->cursor -= t-1;
		}
		break;
	default:
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;
			*p_vmem++ = OutCharColor;
			p_con->cursor++;
		}
		break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

/*======================================================================*
			   clear_line
 *----------------------------------------------------------------------*
 清空一行  
 *----------------------------------------------------------------------*
 
 *======================================================================*/
PUBLIC void clear_line(CONSOLE* p_con)
{
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

    if (p_con->cursor > p_con->original_addr) {
        int i =0;
        for(; i< (p_con->cursor - p_con->original_addr)%SCREEN_WIDTH; i++){
            *(p_vmem-2*i-2) = '\0';
        }
        p_con->cursor -= i;
    }
}

/*======================================================================*
			  clear_screen
 *----------------------------------------------------------------------*
 *----------------------------------------------------------------------*

 *======================================================================*/
PUBLIC void clear_screen() {
    for (int i = 0; i < NR_CONSOLES ; i++) {
        CONSOLE* p_con = console_table + i;
        u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
        int i=0;
        for(; i < (p_con->cursor - p_con->original_addr); i++){
            *(p_vmem-2*i-2) = '\0';
            *(p_vmem-2*i-1) = OutCharColor;
        }
        p_con->cursor -= i;
    }
}

/*======================================================================*
			   search_highlight
 *----------------------------------------------------------------------*
 按回⻋后，所有匹配的文本 (区分大小写) 以红色显示
 *----------------------------------------------------------------------*
 
 *======================================================================*/
PUBLIC void search_highlight(CONSOLE* p_con)
{
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

    // init Text
    char text[1024];
    int textLen = 0;
    while (textLen < p_con->cursor- p_con->original_addr) {
        if (*(p_vmem - 1 - 2*textLen) == TabColor) {
            text[textLen] = '\t';
            textLen ++;
            continue;
        }
        if (*(p_vmem - 1 - 2*textLen) != OutCharColor){
            break;
        }
        char c = *(p_vmem - 2 - 2* textLen);
        if (c == '\0') break;
        text[textLen] = c;
        textLen ++;
    }
    text[textLen] = '\0';
    //for and check
    for (int pos = textLen; pos < (p_con->cursor- p_con->original_addr); pos++) {
        if (check(p_con, 0, text, pos)) {
            for (int i=0; i<textLen; i++){
                *(p_vmem-1-2*pos-2*i) = OutCharColor;
            }
        }
    }

}

/*======================================================================*
			   color_recover
 *----------------------------------------------------------------------*
 所有文本恢复白颜色
 *----------------------------------------------------------------------*

 *======================================================================*/
PUBLIC void color_recover(CONSOLE* p_con) {
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    for (int i = 0; i < (p_con->cursor- p_con->original_addr); i++) {
        *(p_vmem-1) = (*(p_vmem-1) != TabColor)? OutCharColor: *(p_vmem-1);
        p_vmem-=2;
    }
}

PRIVATE int check(CONSOLE * p_con, int dev, char* text, int pos){
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

    char t = *(text+dev);
    if (t == '\0') return 1;

    char p = *(p_vmem - 2* pos -2*dev - 2);
    int color = *(p_vmem - 2* pos -2*dev - 1);
    if (t == '\t')  {
        if (p != ' ' || color != TabColor) {
            return 0;
        }
    } else if (t != p) return 0;
    return check(p_con, dev+1, text, pos);
}
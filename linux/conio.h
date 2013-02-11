#ifndef __linux__
#error This conio.h was only tested to work under LINUX !
#endif

#ifndef __LINUXCONIO_H
#define __LINUXCONIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <curses.h>

enum COLORS {
   BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHTGRAY,
   DARKGRAY, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED, LIGHTMAGENTA, YELLOW, WHITE
};

#define BLINK 128    

extern void initconio(void); /* Please run this function before any other */ 
extern void doneconio(void); /* Please run this function before exiting your program */

extern int wherex(void);
extern int wherey(void);
extern int putch(int c); 
extern int kbhit(void);
extern int cprintf(const char *format, ...);
extern int cscanf(const char *format, ...); 

extern void clrscr(void);
extern void gotoxy(int x, int y);
extern void textbackground(int color);
extern void textcolor(int color);
extern void textmode(int unused_mode);
extern void cputs(char *str);

extern int  cgets(char *str);

#ifdef  __cplusplus
}
#endif

#endif










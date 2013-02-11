#include <curses.h>
#include <ctype.h>
#include "conio.h"

WINDOW *conioscr;
int bgc,fgc;
int txtattr,oldattr;

int colortab(int a) /* convert LINUX Color code to DOS-standard */
{
   switch(a) {
      case 0 : return COLOR_BLACK;
      case 1 : return COLOR_BLUE;
      case 2 : return COLOR_GREEN;
      case 3 : return COLOR_CYAN;
      case 4 : return COLOR_RED;
      case 5 : return COLOR_MAGENTA;
      case 6 : return COLOR_YELLOW;
      case 7 : return COLOR_WHITE;
   }
} 

void docolor (int color) /* Set DOS-like text mode colors */
{
   wattrset(stdscr,0); 
   if ((color&128)==128) 
     txtattr=A_BLINK; 
   else 
     txtattr=A_NORMAL;
   if ((color&15)>7) 
     txtattr|=A_STANDOUT; 
   else 
     txtattr|=A_NORMAL;
   
   txtattr|=COLOR_PAIR((1+(color&7)+(color&112)) >> 1);
   if (((color&127)==15) || ((color&127)==7)) 
     txtattr=COLOR_PAIR(0);
   if (((color&127)==127) || ((color&127)==119)) 
     txtattr=COLOR_PAIR(8);
   wattron(stdscr,txtattr);
   wattron(stdscr,COLOR_PAIR(1+(color&7)+((color&112)>>1))); 
}

void initconio(void)

{
  int x,y;

  initscr();
  start_color();
  cbreak();
  keypad(stdscr,TRUE);
  scrollok(stdscr,TRUE);
  nonl();
  noecho();
  for (y=0;y<=7;y++)
      for (x=0;x<=7;x++)
         init_pair((8*y)+x+1, colortab(x), colortab(y));
  fgc = LIGHTGRAY;
  bgc = BLACK;
}

void doneconio(void)

{
  endwin();
}

int _getch(void)

{
  int ch;

  ch = wgetch(stdscr);
  if((ch == 127) || (ch == 263))
    return 8;
  if(ch == 27)
    return ((0xFF & toupper(wgetch(stdscr))) | 0x1B00);
  return ch;
}

void newgets(char *str)

{
   char *tempstr;
   unsigned short ch;

   tempstr = str;
   ch = _getch();
   while(ch != 13)
   {
      if((ch >= 32) && (ch < 256))
      {
      	 putch(ch);  
         *tempstr = ch & 0xff;
         tempstr++;
      }
      if(ch == 8)
      {
         if(tempstr > str)
         {
	   gotoxy(wherex()-1,wherey());
	   delch();
           tempstr--;
         }
      }
      ch = _getch();
   }
   *tempstr = 0;
   cprintf("\n");
}

void clrscr (void)
{
   wclear(stdscr);
   wmove(stdscr,0,0);
   wrefresh(stdscr);
}

void textbackground (int color)
{
   bgc=color;
   color=(bgc*16)+fgc;
   docolor(color);
}

void textcolor (int color)
{
   fgc=color;
   color=(bgc*16)+fgc;
   docolor(color);
}

int wherex (void)
{
   int y;
   int x;

   getyx(stdscr,y,x);
   x++;
   return(x);
}

int wherey (void)
{
   int y;
   int x;
  
   getyx(stdscr,y,x);
   y++;
   return(y);
}

void gotoxy (int x, int y)
{
   y--;
   x--;
   wmove(stdscr,y,x);
   wrefresh(stdscr);
}

int cprintf (const char *format, ... )
{
   int i;
   char buffer[BUFSIZ]; /* Well, BUFSIZ is from ncurses...  */
   va_list argp;
   
   va_start(argp,format);
   vsprintf(buffer,format,argp);
   va_end(argp);
   i=waddstr(stdscr,buffer);
   //wrefresh(stdscr);
   return(i);
}

int cgets(char *str)

{
   newgets(str);
   return 0;
}

void cputs (char *str)
{
   waddstr(stdscr,str);
   wrefresh(stdscr);
}

int cscanf (const char *format, ...)
{
   int i;
   char buffer[BUFSIZ]; /* See cprintf */
   va_list argp;

   newgets(buffer);                    
   va_start(argp,format);
   i=vsscanf(buffer,format,argp);                         
   va_end(argp);
  
   return(i);
}

int kbhit (void)
{
   int i;
   
   nodelay(stdscr,TRUE);
   i=wgetch(stdscr);
   nodelay(stdscr,FALSE);
   if (i==-1) i=0;
   return(i);
}

int putch (int c)
{
   
   if (waddch(stdscr,c)!=ERR) {
     //wrefresh(stdscr); 
      return(c);
   }
   return(0);
}
/*
int main(void)

{ 
  int ch;
  int loop;
  char str[50];

  initconio();

  textcolor(LIGHTGRAY);
  textbackground(BLUE);
 
  cprintf("Plz enter number->");
  cgets(str);
  _getch();
  
  doneconio();

  printf("Character typed = %ld\n",ch);
  return 0;
}

*/

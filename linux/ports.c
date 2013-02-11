#include <unistd.h>
#include "ports.h"

int inportb(int port)
{
   unsigned char value;
  __asm__ volatile ("inb %1,%0"
                    : "=a" (value)
                    : "d" ((unsigned short) port));
   return value;
}

void outportb(unsigned short int port, unsigned char val)
{
  __asm__ volatile (
                    "outb %0,%1\n"
                    :
                    : "a" (val), "d" (port)
                    );
}


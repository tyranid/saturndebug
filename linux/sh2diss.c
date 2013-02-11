#include <stdio.h>
#include "conio.h"
#include "sh2diss.h"

unsigned long signex8(unsigned long x)

{
     if((x & 0x80) == 0)
       return x & 0x000000FF;
     else
       return x | 0xFFFFFF00;
}

unsigned long signex12(unsigned long x)

{
     if((x & 0x800) == 0)
       return x & 0x00000FFF;
     else
       return x | 0xFFFFF000;
}


const char *code0000a[16] =
     { "%08lX : ??????  ???????        : %lX%lX %lX",
       "%08lX : ??????  ???????        : %lX%lX %lX",
       "%08lX : ??????  ???????        : %lX%lX %lX",
       "%08lX : ??????  ???????        : %lX%lX %lX",
       "%08lX : MOV.B   R%1X,@(R0,R%1X) : %04lX",
       "%08lX : MOV.W   R%1X,@(R0,R%1X) : %04lX",
       "%08lX : MOV.L   R%1X,@(R0,R%1X) : %04lX",
       "%08lX : MUL.L   R%1X,R%1X       : %04lX",
       "%08lX : ??????  ?????????      : %lX%lX %04lX",
       "%08lX : ??????  ?????????      : %lX%lX %04lX",
       "%08lX : ??????  ?????????      : %lX%lX %04lX",
       "%08lX : ??????  ?????????      : %lX%lX %04lX",
       "%08lX : MOV.B   @(R0,R%1X),R%1X : %04lX",
       "%08lX : MOV.W   @(R0,R%1X),R%1X : %04lX",
       "%08lX : MOV.L   @(R0,R%1X),R%1X : %04lX",
       "%08lX : MAC.L   @R%1X+,@R%1X+   : %04lX" };


const char *code0010a[16] =
     { "%08lX : MOV.B   R%1X,@R%1X         : %lX",
       "%08lX : MOV.W   R%1X,@R%1X         : %lX",
       "%08lX : MOV.L   R%1X,@R%1X         : %lX",
       "%08lX : ?????   ???????           : %lX%lX %lX",
       "%08lX : MOV.B   R%1X,@-R%1X        : %lX",
       "%08lX : MOV.W   R%1X,@-R%1X        : %lX",
       "%08lX : MOV.L   R%1X,@-R%1X        : %lX",
       "%08lX : DIV0S   R%1X,R%1X          : %lX",
       "%08lX : TST     R%1X,R%1X          : %lX",
       "%08lX : AND     R%1X,R%1X          : %lX",
       "%08lX : XOR     R%1X,R%1X          : %lX",
       "%08lX : OR      R%1X,R%1X          : %lX",
       "%08lX : CMP/STR R%1X,R%1X          : %lX",
       "%08lX : XTRCT   R%1X,R%1X          : %lX",
       "%08lX : MULU.W  R%1X,R%1X          : %lX",
       "%08lX : MULS.W  R%1X,R%1X          : %lX" };

const char *code0011a[16] =
      {
       "%08lX : CMP/EQ  R%1X,R%1X          : %lX",
       "%08lX : ??????  ?????             : %lX%lX %lX",
       "%08lX : CMP/HS  R%1X,R%1X          : %lX",
       "%08lX : CMP/GE  R%1X,R%1X          : %lX",
       "%08lX : DIV1    R%1X,R%1X          : %lX",
       "%08lX : DMULU.L R%1X,R%1X          : %lX",
       "%08lX : CMP/HI  R%1X,R%1X          : %lX",
       "%08lX : CMP/GT  R%1X,R%1X          : %lX",
       "%08lX : SUB     R%1X,R%1X          : %lX",
       "%08lX : ??????? ?????             : %lX%lX %lX",
       "%08lX : SUBC    R%1X,R%1X          : %lX",
       "%08lX : SUBV    R%1X,R%1X          : %lX",
       "%08lX : ADD     R%1X,R%1X          : %lX",
       "%08lX : DMULS.L R%1X,R%1X          : %lX",
       "%08lX : ADDC    R%1X,R%1X          : %lX",
       "%08lX : ADDV    R%1X,R%1X          : %lX" };

#define Rn ((opcode & 0xF00) >> 8)
#define Rm ((opcode & 0xF0) >> 4)

void code0000(unsigned short opcode,int address)

{
     switch(opcode & 0x3F)
       {
          case 0x8 : cprintf("%08lX : CLRT                   : %04lX",address,opcode);
                     break;
          case 0x9 : cprintf("%08lX : NOP                    : %04lX",address,opcode);
                     break;
          case 0xB : cprintf("%08lX : RTS                    : %04lX",address,opcode);
                     break;
          case 0x18: cprintf("%08lX : SETT                   : %04lX",address,opcode);
                     break;
          case 0x19: cprintf("%08lX : DIV0U                  : %04lX",address,opcode);
                     break;
          case 0x1B: cprintf("%08lX : SLEEP                  : %04lX",address,opcode);
                     break;
          case 0x28: cprintf("%08lX : CLRMAC                 : %04lX",address,opcode);
                     break;
          case 0x2B: cprintf("%08lX : RTE                    : %04lX",address,opcode);
                     break;
          case 0x2 : cprintf("%08lX : STC SR,R%X               : %04lX",address,Rn,opcode);
                     break;
          case 0x3 : cprintf("%08lX : BSRF R%lX                : %04lX",address,Rn,opcode);
                     break;
          case 0xA : cprintf("%08lX : STS MACH,R%lX            : %04lX",address,Rn,opcode);
                     break;
          case 0x12: cprintf("%08lX : STS GBR,R%lX             : %04lX",address,Rn,opcode);
                     break;
          case 0x1A: cprintf("%08lX : STS MACL,R%lX            : %04lX",address,Rn,opcode);
                     break;
          case 0x22: cprintf("%08lX : STC VBR,R%lX             : %04lX",address,Rn,opcode);
                     break;
          case 0x23: cprintf("%08lX : BRAF R%lX                : %04lX",address,Rn,opcode);
                     break;
          case 0x29: cprintf("%08lX : MOVT R%lX                : %04X",address,Rn,opcode);
                     break;
          case 0x2A: cprintf("%08lX : STS PR,R%lX              : %04lX",address,Rn,opcode);
                     break;
          default :
               cprintf(code0000a[opcode & 0xF],address,Rm,Rn,opcode);
       }
}

void code0001(unsigned short opcode,int address)

{
     cprintf("%08lX : MOV.L  R%lX,@(%lX,R%lX)     : %lX",address,Rm,(opcode & 0xF) << 2,Rn,opcode);
}

void code0010(unsigned short opcode,int address)

{
     cprintf(code0010a[opcode & 0xF],address,Rm,Rn,opcode);
}

void code0011(unsigned short opcode,int address)

{
     cprintf(code0011a[opcode & 0xF],address,Rm,Rn,opcode);
}

void code0100(unsigned short opcode,int address)

{
     switch(opcode & 0x3F)
       {
         case 0  : cprintf("%08lX : SHLL R%X                : %04lX",address,Rn,opcode);
                   break;
         case 1  : cprintf("%08lX : SHLR R%X                : %04lX",address,Rn,opcode);
                   break;
         case 2  : cprintf("%08lX : STS.L MACH,@-R%X        : %04lX",address,Rn,opcode);
                   break;
         case 3  : cprintf("%08lX : STC.L SR,@-R%X          : %04lX",address,Rn,opcode);
                   break;
         case 4  : cprintf("%08lX : ROTL R%X                : %04lX",address,Rn,opcode);
                   break;
         case 5  : cprintf("%08lX : ROTR R%X                : %04lX",address,Rn,opcode);
                   break;
         case 6  : cprintf("%08lX : LDS.L @R%X+,MACH        : %04lX",address,Rn,opcode);
                   break;
         case 7  : cprintf("%08lX : LDC.L @R%X+,SR          : %04lX",address,Rn,opcode);
                   break;
         case 8  : cprintf("%08lX : SHLL2 R%X               : %04lX",address,Rn,opcode);
                   break;
         case 9  : cprintf("%08lX : SHLR2 R%X               : %04lX",address,Rn,opcode);
                   break;
         case 10 : cprintf("%08lX : LDS   R%X,MACH          : %04lX",address,Rn,opcode);
                   break;
         case 11 : cprintf("%08lX : JSR   R%X               : %04lX",address,Rn,opcode);
                   break;
         case 14 : cprintf("%08lX : LDC   R%X,SR            : %04lX",address,Rn,opcode);
                   break;
         case 16 : cprintf("%08lX : DT    R%X               : %04lX",address,Rn,opcode);
                   break;
         case 17 : cprintf("%08lX : CMP/PZ R%X              : %04lX",address,Rn,opcode);
                   break;
         case 18 : cprintf("%08lX : STS.L MACL,-R%X         : %04lX",address,Rn,opcode);
                   break;
         case 19 : cprintf("%08lX : STC.L GBR,-R%X          : %04lX",address,Rn,opcode);
                   break;
         case 21 : cprintf("%08lX : CMP/PL R%X              : %04lX",address,Rn,opcode);
                   break;
         case 22 : cprintf("%08lX : STS.L @R%X+,MACL        : %04lX",address,Rn,opcode);
                   break;
         case 23 : cprintf("%08lX : STC.L @R%X+,GBR         : %04lX",address,Rn,opcode);
                   break;
         case 24 : cprintf("%08lX : SHLL8 R%X               : %04lX",address,Rn,opcode);
                   break;
         case 25 : cprintf("%08lX : SHLR8 R%X               : %04lX",address,Rn,opcode);
                   break;
         case 26 : cprintf("%08lX : LDS   R%X,MACL          : %04lX",address,Rn,opcode);
                   break;
         case 27 : cprintf("%08lX : TAS   R%X               : %04lX",address,Rn,opcode);
                   break;
         case 30 : cprintf("%08lX : LDC   R%X,GBR           : %04lX",address,Rn,opcode);
                   break;
         case 32 : cprintf("%08lX : SHAL  R%X               : %04lX",address,Rn,opcode);
                   break;
         case 33 : cprintf("%08lX : SHAR  R%X               : %04lX",address,Rn,opcode);
                   break;
         case 34 : cprintf("%08lX : STS.L PR,@-R%X          : %04lX",address,Rn,opcode);
                   break;
         case 35 : cprintf("%08lX : STC.L VBR,@-R%X         : %04lX",address,Rn,opcode);
                   break;
         case 36 : cprintf("%08lX : ROTCL R%X               : %04lX",address,Rn,opcode);
                   break;
         case 37 : cprintf("%08lX : ROTCR R%X               : %04lX",address,Rn,opcode);
                   break;
         case 38 : cprintf("%08lX : LDS.L @R%X+,PR          : %04lX",address,Rn,opcode);
                   break;
         case 39 : cprintf("%08lX : LDC.L @R%X+,VBR         : %04lX",address,Rn,opcode);
                   break;
         case 40 : cprintf("%08lX : SHLL16 R%X              : %04lX",address,Rn,opcode);
                   break;
         case 41 : cprintf("%08lX : SHLR16 R%X              : %04lX",address,Rn,opcode);
                   break;
         case 42 : cprintf("%08lX : LDS   R%X,PR            : %04lX",address,Rn,opcode);
                   break;
         case 43 : cprintf("%08lX : JMP   R%X               : %04lX",address,Rn,opcode);
                   break;
         case 46 : cprintf("%08lX : LDC   R%X,VBR           : %04lX",address,Rn,opcode);
                   break;
         default : if((opcode & 0xF) == 0xF)
                     cprintf("%08lX : MAC.W @R%X+,@R%X+               : %04lX",address,Rm,Rn,opcode);
                   else
                     cprintf("%08lx ??",address);
       }
}

void code0101(unsigned short opcode,int address)

{
     cprintf("%08lX : MOV.L  @(%02lX,R%lX),R%lX     : %lX",address,(opcode & 0xF) << 2,Rm,Rn,opcode);
}

void code0110(unsigned short opcode,int address)

{
      switch(opcode & 0xF)
       {
          case 0 : cprintf("%08lX : MOV.B   @R%X,R%X         : %04lX",address,Rm,Rn,opcode);
                   break;
          case 1 : cprintf("%08lX : MOV.W   @R%X,R%X         : %04lX",address,Rm,Rn,opcode);
                   break;
          case 2 : cprintf("%08lX : MOV.L   @R%X,R%X         : %04lX",address,Rm,Rn,opcode);
                   break;
          case 3 : cprintf("%08lX : MOV     R%X,R%X          : %04lX",address,Rm,Rn,opcode);
                   break;
          case 4 : cprintf("%08lX : MOV.B   @R%X+,R%X        : %04lX",address,Rm,Rn,opcode);
                   break;
          case 5 : cprintf("%08lX : MOV.W   @R%X+,R%X        : %04lX",address,Rm,Rn,opcode);
                   break;
          case 6 : cprintf("%08lX : MOV.L   @R%X+,R%X        : %04lX",address,Rm,Rn,opcode);
                   break;
          case 7 : cprintf("%08lX : NOT     R%X,R%X          : %04lX",address,Rm,Rn,opcode);
                   break;
          case 8 : cprintf("%08lX : SWAP.B  R%X,R%X          : %04lX",address,Rm,Rn,opcode);
                   break;
          case 9 : cprintf("%08lX : SWAP.W  R%X,R%X          : %04lX",address,Rm,Rn,opcode);
                   break;
          case 10: cprintf("%08lX : NEGC    R%X,R%X          : %04lX",address,Rm,Rn,opcode);
                   break;
          case 11: cprintf("%08lX : NEG     R%X,R%X          : %04lX",address,Rm,Rn,opcode);
                   break;
          case 12: cprintf("%08lX : EXTU.B  R%X,R%X          : %04lX",address,Rm,Rn,opcode);
                   break;
          case 13: cprintf("%08lX : EXTU.W  R%X,R%X          : %04lX",address,Rm,Rn,opcode);
                   break;
          case 14: cprintf("%08lX : EXTS.B  R%X,R%X          : %04lX",address,Rm,Rn,opcode);
                   break;
          case 15: cprintf("%08lX : EXTS.W  R%X,R%X          : %04lX",address,Rm,Rn,opcode);
                   break;
       }
}

void code0111(unsigned short opcode,int address)

{
     cprintf("%08lX : ADD    #%02lX,R%lX          : %lX",address,opcode & 0xFF,Rn,opcode);
}

void code1000(unsigned short opcode,int address)

{
     switch((opcode & 0xF00) >> 8)
       {
         case 0 : cprintf("%08lX : MOV.B  R0,@(%lX,R%X)      : %04lX",address,opcode & 0xF,Rm,opcode);
                  break;
         case 1 : cprintf("%08lX : MOV.W  R0,@(%lX,R%X)      : %04lX",address,(opcode & 0xF) << 1,Rm,opcode);
                  break;
         case 4 : cprintf("%08lX : MOV.B  @(%lX,R%X),R0      : %04lX",address,opcode & 0xF,Rm,opcode);
                  break;
         case 5 : cprintf("%08lX : MOV.W  @(%lX,R%X),R0      : %04lX",address,(opcode & 0xF) << 1,Rm,opcode);
                  break;
         case 8 : cprintf("%08lX : CMP/EQ #%X,R0             : %04lX",address,opcode & 0xFF,opcode);
                  break;
         case 9 : cprintf("%08lX : BT     %08lX        : %04lX",address,signex8(opcode & 0xFF) * 2 + address + 4,opcode);
                  break;
         case 13: cprintf("%08lX : BTS    %08lX        : %04lX",address,signex8(opcode & 0xFF) * 2 + address + 4,opcode);
                  break;
         case 11: cprintf("%08lX : BF     %08lX        : %04lX",address,signex8(opcode & 0xFF) * 2 + address + 4,opcode);
                  break;
         case 15: cprintf("%08lX : BFS    %08lX        : %04lX",address,signex8(opcode & 0xFF) * 2 + address + 4,opcode);
                  break;
         default : cprintf("Invalid opcode");
       }
}

void code1001(unsigned short opcode,int address)

{
     cprintf("%08lX : MOV.W  @(%08lX),R%lX  : %lX",address,(opcode & 0xFF) * 2 + address + 4,Rn,opcode);
}

void code1010(unsigned short opcode,int address)

{
     cprintf("%08lX : BRA    %08lX        : %lX",address,signex12(opcode & 0xFFF) * 2 + address + 4,opcode);
}

void code1011(unsigned short opcode,int address)

{
     cprintf("%08lX : BSR    %08lX        : %lX",address,signex12(opcode & 0xFFF) * 2 + address + 4,opcode);
}

void code1100(unsigned short opcode,int address)

{
     switch((opcode & 0xF00) >> 8)
       {
         case 0 : cprintf("%08lX : MOV.B  R0,@(%02lX,GBR)    : %04lX",address,opcode & 0xFF,opcode);
                  break;
         case 1 : cprintf("%08lX : MOV.W  R0,@(%02lX,GBR)    : %04lX",address,(opcode & 0xFF)<<1,opcode);
                  break;
         case 2 : cprintf("%08lX : MOV.L  R0,@(%04lX,GBR)  : %04lX",address,(opcode & 0xFF)<<2,opcode);
                  break;
         case 3 : cprintf("%08lX : TRAPA  #%02lX             : %04lX",address,opcode & 0xFF,opcode);
                  break;
         case 4 : cprintf("%08lX : MOV.B  @(%04lX,GBR),R0  : %04lX",address,opcode & 0xFF,opcode);
                  break;
         case 5 : cprintf("%08lX : MOV.W  @(%04lX,GBR),R0  : %04lX",address,(opcode & 0xFF)<<1,opcode);
                  break;
         case 6 : cprintf("%08lX : MOV.L  @(%08lX,GBR),R0  : %04lX",address,(opcode & 0xFF)<<2,opcode);
                  break;
         case 7 : cprintf("%08lX : MOVA   @(%08lX),R0     : %04lX",address,(opcode & 0xFF)<<2+address+4,opcode);
                  break;
         case 8 : cprintf("%08lX : TST    #%02lX,R0          : %04lX",address,opcode & 0xFF,opcode);
                  break;
         case 9 : cprintf("%08lX : AND    #%02lX,R0          : %04lX",address,opcode & 0xFF,opcode);
                  break;
         case 10: cprintf("%08lX : XOR    #%02lX,R0          : %04lX",address,opcode & 0xFF,opcode);
                  break;
         case 11: cprintf("%08lX : OR     #%02lX,R0          : %04lX",address,opcode & 0xFF,opcode);
                  break;
         case 12: cprintf("%08lX : TST.B  #%02lX,@(R0,GBR)   : %04lX",address,opcode & 0xFF,opcode);
                  break;
         case 13: cprintf("%08lX : AND.B  #%02lX,@(R0,GBR)   : %04lX",address,opcode & 0xFF,opcode);
                  break;
         case 14: cprintf("%08lX : XOR.B  #%02lX,@(R0,GBR)   : %04lX",address,opcode & 0xFF,opcode);
                  break;
         case 15: cprintf("%08lX : OR.B   #%02lX,@(R0,GBR)   : %04lX",address,opcode & 0xFF,opcode);
                  break;
       }
}

void code1101(unsigned short opcode,int address)

{
     cprintf("%08lX : MOV.L  @(%08lX),R%lX  : %lX",address,(opcode & 0xFF)*4+address+4,Rn,opcode);
}

void code1110(unsigned short opcode,int address)

{
     cprintf("%08lX : MOV    #%02lX,R%lX          : %lX",address,opcode & 0xFF,Rn,opcode);
}

int scanbp(unsigned int addr,const struct breakp *bps)

// scans breakpoints and returns 1 if one matches bps

{
    int loop = 0;

    for(loop = 0;loop < 10;loop++)
     {
        if((bps[loop].addr == addr) && (bps[loop].enable))
          return 1;
     }

    return 0;
}

void disasm(unsigned int startaddr,unsigned int pcaddr,struct breakp *bps,const unsigned short *opcodes)

{
    unsigned int address = startaddr;
    unsigned short temp;
    int loop;
    unsigned int opcode;
    int breakpoint = 0;

    for(loop = 0;loop < 10;loop++)
    {
    opcode = opcodes[loop];

    if(address == pcaddr)
    {
       textcolor(BLACK);
       textbackground(GREEN);
    }
    breakpoint = scanbp(address,bps);
    if(breakpoint)
    {
       textcolor(WHITE);
       textbackground(RED);
    }
    if(loop == 5)
    {
       textcolor(YELLOW);
       textbackground(BLUE);
    }
    gotoxy(1,(address - startaddr) / 2 + 3);

    switch((opcode & 0xF000) >> 12)
    {
           case 0 : code0000(opcode,address);
                    break;
           case 1 : code0001(opcode,address);
                    break;
           case 2 : code0010(opcode,address);
                    break;
           case 3 : code0011(opcode,address);
                    break;
           case 4 : code0100(opcode,address);
                    break;
           case 5 : code0101(opcode,address);
                    break;
           case 6 : code0110(opcode,address);
                    break;
           case 7 : code0111(opcode,address);
                    break;
           case 8 : code1000(opcode,address);
                    break;
           case 9 : code1001(opcode,address);
                    break;
           case 10: code1010(opcode,address);
                    break;
           case 11: code1011(opcode,address);
                    break;
           case 12: code1100(opcode,address);
                    break;
           case 13: code1101(opcode,address);
                    break;
           case 14: code1110(opcode,address);
                    break;
           default : cprintf("%08lx : Opcode unknown",address);
      }
      if((loop == 5) || (address == pcaddr) || (breakpoint))
      {
       textcolor(LIGHTGRAY);
       textbackground(BLACK);
      }
      address += 2;
    }

    cprintf("\n");

}


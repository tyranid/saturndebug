/* Sega Saturn DeBuGgEr Written by TyRaNiD
   PC side of debugger. Where most the the stuff actually goes on
   main.c - Currently contains all the function required except for
   the sh2 disasm.
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <pc.h>
#include <io.h>
#include <ctype.h>
#include <process.h>
#include <dir.h>
#include "sh2diss.h"
#include "iasm.h"
#include "bootload.h"
#define release 1       // 1 means release version 0 means non release version
#define version 0x7a    // Defines version number for printing
typedef unsigned long  LONG;
typedef unsigned short WORD;
typedef unsigned char  BYTE;

struct memarea

{
       LONG saddr,    // Start address of range
            faddr,    // End address of range
            write;    // Write 1 = able 0 = disable
};

struct memarea memareas[] ={ // Cached addresses
                            { 0x06000000,0x06100000,1},// Main WorkRAM
                            { 0x00000000,0x00080000,0},// BIOS
                            { 0x00180000,0x00190000,0},// Backup RAM
                            { 0x00200000,0x00300000,1},// Lower WorkRAM
                            { 0x05C00000,0x05C80000,1},// VDP1 VRAM
                            { 0x05E00000,0x05e80000,1},// VDP2 VRAM
                            { 0x05F00000,0x05F01000,1},// VDP2 CRAM
                            // Cached through addresses
                            { 0x26000000,0x26100000,1},// Main WorkRAM
                            { 0x20000000,0x20080000,0},// BIOS
                            { 0x20180000,0x20190000,0},// Backup RAM
                            { 0x20200000,0x20300000,1},// Lower WorkRAM
                            { 0x25C00000,0x25C80000,1},// VDP1 VRAM
                            { 0x25E00000,0x25e80000,1},// VDP2 VRAM
                            { 0x25F00000,0x25F01000,1},// VDP2 CRAM
                            { 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}};// End

#define flength(fp) (filelength(fileno(fp)))

WORD FlagPort;         // PC port address for busy flag
WORD DataPort;         // PC port address for data port
char loadfile[100];    // load file name.
LONG loadaddr;         // load address.
LONG entryaddr;        // Entry location.
LONG opsran;           // debug variable determines how many ops ran
LONG bytestrans;       // debug variable determines how many ops ran.
LONG iscoff;           // is the load file a coff format file.
LONG toentry;          // r we running to the entry point ?
LONG datel;            // 1 = Using datel PAR 0 = 4in1+
char gaspath[100];     // Path to the gas assembler
char go32path[100];    // Path to go32 extender
LONG undoasm[2];       // Place to space last asm command. LONG1 = addr
                       // LONG2 = opcode. If addr = 0 then no undo.

LONG Swap_End(LONG data)   // Swap Endian of a LONG word

{
     BYTE *temp;

     temp = (BYTE *) &data;

     return (temp[0] << 24) | (temp[1] << 16) |
            (temp[2] << 8)  | temp[3];
}

static inline void BusyChk(void)            // Waits for flag to become 0

{
    while(inportb(FlagPort) & 0x1);
}

static inline LONG SendRecev(LONG OutData)  // Sends and receives data

{
    outportb(DataPort,OutData);             // output data
    if(inportb(FlagPort) & 0x01)            // wait for flag to be reset
      BusyChk();

    bytestrans++;                           // Increment bytes transfered
    return inportb(DataPort);               // return value from saturn
}

static inline LONG SendWord(LONG OutData)   // Sends a word to saturn

{
    LONG result;

    result = SendRecev(OutData >> 8) << 8;
    result += SendRecev(OutData & 0xFF);

    return result;
}

static inline void SendLong(LONG OutData)   // Sends a LONG to saturn

{
    SendRecev(OutData >> 24);
    SendRecev((OutData >> 16) & 0xFF);
    SendRecev((OutData >> 8) & 0xFF);
    SendRecev(OutData & 0xFF);
}

static inline LONG GetLong(void)            // Gets a long from the saturn

{
    LONG result;

    result = ((SendRecev(0) & 0xFF) << 24);
    result |= ((SendRecev(0) & 0xFF) << 16);
    result |= ((SendRecev(0) & 0xFF) << 8);
    result |= (SendRecev(0) & 0xFF);

    return result;
}

void Connect(void)                      // Sync with PARs code

{

    LONG res;

    for (;;)
    {
        res = SendRecev('I');           // Send I and wait for D
        if (res != 'D')
           continue;
        res = SendRecev('N');           // Send N and wait for O
        if (res == 'O')
           break;
    }

}

void Connect2(void)                  // Sync with bootloader code
                                     // different format to ensure we r not
{                                    // still in par code.

    LONG res;

    for (;;)
    {
        res = SendRecev('S');        // Send S and wait for H
        if (res != 'H')
           continue;
        res = SendRecev('2');        // Send 2 and wait for M
        if (res == 'M')
           break;
    }

}

WORD getch_ext(void)

// get extended character format. if extended character append a 0xFF in hi
// byte. Heavily simplifies coding.

{
     WORD ch;

     ch = getch();
     if(ch == 0)
       ch = 0xFF00 | getch();

     return ch;
}

void newgets(char *str)

// Get string procedure due to it being broken in djgpp

{
   char *tempstr;
   WORD ch;

   fflush(stdout);
   tempstr = str;
   ch = getch_ext();
   while(ch != 13)
   {
      if((ch >= 32) && (ch < 256))
      {
         putch(ch & 0xFF);
         *tempstr = ch & 0xff;
         tempstr++;
      }
      if(ch == 8)
      {
         if(tempstr > str)
         {
            putch(8);
            putch(' ');
            putch(8);
            tempstr--;
         }
      }
      ch = getch_ext();
   }
   *tempstr = 0;
   printf("\n");
   fflush(stdout);
}

void DispRegs(const LONG *regblock)    // Displays regs on screen

{
     int loop;
     LONG regtemp[23];

     for(loop = 0;loop < 23;loop++)
        regtemp[loop] = Swap_End(regblock[loop]);

     printf("(00) R0  =%08lX (01) R1  =%08lX (02) R2  =%08lX (03) R3  =%08lX\n"
           ,regtemp[20],regtemp[19],regtemp[18],regtemp[17]);
     printf("(04) R4  =%08lX (05) R5  =%08lX (06) R6  =%08lX (07) R7  =%08lX\n"
           ,regtemp[16],regtemp[15],regtemp[14],regtemp[13]);
     printf("(08) R8  =%08lX (09) R9  =%08lX (10) R10 =%08lX (11) R11 =%08lX\n"
           ,regtemp[12],regtemp[11],regtemp[10],regtemp[9]);
     printf("(12) R12 =%08lX (13) R13 =%08lX (14) R14 =%08lX (XX) R15 =%08lX\n"
           ,regtemp[8],regtemp[7],regtemp[6],regtemp[5]);
     printf("(XX) VBR =%08lX (17) GBR =%08lX (18) MACL=%08lX (19) MACH=%08lX\n"
           ,regtemp[3],regtemp[4],regtemp[2],regtemp[1]);
     printf("(20) PC  =%08lX (21) PR  =%08lX (22) SR  =%08lX\n"
           ,regtemp[21],regtemp[0],regtemp[22]);
     printf("--MQIIII--ST\n");
     for(loop = 0;loop < 12;loop++)
        putch(((regtemp[22] >> (11 - loop)) & 0x1) + '0');
     printf("\n");
}

void GetRegs(LONG *regs)

// Get register block from saturn

{
     int loop;
     BYTE *regp;

     SendRecev(0x4);                   //Send Get registers Command
     regp = (BYTE *)regs;              //Get reg values
     for(loop = 0;loop < 0x5C;loop++)  //Get regs from PAR
        *regp++ = SendRecev(0);
     *regp++ = 0;                      //Set the dirty flag to 0
     *regp++ = 0;
     *regp++ = 0;
     *regp++ = 0;

}

void SetRegs(LONG *regs)

// Set register block in saturn

{
     int loop;
     BYTE *regp;

     SendRecev(0x6);                   // Send set regs command
     regp = (BYTE *)regs;              // Init pointer
     for(loop = 0;loop < 0x5C;loop++)  // Send regs to Saturn
        SendRecev(*regp++);
}

void modify_reg(LONG *regs)

// procedure to allow user to modify a reg value

{
     LONG reg_no;
     LONG val;

     printf("Enter register number ->");
     scanf("%d",&reg_no);
     if((reg_no < 0) && (reg_no > 22))
     {
       printf("Invalid Register number\n");
       getch();
       return;
     }

     printf("And value ->");
     scanf("%lx",&val);
     if((reg_no >= 0) && (reg_no < 15))
       {
          regs[20 - reg_no] = Swap_End(val);
       }
     else
     {
       switch(reg_no)
       {
          case 17: regs[4] = Swap_End(val);   // GBR
                   break;
          case 18: regs[2] = Swap_End(val);   // MACL
                   break;
          case 19: regs[1] = Swap_End(val);   // MACH
                   break;
          case 20: regs[21] = Swap_End(val&0xFFFFFFFE);  // PC
                   break;
          case 21: regs[0] = Swap_End(val&0xFFFFFFFE);   // PR
                   break;
          case 22: regs[22] = Swap_End(val&0x3E3);  // SR
                   break;
          default: return;
       }
     }

     regs[23] = 1;   // Set the modify flag. Saves rewriting regs everytime

}

void GetPRegs(LONG *Pregs)

// issues command 7 to get just PC and PR reg values. Speedup hack

{
     SendRecev(0x7);
     Pregs[0] = GetLong();
     Pregs[1] = GetLong();
}

void GetOps(WORD *ops,LONG PC)

// Gets memory block of current ops using command 3

{
    int loop;

    SendRecev(0x3);                   //Send Get memory command
    SendLong(PC-10);     	      //Send PC as read address
    SendLong(20);                     //Send length 20 bytes (10 opcodes)

    for(loop = 0;loop < 10;loop++)
    {
        ops[loop] = SendRecev(0) << 8;
        ops[loop] |= SendRecev(0);
    }
}

void GetMemory(LONG saddr,LONG length,BYTE *memory)

// Get a memory block

{
    int loop;

    SendRecev(0x3);                   //Send Get memory command
    SendLong(saddr);                  //Send PC as read address
    SendLong(length);                 //Send length 20 bytes (10 opcodes)

    for(loop = 0;loop < length;loop++)
       memory[loop] = SendRecev(0);
}

int SetMemByte(LONG addr,BYTE data)

// Uses command 5 to set a memory byte
// Returns 1 if successful, 0 if not

{
     int loop,valid;

     loop = 0;
     valid = 0;

     while(memareas[loop].saddr != 0xFFFFFFFF)
     {
        if((addr >= memareas[loop].saddr) &&
          (addr < memareas[loop].faddr))
          if(memareas[loop].write)
            valid = 1;
          else
             return 0;

        loop++;
     }

     if(!valid)
       return 0;

     SendRecev(0x5);
     SendLong(addr);
     SendLong(1);

     SendRecev(data & 0xFF);

     return 1;
}

void disp_memblock(LONG saddr,const BYTE *mem)

// Helper to display memory block. Displays 256 bytes from *mem

{
     int loopx,loopy;
     LONG address = saddr;

     printf("  Addr  | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 0123456789ABCDEF\n");
     printf("--------------------------------------------------------- ----------------\n");

     for(loopy = 0;loopy < 16;loopy++)
     {
         printf("%08lX:",address);
         for(loopx = 0;loopx < 16;loopx++)
            printf(" %02X",mem[(loopy * 16) + loopx]);
         printf(" ");
         for(loopx = 0;loopx < 16;loopx++)
            if(mem[(loopy * 16) + loopx] >= 32)
              printf("%c",mem[(loopy * 16) + loopx]);
            else
              printf("%c",'-');
        address += 16;
        printf("\n");
     }
}

void SetMemory(LONG addr,LONG length,BYTE *memory)

// Uses command 5 to set a memory block

{
     int loop,valid;

     loop = 0;
     valid = 0;

     while(memareas[loop].saddr != 0xFFFFFFFF)
     {
        if((addr >= memareas[loop].saddr) &&
          ((addr + length) < memareas[loop].faddr))
          if(memareas[loop].write)
            valid = 1;
          else
          {
             printf("Memory block write protected\n");
             getch();
             return;
          }

        loop++;
     }

     if(!valid)
     {
        printf("Memory address not in valid ranges\n");
        getch();
        return;
     }

     SendRecev(0x5);
     SendLong(addr);
     SendLong(length);

     for(loop = 0;loop < length;loop++)
        SendRecev(memory[loop]);
}


void edit_mem(LONG saddr,BYTE *mem)

// Procedure for dynamic memory editing.

{
     int  loc;
     LONG scrx,scry;
     LONG ascii;
     LONG ch;
     LONG temp;
     BYTE undomem[256];
     LONG modified;

     for(loc = 0;loc < 256;loc++)
        undomem[loc] = mem[loc];

     clrscr();
     printf("Memory Edit Mode\n\n");

     disp_memblock(saddr,mem);
     printf("Press ESC to end,Arrow keys to move cusor,TAB to switch edit mode .\n");

     loc = 0;
     scrx = 11;
     scry = 5;
     ascii = 0;
     modified = 0;
     gotoxy(scrx,scry);
     textcolor(YELLOW);
     textbackground(BLUE);
     cprintf("%02X",mem[loc]);
     gotoxy(scrx,scry);

     while((ch = getch_ext()) != 27)
     {
        if(ascii == 0)
        {
          textcolor(LIGHTGRAY);
          textbackground(BLACK);
          gotoxy(11 + ((loc&0xFE) & 0x1F) + (((loc&0xFE) & 0x1F) >> 1),scry);
          cprintf("%02X",mem[loc/2]); // Clear the selection colors

          if(ch == 0xFF50)
          {
            if((loc + 32) < 512)
              loc+=32;
          }
          if(ch == 0xFF4B)
          {
            if((loc - 1) >= 0)
              loc -= 1;
          }
          if(ch == 0xFF4D)
          {
            if((loc + 1) < 512)
              loc += 1;
          }
          if(ch == 0xFF48)
          {
            if((loc - 32) >= 0)
              loc -= 32;
          }
          if(((ch & 0xFF00) == 0) && (ch != 9))
          {
            if((ch >= '0') && (ch <= '9'))
              {
                 temp = mem[loc/2];
                 if(loc & 1)
                 {
                   temp &= 0xF0;
                   temp |= ((ch - '0') & 0xF);
                 }
                 else
                 {
                   temp &= 0xF;
                   temp |= (((ch - '0') & 0xF) << 4);
                 }
                 if(SetMemByte(saddr + (loc/2),temp))
                   {
                      mem[loc/2] = temp;
                      clrscr();
                      printf("Memory Edit Mode\n\n");

                      disp_memblock(saddr,mem);
                      printf("Press ESC to end,Arrow keys to move cusor,TAB to switch edit mode .\n");
                      if((loc + 1) < 512)
                        loc++;
                      modified = 1;
                   }
              }  // set nibble to value and write to mem
            else
              if((toupper(ch) >= 'A') && (toupper(ch) <= 'F'))
                {
                   temp = mem[loc/2];
                   if(loc & 0x1)
                   {
                     temp &= 0xF0;
                     temp |= (((toupper(ch) - 'A') & 0xF)+10);
                   }
                   else
                   {
                     temp &= 0xF;
                     temp |= ((((toupper(ch) - 'A') & 0xF) + 10) << 4);
                   }

                   if(SetMemByte(saddr + (loc/2),temp))
                   {
                      mem[loc/2] = temp;
                      clrscr();
                      printf("Memory Edit Mode\n\n");

                      disp_memblock(saddr,mem);
                      printf("Press ESC to end,Arrow keys to move cusor,TAB to switch edit mode .\n");
                      if((loc + 1) < 512)
                        loc++;
                      modified = 1;
                   }
                   // write to mem
                   // update both sides of mem dump
                }
          }
          if(ch == 9)
          {
             loc /= 2;
             scrx = 59 + (loc & 0xF);
             scry = 5 + (loc >> 4);
             gotoxy(scrx,scry);
             textcolor(YELLOW);
             textbackground(BLUE);
             if(mem[loc] >= 32)
               putch(mem[loc]);
             else
               putch('-');
             gotoxy(scrx,scry);
             ascii = 1;
          }
          else
          {
             scrx = 11 + (loc & 0x1F) + ((loc & 0x1F) >> 1);
             scry = 5 + (loc >> 5);
             textcolor(YELLOW);
             textbackground(BLUE);
             gotoxy(11 + ((loc&0xFE) & 0x1F) + (((loc&0xFE) & 0x1F) >> 1),scry);
             cprintf("%02X",mem[loc/2]); // Set the selection colors

             gotoxy(scrx,scry);
          }
        }
        else
        {
          gotoxy(scrx,scry);
          textcolor(LIGHTGRAY);
          textbackground(BLACK);
          if(mem[loc] >= 32)
            putch(mem[loc]);
          else
            putch('-');

          if(ch == 0xFF50)
          {
            if((loc + 16) < 256)
              loc+=16;
          }
          if(ch == 0xFF4B)
          {
            if((loc - 1) >= 0)
              loc -= 1;
          }
          if(ch == 0xFF4D)
          {
            if((loc + 1) < 256)
              loc += 1;
          }
          if(ch == 0xFF48)
          {
            if((loc - 16) >= 0)
              loc -= 16;
          }
          if((ch >= 32) && (ch < 256))
          {
             if(SetMemByte(saddr + loc,ch & 0xFF))
             {
                mem[loc] = ch & 0xFF;
                clrscr();
                printf("Memory Edit Mode\n\n");

                disp_memblock(saddr,mem);
                printf("Press ESC to end,Arrow keys to move cusor,TAB to switch edit mode .\n");
                if((loc + 1) < 256)
                  loc++;
                modified = 1;
             }

          }
          if(ch == 9)
          {
            loc *= 2;
            scrx = 11 + (loc & 0x1F) + ((loc & 0x1F) >> 1);
            scry = 5 + (loc >> 5);
            gotoxy(scrx,scry);
            textcolor(YELLOW);
            textbackground(BLUE);
            cprintf("%02X",mem[loc/2]);
            gotoxy(scrx,scry);
            ascii = 0;
          }
          else
          {
            scrx = 59 + (loc & 0xF);
            scry = 5 + (loc >> 4);
            gotoxy(scrx,scry);
            textcolor(YELLOW);
            textbackground(BLUE);
            if(mem[loc] >= 32)
              putch(mem[loc]);
            else
              putch('-');
            gotoxy(scrx,scry);
          }
        }
     }

     textcolor(LIGHTGRAY);
     textbackground(BLACK);
     clrscr();
     if(modified)
     {
        printf("Keep Changes? Press Y to keep changes or anyother key to not.\n");
        ch = getch_ext();
        if(toupper(ch&0xFF) != 'Y')
          SetMemory(saddr,256,undomem);
     }
}

void dump_mem(LONG saddr)

//display a memory dump

{
     int loopx,loopy;
     LONG address;
     LONG lastaddr;
     WORD ch;
     BYTE memory[256];

     address = saddr;
     lastaddr = saddr;
     clrscr();
     do
     {
       loopx = 0;
       loopy = 0;

       while(memareas[loopx].saddr != 0xFFFFFFFF)
       {
          if((address >= memareas[loopx].saddr) &&
            ((address + 256) < memareas[loopx].faddr))
          loopy = 1;
          loopx++;
       }

       if((loopy == 0) || (address >= 0x30000000))
       {
          printf("Invalid memory range\n");
          getch();
          address = lastaddr;
       }

       clrscr();
       GetMemory(address,256,memory);
       printf("Memory Dump Mode\n\n");
       disp_memblock(address,memory);
       printf("Press ESC to end,PGUP and PGDOWN to move dump or F1 to edit mem\n");

       for(;;)
       {
         ch = getch_ext();
         if(ch == 27)
           break;
         if(ch == 0xFF49)
         {
            lastaddr = address;
            address -= 256;
            break;
         }
         if(ch == 0xFF51)
         {
            lastaddr = address;
            address += 256;
            break;
         }
         if(ch == 0xFF3B)
         {
            edit_mem(address,memory);
            break;
         }

       }
     }
     while(ch != 27);
}

void get_memdump(void)

{
     LONG address;

     printf("Enter starting mem address ->");
     scanf("%lx",&address);

     dump_mem(address);
}

void write_byte(void)

// Allow a user to modify a memory byte

{
     int loopx,loopy;
     LONG addr;
     LONG val;

     printf("Enter byte memaddress ->");
     scanf("%lx",&addr);
     printf("And byte value ->");
     scanf("%x",&val);

     SetMemory(addr,1,(BYTE *)&val);
}

void write_word(void)

// allows user to modify a memory word

{
     LONG addr;
     LONG val;
     BYTE temp[4];

     printf("Enter word memaddress ->");
     scanf("%lx",&addr);
     printf("And word value ->");
     scanf("%lx",&val);

     temp[0] = (val >> 8) & 0xFF;
     temp[1] = val & 0xFF;

     SetMemory(addr,2,temp);
}

void write_long(void)

// allows user to modify a memory long

{
     LONG addr;
     LONG val;
     BYTE temp[4];

     printf("Enter long memaddress ->");
     scanf("%lx",&addr);
     printf("And long value ->");
     scanf("%lx",&val);

     temp[0] = (val >> 24) & 0xFF;
     temp[1] = (val >> 16) & 0xFF;
     temp[2] = (val >> 8) & 0xFF;
     temp[3] = val & 0xFF;

     SetMemory(addr,4,temp);
}

void mem_to_file(void)

// transfers a memory block to a file

{
     char filename[30];
     unsigned long saddr,eaddr,loop;
     FILE *fp;
     char *buffer;
     int  loopx,loopy;

     printf("Please enter the filename ->");
     newgets(filename);
     if((fp = fopen(filename,"wb")) == NULL)
       {
          printf("File does not exist");
          getch();
          return;
       }
     printf("Enter starting address ->");
     scanf("%lx",&saddr);
     printf("Enter ending address ->");
     scanf("%lx",&eaddr);

     loopx = 0;
     loopy = 0;

     while(memareas[loopx].saddr != 0xFFFFFFFF)
     {
        if((saddr >= memareas[loopx].saddr) &&
          ((eaddr) <= memareas[loopx].faddr))
        loopy = 1;
        loopx++;
     }

     if(!loopy)
     {
        printf("Memory out of range\n");
        getch();
        return;
     }

     buffer = (char *) malloc(eaddr - saddr);
     if(buffer == NULL)
     {
        printf("Cant allocate Buffer for memory\n");
        getch();
        return;
     }

     GetMemory(saddr,eaddr - saddr,buffer);
     fwrite(buffer,1,eaddr - saddr,fp);

     free(buffer);
     fclose(fp);
}

void file_to_mem(void)

{
     char filename[30];
     unsigned long saddr,eaddr,loop;
     FILE *fp;
     char *buffer;
     int  loopx,loopy;

     printf("Please enter the filename ->");
     fflush(stdout);
     fflush(stdin);
     newgets(filename);
     if((fp = fopen(filename,"rb")) == NULL)
       {
          printf("File does not exist");
          getch();
          return;
       }
     printf("Enter starting address ->");
     scanf("%lx",&saddr);

     eaddr = saddr + flength(fp);

     loopx = 0;
     loopy = 0;

     while(memareas[loopx].saddr != 0xFFFFFFFF)
     {
        if((saddr >= memareas[loopx].saddr) &&
          ((eaddr) <= memareas[loopx].faddr))
        loopy = 1;
        loopx++;
     }

     if(!loopy)
     {
        printf("Memory out of range\n");
        getch();
        return;
     }

     buffer = (char *) malloc(eaddr - saddr);
     if(buffer == NULL)
     {
        printf("Cant allocate Buffer for memory\n");
        getch();
        return;
     }

     fread(buffer,1,eaddr - saddr,fp);
     SetMemory(saddr,eaddr - saddr,buffer);

     free(buffer);
     fclose(fp);
}

void dump_fromregs(const LONG *regs)

{
     LONG reg_no;
     LONG val;

     printf("Enter register number ->");
     scanf("%d",&reg_no);
     if((reg_no < 0) && (reg_no > 22))
     {
       printf("Invalid Register number\n");
       getch();
       return;
     }

     if((reg_no >= 0) && (reg_no < 16))
       {
          dump_mem(Swap_End(regs[20 - reg_no]));
       }
     else
     {
       switch(reg_no)
       {
          case 16: dump_mem(Swap_End(regs[3]));   // VBR
                   break;
          case 17: dump_mem(Swap_End(regs[4]));   // GBR
                   break;
          case 20: dump_mem(Swap_End(regs[21]));  // PC
                   break;
          case 21: dump_mem(Swap_End(regs[0]));   // PR
                   break;
          default: return;
       }
     }
}

int scan_bp(LONG addr,const struct breakp *bps)

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

int find_freebp(struct breakp *bps)

// Find the first free breakpoint in array

{
    int loop = 0;

    for(loop = 0;loop < 10;loop++)
       if(bps[loop].addr == 0xFFFFFFFF)
         return loop;

    return 0xFF;
}

int find_bp(LONG addr,struct breakp *bps)

// Checks for a matching bp

{
    int loop = 0;

    for(loop = 0;loop < 10;loop++)
       if(bps[loop].addr == addr)
         return loop;

    return 0xFF;
}

void disp_breaks(struct breakp *bps)

// Displays breakpoints, allow modifications to be performed.

{
    int loop;
    int done;
    int ch;

    done = 0;

    while(!done)
    {
       clrscr();
       printf("Breakpoint listing\n\n");

       for(loop = 0;loop < 10;loop++)
       {
          if(bps[loop].addr != 0xFFFFFFFF)
             printf("bp %d - At address 0x%08lX\n",loop,bps[loop].addr);
          else
             printf("bp %d - Undefined\n",loop);
       }

       printf("\nPress the corresponding number to delete or modify a bp\n");
       printf("Press ESC to exit\n");
       ch = getch_ext();
       if((ch >= '0') && (ch <= '9'))
       {
          loop = ch - '0';
          printf("Bp %c - press D to delete, M to modify or any key to cancel\n",ch);
          ch = toupper(ch = getch_ext());
          if(ch == 'D')
          {
             bps[loop].addr = 0xFFFFFFFF;
             bps[loop].enable = 0;
          }
          else
            if(ch == 'M')
            {
               printf("Enter new address->");
               scanf("%lX",&bps[loop].addr);
               bps[loop].enable = 1;
            }
       }
       if(ch == 27)
         done = 1;
    }
}

void disp_debughelp(void)

// Display online help :)

{
     printf("Debugger commands :-\n\n");
     printf("F8     -  Execute program\n");
     printf("F9     -  Step into instruction\n");
     printf("F10    -  Step over instruction\n");
     printf("F1     -  Memory to screen dump\n");
     printf("F2     -  Write byte to memory\n");
     printf("F3     -  Write word to memory\n");
     printf("F4     -  Write long to memory\n");
     printf("F5     -  Modify a register\n");
     printf("F6     -  Dump memory block to a file\n");
     printf("F7     -  Read a file to memory\n");
     printf("UP,\n");
     printf("DOWN,\n");
     printf("PGUP,\n");
     printf("PGDOWN -  Move current disasm view\n");
     printf("HOME   -  Reset current disasm view\n");
     printf("ALT+D  -  Dump memory from register value\n");
     printf("SPACE  -  Set or unset a breakpoint at current location\n");
     printf("ALT+L  -  Display current breakpoint list\n");
     printf("ALT+I  -  Asm instruction at current point\n");
     printf("ALT+U  -  Undo previous inline asm\n");
     printf("ALT+R  -  Resets current program\n");
     printf("\nPress a key to continue\n");
     getch_ext();
}

void inline_asm(LONG addr)

// Do inline asm

{
    char str[20];
    FILE *fp;
    unsigned char temp[2];
    unsigned char undo[2];
    int opcode;

    printf("Enter opcode->");
    newgets(str);

    if((opcode = iasm(str)) == 0)
    {
       printf("Error in assembling\n");
       getch();
       return;
    }
    temp[0] = (opcode >> 8) & 0xFF;
    temp[1] = opcode & 0xFF;
    GetMemory(addr,2,undo);
    SetMemory(addr,2,temp);
    undoasm[0] = addr;
    undoasm[1] = (undo[0] << 8) | undo[1];
}

void undo_asm(void)

// Undo last inline asm

{
    BYTE temp[2];
    if(undoasm[0] != 0)
    {
      temp[0] = (undoasm[1] >> 8) & 0xFF;
      temp[1] = undoasm[1] & 0xFF;
      SetMemory(undoasm[0],2,temp);
      undoasm[0] = 0;
    }
}

int take_action(WORD ch,int *running,LONG *regs,LONG *daddr,struct breakp *bps)

// acts upon a user keypress. returns 1 if display needs rewriting

{
     int temp;

     switch(ch)
     {
        case 0x20:   if(scan_bp(*daddr,bps))   // Add breakpoint on cursor
                     {
                         temp = find_bp(*daddr,bps);
                         bps[temp].addr = 0xFFFFFFFF;
                         bps[temp].enable = 0;
                         return 1;
                     }
                     temp = find_freebp(bps);
                     if(temp != 0xFF)
                     {
                        bps[temp].addr = *daddr;
                        bps[temp].enable = 1;
                     }
                     else
                     {
                        printf("No free breakpoints available\n");
                        getch();
                     }
                     return 1;
        case 0xFF17: inline_asm(*daddr); // Inline asm at cursor
                     return 1;
        case 0xFF16: undo_asm(); // Undo asm
                     return 1;
        case 0xFF42: *running = 1; // Start execution
                     printf("Executing code. Press any key to return to disasm\n");
                     break;
        case 0xFF3B: get_memdump();
                     return 1;
        case 0xFF3C: write_byte();
                     return 1;
        case 0xFF3D: write_word();
                     return 1;
        case 0xFF3E: write_long();
                     return 1;
        case 0xFF40: mem_to_file();
                     return 1;
        case 0xFF3F: modify_reg(regs);
                     return 1;
        case 0xFF41: file_to_mem();
                     return 1;
        case 0xFF48: if((*daddr - 12) >= 0x6000000)
                       *daddr-=2;
                     return 1;
        case 0xFF50: if((*daddr + 2 + 10) < 0x6100000)
                       *daddr+=2;
                     return 1;
        case 0xFF49: if((*daddr - 30) >= 0x6000000)
                       *daddr -= 20;
                     return 1;
        case 0xFF51: if((*daddr + 30) < 0x6100000)
                       *daddr += 20;
                     return 1;
        case 0xFF47: *daddr = Swap_End(regs[21]);
                     return 1;
        case 0xFF20: dump_fromregs(regs);
                     return 1;
        case 0xFF26: disp_breaks(bps);
                     return 1;
        /*case 0xFF1F: regs[21] = Swap_End(Swap_End(regs[21]) + 2);
                     regs[23] = 1;
                     return 1;*/
        case '?'   : disp_debughelp();
                     return 1;
     }

     return 0;
}

int run_debug(void)

// main debugger loop

{
    LONG regs[24];
    LONG Pregs[2];
    BYTE *regp;
    int  loop;
    WORD ops[10];
    int  running = 0;
    WORD ch;
    LONG oldPR,breakPR,oldPC;
    LONG disasm_addr;
    struct breakp bpoints[10];
    int bp_met = 0;

    oldPR = 0;
    breakPR = 0;
    for(loop = 0;loop < 10;loop++)
    {
        bpoints[loop].addr = 0xFFFFFFFF;
        bpoints[loop].enable = 0;
    }
    undoasm[0] = 0;

    for(;;)
    {
        if(!bp_met)
          Connect2();        // Connect to UBC interrupt
        else
          bp_met = 0;

        GetPRegs(Pregs);

        if((oldPR != Pregs[0]) && (oldPR != 0) && ((oldPC+4) == Pregs[0]))
          {
             printf("Executing call back. Press any key to exit\n");
             oldPR = 0;
             oldPC = 0;
             breakPR = Pregs[0];
             running = 1;
          }

        if((breakPR == Pregs[1]) && (breakPR != 0))
            {
                running = 0;
                breakPR = 0;
                oldPR = 0;
                oldPC = 0;
            }

        if(!running)       // if stepping
        {
           GetRegs(regs);			// Read reg block
           disasm_addr = Swap_End(regs[21]);    // Get current PC address
           clrscr();				// Clr screen

           printf("Sega Saturn DeBuGgEr ver 0.%x. (c) TyRaNiD 2000\n\n",version);

           GetOps(ops,disasm_addr); // Read 20 bytes from mem

           disasm(disasm_addr-10,Swap_End(regs[21]),bpoints,ops); // Display disasm
           printf("\n");
           DispRegs(regs); // Display regs
        }

        if(!running)
        {
           ch = 0;
           while((ch != 0xFF43) && (!running))
           {
               ch = getch_ext();
               if(ch == 27)
               {
                  SendRecev(2);
                  return 1;
               }
               if(ch == 0xFF13)
               {
                  /*regs[21] = Swap_End(0x02000F00);// Set PC to 0x02000F00
                  SetRegs(regs);                  // Set registers
                  */
                  SendRecev(9);                   // Execute command 9.
                  return 0;
               }
               if(ch == 0xFF44)
               {
                 oldPR = Swap_End(regs[0]);
                 oldPC = Swap_End(regs[21]);
                 ch = 0xFF43;          // dirty hack :)
               }
               else
                 oldPR = 0;


               if(take_action(ch,&running,regs,&disasm_addr,bpoints))
               {
                  clrscr();

                  printf("Sega Saturn DeBuGgEr ver 0.%x. (c) TyRaNiD 2000\n\n",version);

                  GetOps(ops,disasm_addr);

                  disasm(disasm_addr-10,Swap_End(regs[21]),bpoints,ops);
                  printf("\n");
                  DispRegs(regs);
               }
           }
           if(regs[23])
             SetRegs(regs);
           SendRecev(1);
        }
        else
        {
            if(kbhit())
            {
                ch = getch_ext();
                running = 0;
                breakPR = 0;
                oldPR = 0;
            }

            if(scan_bp(Pregs[1],bpoints))
            {
                running = 0;
                breakPR = 0;
                oldPR = 0;
                bp_met = 1;
            }
            else
            SendRecev(1);
        }
        opsran++;
    }
   return 1;
}

void disp_help(void)

{
    printf("Sega Saturn(TM) DeBuGgEr ver 0.%x.\n",version);
    printf("Coded by TyRaNiD (c) 2000\n\n");

    printf("USAGE    : satdis.exe filename [options]\n\n");
    printf("filename : Filename of program to be loaded.\n");
    printf("[options]\n");
    printf("-pX      : Number of distinct commlink port. X in 0x3X0\n");
    printf("-lX      : X is the load address of program in hex.Not needed in coff format.\n");
    printf("-c       : Force program to coff format.\n");
    printf("-d       : Turn on Datel PAR support. Defaults as 4in1+\n");
    printf("-h       : Display this help\n");
    printf("\nFor more infomation on usage plz read the readme.txt file\n");
}

void update_pro(int percent,int lastper)

{
    if((percent / 10) != (lastper / 10))
      putch('.');
}


void load_bin(void)

{
    int loop;
    FILE *fp;
    int lastper = 100;
    LONG filesize;

    fp = fopen(loadfile,"rb");
    filesize = flength(fp);

    Connect2();                              //Sync with custom handler
    SendRecev(9);                            //Send com code (bullshit)
    SendLong(loadaddr-2);                    //Send program start
    SendLong(filesize+2);                 //Send p length
    SendLong(loadaddr-2);                    //Send initial PC
    SendRecev(0);                            //Send a nop to start
    SendRecev(9);
    for(loop = 0;loop < filesize;loop++)  //Send program
    {
       SendRecev(getc(fp) & 0xFF);
       update_pro((loop * 100) / filesize,lastper);
       lastper = (loop * 100) / filesize;
    }
}

LONG cofflread(const BYTE *buf)

{
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

WORD coffwread(const BYTE *buf)

{
    return (buf[0] << 8) | buf[1];
}

int load_coff(void)

{
    FILE *fp;
    LONG coffsize;
    LONG loadaddr;
    LONG size;
    LONG addr;
    LONG segs;
    BYTE *buffer;
    int loop1,loop2;
    int total,lastper;

    total = 0;
    lastper = 100;
    fp = fopen(loadfile,"rb");

    buffer = (unsigned char *)malloc(flength(fp));	/* Allocate buffer for file */
    fread(buffer,1,flength(fp),fp);			/* Read in File */
    fclose(fp);

    segs = coffwread(buffer + 2);			/* Get number of program segements */
    loadaddr = cofflread(buffer + 0x24);		/* Get program load address */

    coffsize = 0;	

    for(loop1 = 0;loop1 < segs;loop1++)
       coffsize += cofflread((buffer + 0x30) + (loop1 * 0x28) + 16);

    /* Get size of coff program */

    Connect2();                              /* Sync with custom handler */
    SendRecev(9);                            /* Send com code */
    SendLong(loadaddr-2);                    /* Send program start */
    SendLong(coffsize+2);                    /* Send code length */
    SendLong(loadaddr-2);                    /* Send initial PC */
    SendRecev(0);                            /* Send a nop to start */
    SendRecev(9);

    for(loop1 = 0;loop1 < segs;loop1++)
    {
       addr = cofflread((buffer + 0x30) + (loop1 * 0x28) + 20);
       size = cofflread((buffer + 0x30) + (loop1 * 0x28) + 16);
       if(size != 0)
       {
          if(addr != 0)
            for(loop2 = 0;loop2 < size;loop2++)
            {
               SendRecev(buffer[loop2 + addr]);
               total++;
               update_pro((total * 100) / coffsize,lastper);
               lastper = (total * 100) / coffsize;
            }
          else
            for(loop2 = 0;loop2 < size;loop2++)
            {
               SendRecev(0);
               total++;
               update_pro((total * 100) / coffsize,lastper);
               lastper = (total * 100) / coffsize;
            }
       }
    }

    free(buffer);
}

int check_coff(const char *filename)

/* Function to check the validy of a coff file before the bootloader is started
   Checks the first word for valid sh2 compile and checks loadaddr */
{
    FILE *fp;
    LONG temp;

    fp = fopen(filename,"rb");
    temp = (getc(fp) << 8) | getc(fp);
    if(temp != 0x0500)
    {
       printf("ERROR : Coff file not compiled for Super H\n");
       return 0;
    }
    fseek(fp,0x24,SEEK_SET);
    temp = (getc(fp) << 24) | (getc(fp) << 16) | (getc(fp) << 8) | getc(fp);
    if((temp < 0x6002000) || (temp > 0x6040000))
    {
       printf("Load address %lX is out of range\n",loadaddr);
       disp_help();
       return 0;
    }
    fclose(fp);
    return 1;
}

int load_boot(void)

{
    FILE *fp;
    int loop;
    char *bootcode;

    if(datel)
      bootcode = bootdatel;
    else
      bootcode = bootems;

    if(!release) /* Release version has bootcode as header data (for testing new boot code) */
    {
      if((fp = fopen("bootload.bin","rb")) == NULL)
        return 0;
    }
    Connect();              /* Syncronise with Saturn */
    SendRecev(9);           /* Send command code 9 */
    SendLong(0x06040000);   /* Send program start address */
    if(!release)
    {
       SendLong(flength(fp));  /* Send code length */
       SendRecev(1);
       for(loop = 0;loop < flength(fp);loop++)  /* Send program to saturn */
          SendRecev(getc(fp) & 0xFF);
       fclose(fp);
    }
    else
    {
       SendLong(bootsize);
       SendRecev(1);           /* Send mode code */

       for(loop = 0;loop < bootsize;loop++)
          SendRecev(bootcode[loop]);
    }
    return 1;
}

int parse_ini(void)

{
    FILE *fp;
    char str[150];
    char tempstr[150];
    int loop,loop2;

    if((fp = fopen("satdebug.ini","r")) == NULL)
      return 0;


    while(fgets(str,150,fp) != NULL)
    {
        if(str[0] != '#')
        {
           loop = 0;
           while((str[loop] != '=') && (str[loop] != 0))
           {
               tempstr[loop] = str[loop];
               loop++;
           }

           tempstr[loop] = 0;

           if(!strcmp(tempstr,"port"))
           {
              DataPort = 0x300 + ((str[5] - '0') * 0x10);
              FlagPort = 0x300 + ((str[5] - '0') * 0x10) + 0x2;
           }

           if(!strcmp(tempstr,"datel"))
             datel = str[6]-'0';

           
        }
    }
}

int parse_args(int argc,char *argv[])

{
    FILE *fp;
    int loop,loop2;
    char tempstr[11];
    char *bp;
    char ext[5];

    if(argc < 2)
    {
       disp_help();
       return 0;
    }

    strcpy(loadfile,argv[1]);      // Copy filename into
    if((fp = fopen(loadfile,"rb")) == NULL)
    {
        printf("ERROR : Loadfile does not exist\n");
        disp_help();
        return 0;
    }

    fclose(fp);
    fnsplit(loadfile,NULL,NULL,NULL,ext);

    if(!strcmp(ext,".cof"))
    {
      iscoff = 1;
      if(!check_coff(loadfile))
        return 0;
    }

    for(loop = 2;loop < argc;loop++)
    {
       if(argv[loop][0] == '-')
         switch(toupper(argv[loop][1]))
         {
             case 'P': DataPort = 0x300 + ((argv[loop][2] - '0') * 0x10);
                       FlagPort = 0x300 + ((argv[loop][2] - '0') * 0x10) + 0x2;
                       break;
             case 'C': iscoff = 1;
                       if(!check_coff(loadfile))
                         return 0;
                       break;
             case 'L': for(loop2 = 0;argv[loop][2+loop2] != 0;loop2++)
                       {
                           tempstr[loop2] = argv[loop][2+loop2];
                       }
		       tempstr[loop2] = 0;
                       loadaddr = strtol(tempstr,&bp,16);
                       if((loadaddr < 0x6002000) || (loadaddr > 0x6040000))
                       {
                          printf("Load address %lX is out of range\n",loadaddr);
                          disp_help();
                          return 0;
                       }
                       break;
             case 'E': toentry = 1;
                       break;
             case 'H': disp_help();
                       return 0;
             case 'D': datel = 1;
                       break;
         }
    }

    if((DataPort == 0) || (FlagPort == 0))
    {
        printf("ERROR : No port number specified\n\n");
        disp_help();
        return 0;
    }

    if((loadaddr == 0) && (!iscoff))
    {
        printf("ERROR : No load address specified\n\n");
        disp_help();
        return 0;
    }
    if(toentry)
      toentry = iscoff;   // Sets toentry to be 0 if not using coff.


    return 1;
}

int main(int argc,char *argv[])

{
    int done;

    opsran = 0;
    iscoff = 0;
    loadaddr = 0;
    entryaddr = 0;
    toentry = 0;
    strcpy(loadfile,"");
    DataPort = 0;
    FlagPort = 0;
    datel = 0;
    done = 0;

    parse_ini();
    if(!parse_args(argc,argv))
      return 1;
    printf("Sega Saturn(TM) DeBuGgEr ver 0.%x.\n",version);

    printf("Coded by TyRaNiD (c) 2000\n\n");

    if(datel)
      printf("<-Using Datel PAR bootcode->\n");
    else
      printf("<-Using EMS 4in1+ bootcode->\n");

    printf("<-Loading boot code at port %lX->\n",DataPort);

    if(!load_boot())       // Load boot code.
      return 1;

    printf("\n<-Boot Code Loaded->\n");
    bytestrans = 0;

    while(!done)
    {
       printf("\n<-About to start user code>\n\n");

       printf("0%% [           ] 100%%\n");
       gotoxy(5,wherey() - 1);

       if(iscoff)
         load_coff();
       else
         load_bin();               // Load program

       done = run_debug();
       clrscr();
    }

    puts("Normal Program Termination\n");
    printf("Ops ran = %ld\n",opsran);
    printf("Bytes Transfered = %ld\n",bytestrans);

    return 0;

}

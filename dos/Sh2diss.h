// sh2diss.h Header file for SH-2 disassembler

struct breakp

{
    unsigned int addr;
    unsigned int enable;
};

void disasm(unsigned int,unsigned int,struct breakp *,const unsigned short *);


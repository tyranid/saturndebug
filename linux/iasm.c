#include <stdio.h>
#include <ctype.h>
#include "conio.h"
#include "sh-opc.h"

typedef struct
  {
    int type;
    int reg;
  }

sh_operand_info;  // Operand structure

void asm_bad(const char *str)

// Displays an error message

{
    cprintf("ERROR : ");
    cputs(str);
}

/* try and parse a reg name, returns number of chars consumed */
int parse_reg (const char *src, int *mode, int *reg)

// Ripped out of the gas asm :)

{
  /* We use !isalnum for the next character after the register name, to
     make sure that we won't accidentally recognize a symbol name such as
     'sram' as being a reference to the register 'sr'.  */

  if (src[0] == 'r')
    {
      if (src[1] == '1')
	{
	  if (src[2] >= '0' && src[2] <= '5' && ! isalnum (src[3]))
	    {
	      *mode = A_REG_N;
	      *reg = 10 + src[2] - '0';
	      return 3;
	    }
	}
      if (src[1] >= '0' && src[1] <= '9' && ! isalnum (src[2]))
	{
	  *mode = A_REG_N;
	  *reg = (src[1] - '0');
	  return 2;
	}
    }

  if (src[0] == 's' && src[1] == 'r' && ! isalnum (src[2]))
    {
      *mode = A_SR;
      return 2;
    }

  if (src[0] == 's' && src[1] == 'p' && ! isalnum (src[2]))
    {
      *mode = A_REG_N;
      *reg = 15;
      return 2;
    }

  if (src[0] == 'p' && src[1] == 'r' && ! isalnum (src[2]))
    {
      *mode = A_PR;
      return 2;
    }
  if (src[0] == 'p' && src[1] == 'c' && ! isalnum (src[2]))
    {
      *mode = A_DISP_PC;
      return 2;
    }
  if (src[0] == 'g' && src[1] == 'b' && src[2] == 'r' && ! isalnum (src[3]))
    {
      *mode = A_GBR;
      return 3;
    }
  if (src[0] == 'v' && src[1] == 'b' && src[2] == 'r' && ! isalnum (src[3]))
    {
      *mode = A_VBR;
      return 3;
    }

  if (src[0] == 'm' && src[1] == 'a' && src[2] == 'c' && ! isalnum (src[4]))
    {
      if (src[3] == 'l')
	{
	  *mode = A_MACL;
	  return 4;
	}
      if (src[3] == 'h')
	{
	  *mode = A_MACH;
	  return 4;
	}
    }

  return 0;
}

int strip_opname(const char *str,char *name)

// Strip out the opcode name and return in *name

{
   int pos;

   pos = 0;

   while((str[pos] != 32) && (str[pos] != 0))
   {
      name[pos] = str[pos];
      pos++;
   }

   name[pos] = 0;

   return pos;
}

int strip_arg(const char *str,char *arg)

// Strip out next arg in the string

{
    int pos;

    pos = 0;
    if(str[0] == '@')
    {
      if(str[1] == '(')
      {
        if(str[2] != 0)
        {
        do
        {
           arg[pos] = str[pos];
           pos++;
        }
        while((str[pos-1] != ')') && (str[pos] != 0));

        }
      }
      else
        while((str[pos] != ',') && (str[pos] != 0))
        {
           arg[pos] = str[pos];
           pos++;
        }
    }
    else
      while((str[pos] != ',') && (str[pos] != 0))
      {
          arg[pos] = str[pos];
          pos++;
      }

    arg[pos] = 0;

    return pos;
}

int parse_at(const char *arg,sh_operand_info *op)

// Parse pointer arguement and return a operand info struct

{
    int mode;
    int len;

    if(arg[0] == 0)
      return 0;

    if(*arg == '-')
    {
       arg++;
       len = parse_reg(arg,&mode,&(op->reg));
       if(len == 0)
       {
          asm_bad("Cant find arg");
          return 0;
       }
       if(mode != A_REG_N)
       {
          asm_bad("Invalid reg after @-");
          return 0;
       }
       else
       {
          op->type = A_DEC_N;
       }
    }
    else
     if(*arg == '(')
     {
        arg++;
        len = parse_reg(arg,&mode,&(op->reg));
        if((len > 0) && (mode == A_REG_N))
        {
           arg+=len;
           if(op->reg != 0)
           {
              asm_bad("Must be @(R0,...)");
              return 0;
           }
           if(arg[0] == ',')
              arg++;
           len = parse_reg(arg,&mode,&(op->reg));
           arg += len;
           if(mode == A_GBR)
           {
              op->type = A_R0_GBR;
           }
           else if (mode == A_REG_N)
           {
              op->type = A_IND_R0_REG_N;
           }
           else
           {
              asm_bad("Syntax error in @(R0,...)");
              return 0;
           }

        }
        else
        {
           while((*(arg-1) != ',') && (*arg != 0))
              arg++;
           len = parse_reg(arg,&mode,&(op->reg));
           arg+=len;
           if(len)
           {
             if(mode == A_REG_N)
             {
                op->type = A_DISP_REG_N;
             }
             else if (mode == A_GBR)
             {
                op->type = A_DISP_GBR;
             }
             else if (mode == A_DISP_PC)
             {
                op->type = A_DISP_PC;
             }
             else
             {
                asm_bad("Bad syntax in @(disp,[Rn,GBR,PC])");
                return 0;
             }
           }
           else
           {
              asm_bad("Bad syntax in @(disp,[Rn,GBR,PC])");
              return 0;
           }
        }
        if(*arg != ')')
        {
          asm_bad("Expected a )");
          return 0;
        }
     }
     else
     {
        arg += parse_reg(arg,&mode,&(op->reg));
        if(mode != A_REG_N)
        {
           asm_bad("Invalid register after @");
           return 0;
        }
        if(arg[0] == '+')
        {
           op->type = A_INC_N;
        }
        else
        {
           op->type = A_IND_N;
        }
     }

    return 1;
}

int parse_arg(const char *arg,sh_operand_info *op)

// Parse arg and return a filled operand struct

{
    int len,mode;
    int loop;
    char temp[100];

    if(arg[0] == 0)
    {
      op->type = 0;
      op->reg = 0;
      return 1;
    }

    if(*arg == '@')
    {
       arg++;
       return parse_at(arg,op);
    }

    if(*arg == '#')
    {
       op->type = A_IMM;
       return 1;
    }

    len = parse_reg(arg,&mode,&(op->reg));
    if(len)
    {
       op->type = mode;
       return 1;
    }
    else
    {
       op->type = A_BDISP12;
       return 1;
    }


    return 0;
}

int fix_arg(int type,sh_operand_info *arg)

// Checks the arg with the opcode type and see if its matchable
// Returns 1 if possible to match and 0 if not

{
   if((type == A_DEC_M) && (arg->type == A_DEC_N))
     return 1;
   if((type == A_DISP_REG_M) && (arg->type == A_DISP_REG_N))
     return 1;
   if((type == A_INC_M) && (arg->type == A_INC_N))
     return 1;
   if((type == A_IND_M) && (arg->type == A_IND_N))
     return 1;
   if((type == A_IND_R0_REG_M) && (arg->type == A_IND_R0_REG_N))
     return 1;
   if((type == A_REG_M) && (arg->type == A_REG_N))
     return 1;
   if((type == A_BDISP8) && (arg->type == A_BDISP12))
     return 1;
   if((type == A_R0) && (arg->type == A_REG_N) && (arg->reg == 0))
     return 1;

   return 0;
}

int search_op(const char *name,sh_operand_info *arg1,sh_operand_info *arg2,sh_opcode_info *op)

// Search for a matching opcode and fix args if necessary

{
   int loop = 0;
   sh_operand_info arg1back,arg2back;

   arg1back = *arg1;
   arg2back = *arg2;

   while(strcmp(sh_table[loop].name,"ftrv"))
   {
      if(!strcmp(sh_table[loop].name,name))
      {
         if(sh_table[loop].arg[0] != 0)
         {
            if(fix_arg(sh_table[loop].arg[0],arg1))
               arg1->type = sh_table[loop].arg[0];
         }
         if(sh_table[loop].arg[1] != 0)
         {
            if(fix_arg(sh_table[loop].arg[1],arg2))
               arg2->type = sh_table[loop].arg[1];
         }

         if((arg1->type == sh_table[loop].arg[0]) &&
            (arg2->type == sh_table[loop].arg[1]))
         {
            *op = sh_table[loop];
            return 1;
         }

      }

      *arg1 = arg1back;
      *arg2 = arg2back;
      loop++;
   }

   return 0;
}

void insert(unsigned int *opcode,int value,int pos)

// Insert a nibble into the supplied word

{
    *opcode |= ((value & 0xF) << (12 - (pos * 4)));
}

unsigned long build_bytes(sh_opcode_info op,sh_operand_info a1,
                          sh_operand_info a2,sh_operand_info disp)

// Now we know the opcode then build its bytes. Returns opcode if valid
// and 0 if not.

{
    int loop;
    int i;
    unsigned int opcode;

    loop = 0;
    opcode = 0;
    while(loop < 4)
    {
       i = op.nibbles[loop];
       if(i < 16)
       {
          insert(&opcode,i,loop);
          loop++;
       }
       else
       {
          switch(i)
          {
            case REG_M   : if(a1.type == REG_M)
                             insert(&opcode,a1.reg,loop);
                           else
                             insert(&opcode,a2.reg,loop);
                           break;
            case REG_N   : if(a1.type == REG_N)
                             insert(&opcode,a1.reg,loop);
                           else
                             insert(&opcode,a2.reg,loop);
                           break;
            case DISP_4  :
            case IMM_4   : insert(&opcode,disp.reg&0xF,loop);
                           break;
            case IMM_4BY2: disp.reg >>= 1;
                           insert(&opcode,disp.reg&0xF,loop);
                           break;
            case IMM_4BY4: disp.reg >>= 2;
                           insert(&opcode,disp.reg&0xF,loop);
                           break;
            case BRANCH_12: insert(&opcode,(disp.reg >> 8) & 0xF,loop);
                            insert(&opcode,(disp.reg >> 4) & 0xF,loop+1);
                            insert(&opcode,disp.reg & 0xF,loop+2);
                            loop += 2;
                            break;
            case DISP_8   :
            case IMM_8    :
            case BRANCH_8 : insert(&opcode,(disp.reg >> 4) & 0xF,loop);
                            insert(&opcode,disp.reg & 0xF,loop+1);
                            loop += 1;
                            break;
            case PCRELIMM_8BY2:
            case IMM_8BY2 :
                            disp.reg >>= 1;
                            insert(&opcode,(disp.reg >> 4) & 0xF,loop);
                            insert(&opcode,disp.reg & 0xF,loop+1);
                            loop += 1;
                            break;
            case PCRELIMM_8BY4:
            case IMM_8BY4 :
                            disp.reg >>= 2;
                            insert(&opcode,(disp.reg >> 4) & 0xF,loop);
                            insert(&opcode,disp.reg & 0xF,loop+1);
                            loop += 1;
                            break;
          }
          loop++;
       }
    }
    return opcode;
}

int rebuild_args(const char *arg1,const char *arg2,sh_operand_info *a1,
                  sh_operand_info *a2,sh_operand_info *disp)

// Rebuild args into the maximum 3 args for building.
// Redefine type values to nibble equivalents and extract imm values.
// returns 1 on error

{
   char s1[30],s2[30];
   int loop;
   char *bp;

   strcpy(s1,arg1);
   strcpy(s2,arg2);
   bp = NULL;

   switch(a1->type)
   {
     case A_IND_R0_REG_M:
     case A_DEC_M:
     case A_INC_M:
     case A_IND_M:
     case A_REG_M: a1->type = REG_M;
                   break;
     case A_IND_R0_REG_N:
     case A_DEC_N:
     case A_INC_N:
     case A_IND_N:
     case A_REG_N: a1->type = REG_N;
                   break;
     case A_DISP_PC: disp->reg = strtol(&s1[2],&bp,16);
                     disp->type = PCRELIMM;
                     break;
     case A_DISP_GBR:disp->reg = strtol(&s1[2],&bp,16);
                     disp->type = IMM;
                     break;
     case A_DISP_REG_M: disp->reg = strtol(&s1[2],&bp,16);
                        disp->type = IMM;
                        a1->type = REG_M;
                        break;
     case A_DISP_REG_N: disp->reg = strtol(&s1[2],&bp,16);
                        disp->type = IMM;
                        a1->type = REG_N;
                        break;
     case A_IMM       : disp->reg = strtol(&s1[1],&bp,16);
                        disp->type = IMM;
                        break;
     case A_BDISP12   :
     case A_BDISP8    : disp->reg = strtol(s1,&bp,16);
                        disp->type = IMM;
                        break;

   }

   switch(a2->type)
   {
     case A_IND_R0_REG_M:
     case A_DEC_M:
     case A_INC_M:
     case A_IND_M:
     case A_REG_M: a2->type = REG_M;

                   break;
     case A_IND_R0_REG_N:
     case A_DEC_N:
     case A_INC_N:
     case A_IND_N:
     case A_REG_N: a2->type = REG_N;

                   break;
     case A_DISP_PC: disp->reg = strtol(&s2[2],&bp,16);
                     disp->type = PCRELIMM;
                     break;
     case A_DISP_GBR:disp->reg = strtol(&s2[2],&bp,16);
                     disp->type = IMM;
                     break;
     case A_DISP_REG_M: disp->reg = strtol(&s2[2],&bp,16);
                        disp->type = IMM;
                        a2->type = REG_M;
                        break;
     case A_DISP_REG_N: disp->reg = strtol(&s2[2],&bp,16);
                        disp->type = IMM;
                        a2->type = REG_N;
                        break;
     case A_IMM       : disp->reg = strtol(&s2[1],&bp,16);
                        disp->type = IMM;
                        break;
     case A_BDISP12   :
     case A_BDISP8    : disp->reg = strtol(s2,&bp,16);
                        disp->type = IMM;
                        break;
   }

   return 0;
}

int iasm(char *str)

// Function to do all the work

{
   char name[30];
   char arg1[30];
   char arg2[30];
   char *p;
   int loop;
   int oplen,arg1len,arg2len;
   sh_operand_info arg1info,arg2info,disp;
   sh_opcode_info opcode;

   arg1info.type = 0;
   arg1info.reg = 0;
   arg2info.type = 0;
   arg2info.reg = 0;
   //gets(str);
   p = str;
   while(*p == ' ')
     p++;

   if((oplen = strip_opname(p,name)) == 0)
    {
       asm_bad("No opcode");
       return 0;
    }

   p += oplen;
   while(*p == ' ')
     p++;

   arg1len = strip_arg(p,arg1);
   p += arg1len;

   while(*p == ' ')
     p++;
   if(*p == ',')
     p++;
   while(*p == ' ')
     p++;

   arg2len = strip_arg(p,arg2);

   for(loop = 0;name[loop] != 0;loop++)
   {
       name[loop] = tolower(name[loop]);
   }
   for(loop = 0;arg1[loop] != 0;loop++)
      arg1[loop] = tolower(arg1[loop]);
   for(loop = 0;arg2[loop] != 0;loop++)
      arg2[loop] = tolower(arg2[loop]);

   if(!parse_arg(arg1,&arg1info))
   {
      if(arg1[0] != 0)
        asm_bad("Arg 1");
      return 0;
   }
   if(!parse_arg(arg2,&arg2info))
   {
      if(arg2[0] != 0)
        asm_bad("Arg 2");
      return 0;
   }

   if(!search_op(name,&arg1info,&arg2info,&opcode))
   {
     asm_bad("Invalid opcode. Likely doesnt exist or format is wrong");
     return 0;
   }

   loop = 0;
   rebuild_args(arg1,arg2,&arg1info,&arg2info,&disp);
   return build_bytes(opcode,arg1info,arg2info,disp);
}


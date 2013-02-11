!       Boot Loader and Comm Routine for Saturn Debugger
!       (c) TyRaNiD 02/2000...

	.section	.text
	.align	4
	.global	_entry
_entry:
	! Possibly do a grafix display to indicate init complete. Rip parboot

        mov.l @(_ConnectSR,pc),r1    ! Syncronise with PC
        jsr   @r1
        nop

        ! Probably dont need command number.. Probably dont matter
        ! Fuckit :) May remove to reduce code length..

        mov   #0x0,r0                ! Setup r0 to command no
        mov.l @(_SendReceSR,pc),r1
        jsr   @r1
        nop

        cmp/eq #0x09,r0
        bf     _Loop
        mov.l @(_Command09SR,pc),r1
        jsr   @r1
        nop

        _Loop:
        bra _Loop                    ! Infinite Loop. Should Never Touch
        nop                          ! Here just to prevent flowing into
                                     ! other code. not needed


_Connect:

! Syncronise with PC. S in H out, 2 in M out

        sts.l  pr,@-r15
        mov.l  r1,@-r15
        mov.l  r0,@-r15
        _ConnectLoop:
        mov    #0x48,r0              ! Send 'H'
        mov.l  @(_SendReceSR,pc),r1
        jsr    @r1                   ! Call Send And Recieve Procedure
        nop                          ! delay
        cmp/eq #0x53,r0              ! Compare return to 'S'
        bf     _ConnectLoop
        mov    #0x4d,r0              ! Send 'M'
        mov.l  @(_SendReceSR,pc),r1
        jsr    @r1                   ! Call Send and Recieve Procedure
        nop
        cmp/eq #0x32,r0              ! Compare to '2'
        bf     _ConnectLoop
        mov.l  @r15+,r0
        mov.l  @r15+,r1
        lds.l  @r15+,pr
        rts
        nop

_SendRece:

! Send and Recieve Data from the PAR+
! NOTE different register values for DATEL PAR/GameShark
! INPUT  R0 : Data to be send
! OUTPUT R0 : Data recieved

        mov.l  r1,@-r15
        mov.l  r2,@-r15
        mov.l  r0,@-r15

        mov.l  @(_FlagPort,pc),r1
        _WaitLoop:
        mov.b  @r1,r0
        and    #0x1,r0
        cmp/eq #0x1,r0
        bf     _WaitLoop                  ! Wait for Data flag to be 1

        mov.l  @(_ReadPort,pc),r1         ! Read input port
        mov.b  @r1,r2
        mov.l  @r15+,r0                   ! Restore input value
        mov.l  @(_WritePort,pc),r1        ! Write to output port
        mov.b  r0,@r1
        mov    r2,r0                      ! Set return value

        mov.l  @r15+,r2
        mov.l  @r15+,r1
        rts
        nop

.align 4

_ConnectSR:   .long _Connect
_SendReceSR:  .long _SendRece
_Command09SR: .long _Command09
_ReadPort:    .long 0x22600001
_WritePort:   .long 0x22400001
_FlagPort:    .long 0x22500001

_Command09:

! Command to copy a program to memory and execute
! Needs to setup correct UBC registers and int routine to run
! Format
! Recieve Long Word -> Program Address -> R2
! Recieve Long Word -> Program Length  -> R3
! Recieve Long Word -> Program Start address -> R4
! Recieve bytes (length) -> Program for starting

        sts.l  pr,@-r15

        mov.l  @(_GetLongSR,pc),r1
        jsr    @r1                   ! Get program load address
        nop
        mov    r0,r2                 ! Save in r2

        jsr    @r1                   ! Get program length
        nop
        mov    r0,r3                 ! Save in r3   

        jsr    @r1                   ! Get program start
        nop
        mov    r0,r4                 ! Save in R4

        mov.l  @(_SendRece2SR,pc),r1 ! Get address of SendRece
        mov    #0x0,r0               ! Clear r0   

        _CopyLoop:
        jsr    @r1                   ! Get byte
        nop
        mov.b  r0,@r2                ! Move byte to load address
        add    #0x01,r2              ! Increment r2   
        dt     r3                    ! Decrement r3 length counter
        bf     _CopyLoop             ! If not 0 then reloop
        nop

        ! Setup UBC here for the start address.
        ! Initialise UBC interrupt Vector and registers
        mov.l  @(_BARA,pc),r5        ! Setup r5 with Break Adress A
        mov.l  r4,@r5                ! Transfer Program start to Break Addr
        xor    r0,r0                 ! Clear r0
        mov.l  @(_BAMRA,pc),r5       ! Get Mask Address
        mov.l  r0,@r5                ! Clear mask register
        mov.l  @(_BBRA,pc),r5        ! Get Break condition reg
        mov.l  @(_BBRAWord,pc),r0    ! Get Break condition word
        mov.w  r0,@r5                ! Set break conds if/cpu/r/w
        mov.l  @(_BRCR,pc),r5        ! Get Break cycle reg
        mov.l  @(_BRCRWord,pc),r0    ! Get Break Cycle Word
        mov.w  r0,@r5                ! Set break cycle afterins/7604
        mov.l  @r5,r0                ! Delay to allow setup to become valid

        stc    vbr,r2
        add    #0x30,r2
        mov.l  @(_UBCIntEntrySR,pc),r1  ! Write UBC entry address to int 12
        mov.l  r1,@r2
        xor    r0,r0
        ldc    r0,sr

        lds    r4,pr
        mov.l  @(_StackP,pc),r0
        mov    r0,r15
        mov    #0x0,r0
        mov    #0x0,r1
        mov    #0x0,r2
        mov    #0x0,r3
        mov    #0x0,r4
        mov    #0x0,r5
        mov    #0x0,r6
        mov    #0x0,r7
        mov    #0x0,r8
        mov    #0x0,r9
        mov    #0x0,r10
        mov    #0x0,r11
        mov    #0x0,r12
        mov    #0x0,r13
        mov    #0x0,r14
                                     ! Need to clear registers
        rts                          ! Do a rts to start of code.
        nop                          

        lds.l  @r15+,pr              ! Should never touch
        rts
        nop

_GetLong:

! Get LONG word from registers
! NO INPUT
! OUTPUT : r0 -> 32 bit word

        sts.l  pr,@-r15
        mov.l  r11,@-r15
        mov.l  r10,@-r15
        mov.l  r1,@-r15
        mov    #0x0,r0
        mov.l  @(_SendRece2SR,pc),r1   ! Get MSB of word
        jsr    @r1
        nop
        extu.b r0,r11                 ! Remove any sign extension
        shll16 r11
        shll8  r11                    ! SHL into MSB position
        jsr    @r1
        nop
        extu.b r0,r0                  ! Remove sign extension
        shll16 r0                     ! SHL by 16
        or     r0,r11                 ! Mask onto first byte
        jsr    @r1
        nop
        extu.b r0,r0                  ! Remove sign extension
        shll8  r0                     ! SHL by 8
        or     r0,r11                 ! Mask onto first two bytes
        jsr    @r1
        nop
        extu.b r0,r0                  ! Remove sign extension
        or     r0,r11                 ! Mask onto LSB
        mov    r11,r0                 ! Move return value to r0
        mov.l  @r15+,r1
        mov.l  @r15+,r10
        mov.l  @r15+,r11
        lds.l  @r15+,pr
        rts
        nop

_SendLong:

! Send LONG word from regsiters
! INPUT  : r0 -> 32 bit word
! OUTPUT : NONE

        sts.l  pr,@-r15
        mov.l  r10,@-r15
        mov.l  r1,@-r15

        mov    r0,r10                  ! Store LONG in r10
        shlr16 r0
        shlr8  r0                      ! Shift down MSB

        mov.l  @(_SendRece2SR,pc),r1   ! Send MSB of LONG
        jsr    @r1
        nop

        mov    r10,r0                  ! Restore LONG
        shlr16 r0                      ! Shift down bits 16->24
        jsr    @r1                     ! Send
        nop

        mov    r10,r0                  ! Restore LONG
        shlr8  r0                      ! Shift down bits 8->16
        jsr    @r1                     ! Send
        nop
        mov    r10,r0
        jsr    @r1
        nop

        mov.l  @r15+,r1
        mov.l  @r15+,r10
        lds.l  @r15+,pr
        rts
        nop

_GetMemory:

! Procedure to get memory from saturn to PC
! Format. No input or output.
! Get memory start address -> R2
! Get length of block -> R3
! Read memory in bytes and send to PC.

        sts.l  pr,@-r15
        mov.l  r0,@-r15
        mov.l  r1,@-r15
        mov.l  r2,@-r15
        mov.l  r3,@-r15

        mov.l  @(_GetLongSR,pc),r1
        xor    r0,r0
        jsr    @r1                    ! Get memory start address
        nop

        mov    r0,r2                  ! Save in r2

        xor    r0,r0
        jsr    @r1                    ! Get memory length
        nop

        mov    r0,r3                  ! Save in r3

        mov.l  @(_SendRece2SR,pc),r1

        _GetMemoryLoop:

        xor    r0,r0            ! Clear r0
        mov.b  @r2,r0           ! Get byte

        jsr    @r1              ! call send receive
        nop

        add    #0x1,r2          ! increment memory pointer
        dt     r3               ! decrement length
        bf     _GetMemoryLoop   ! Jump if length != 0
        nop
        
        mov.l  @r15+,r3
        mov.l  @r15+,r2
        mov.l  @r15+,r1
        mov.l  @r15+,r0
        lds.l  @r15+,pr

        rts
        nop

_SetMemory:

! Procedure to set memory from saturn to PC
! Format. No input or output.
! Get memory start address -> R2
! Get length of block -> R3
! Read memory in bytes from PC and send to memory.

        sts.l  pr,@-r15
        mov.l  r0,@-r15
        mov.l  r1,@-r15
        mov.l  r2,@-r15
        mov.l  r3,@-r15

        mov.l  @(_GetLongSR,pc),r1
        xor    r0,r0
        jsr    @r1                    ! Get memory start address
        nop

        mov    r0,r2                  ! Save in r2

        xor    r0,r0
        jsr    @r1                    ! Get memory length
        nop

        mov    r0,r3                  ! Save in r3

        mov.l  @(_SendRece2SR,pc),r1

        _SetMemoryLoop:

        xor    r0,r0                  ! Clear r0

        jsr    @r1                    ! Get byte from pc
        nop

        mov.b  r0,@r2                 ! write byte into memory

        add    #0x1,r2                ! increment memory address
        dt     r3                     ! decrease length
        bf     _SetMemoryLoop         ! jump if length != 0
        nop
        
        mov.l  @r15+,r3
        mov.l  @r15+,r2
        mov.l  @r15+,r1
        mov.l  @r15+,r0
        lds.l  @r15+,pr

        rts
        nop


_UBCIntEntry:

! Command Set
! Command 1 : Step. Resets UBC for next instruction.
!             Format: NONE, just call the appropriate routine when sent.
! Command 2 : Run.  Clears UBC and continues execution of program
!             Format: NONE. Same as above.          
! Command 3 : Get Memory Range. Gets a specified memory range and send to PC
!             Format: PC sends memory start and length.
!                     Saturn sends memory data in byte units
! Command 4 : Get Registers. Get reg values Saturn -> PC
!             Format: Saturn sends register block to PC
! Command 5 : Set Memory. Sets a range of memory to values from PC (1/2/4)
!             Format: PC sends mem address, length and data in byte format.
! Command 6 : Set Registers. Set reg values PC -> Saturn
!             Format: PC sends register block and saturn rewrites the 
! Command 7 : Get PR and PC regsiters.
!             Format: transfers PR then PC saturn->pc
! Command 8 : Set hardware break point and start execution.
!             Sets both hardware bps and does a return.
!             Format: Get long A and B bp addresses..
! Command 9 : Reset program. Clears the UBC and rtes to the entry point.
!             Allows u to reload the original program in its correct state
!             and restart disasm.
!             FORMAT : None

	mov.l  r0,@-r15
	mov.l  r1,@-r15
 	mov.l  r2,@-r15
        mov.l  r3,@-r15
	mov.l  r4,@-r15
	mov.l  r5,@-r15
	mov.l  r6,@-r15
	mov.l  r7,@-r15
	mov.l  r8,@-r15
	mov.l  r9,@-r15
	mov.l  r10,@-r15
	mov.l  r11,@-r15
	mov.l  r12,@-r15
	mov.l  r13,@-r15
	mov.l  r14,@-r15
        mov    r15,r3
        add    #0x44,r3
        mov.l  r3,@-r15
        stc.l  gbr,@-r15
        stc.l  vbr,@-r15
        sts.l  macl,@-r15
        sts.l  mach,@-r15
	sts.l  pr,@-r15
	
	! Reset SRs I to original value in program but dont destroy stack copy (for T bit)
	! Start of Comm Routine
        
	! Setup UBC for next instruction and clear condition flags (delay for reset).	
	
        !xor     r0,r0
        !ldc     r0,sr                  ! Dont try this again. Ever.

        mov.l   @(_Connect2SR,pc),r1   ! Sync with PC. i.e. discard garbage
        jsr     @r1
        mov     #0x0,r0

        _CommLoop:                     ! Start of Comm code

        mov.l    @(_SendRece2SR,pc),r1 ! Get SendRecev pointer
        xor      r0,r0                 ! Clear r0
        jsr      @r1                   ! Get command value
        nop                            ! delay

        and      #0xF,r0               ! Mask off lower 16 values

        cmp/eq   #0x1,r0               ! Command 1. Step
        bf       _Skip1                ! If not command 1 then goto next
        nop

        bra      _RunUBC               ! Jump to set UBC routine
        nop

        _Skip1:

        cmp/eq   #0x2,r0               ! Command 2. Run
        bf       _Skip2
        nop

        bra      _ClrUBC               ! Jump to clear UBC and execute
        nop

        _Skip2:

        cmp/eq   #0x3,r0               ! Command 3. Get memory
        bf       _Skip3
        nop

        mov.l    @(_GetMemorySR,pc),r1 ! Call get memory routine
        jsr      @r1
        nop

        bra      _CommLoop             ! When exited jump back to start of 
        nop                            ! comm routine

        _Skip3:

        cmp/eq   #0x4,r0               ! Command 4. Get registers
        bf       _Skip4
        nop

        mov     #0x5C,r2               ! move byte length of regs
        mov     r15,r3                 ! move stack pointer to r3
        mov     #0x0,r0                ! clr r0
        _GRegLoop:
        mov.b   @r3,r0                 ! Get byte from stack
        add     #0x1,r3                ! Increment r3 pointer
        jsr     @r1                    ! Jump to send receive sr
        nop
        dt      r2                     ! Decrease length
        bf      _GRegLoop              ! Jump back if not finished
        nop

        bra     _CommLoop              ! When finished jump back to 
        nop                            ! Connect loop

        _Skip4:

        cmp/eq  #0x5,r0                ! Command 5 Set Memory
        bf      _Skip5
        nop

        mov.l   @(_SetMemorySR,pc),r1
        jsr     @r1
        nop

        bra     _CommLoop
        nop

        _Skip5:

        cmp/eq  #0x6,r0                ! Command 6. Set registers
        bf      _Skip6
        nop

        mov     #0x5C,r2               ! move byte length of regs
        mov     r15,r3                 ! move stack pointer to r3
        mov     #0x0,r0                ! clr r0
        _SRegLoop:
        jsr     @r1                    ! Get byte from PC
        nop

        mov.b   r0,@r3                 ! mov byte to reg location
        add     #0x1,r3                ! Increment r3 pointer
        dt      r2                     ! Decrease length
        bf      _SRegLoop              ! Jump back if not finished
        nop


        bra     _CommLoop
        nop

        _Skip6:

        cmp/eq  #0x7,r0
        bf      _Skip7
        nop

        mov     r15,r3       ! Setup r3 to point to stack base
        mov     #0x4,r2      ! Set offset
        _PRegL1:
        mov.b   @r3,r0       ! Transfer stacked PR reg
        jsr     @r1
        nop

        add     #0x1,r3
        dt      r2
        bf      _PRegL1
        nop

        add     #0x50,r3     ! Setup r3 to point to PC reg
        mov     #0x4,r2
        _PRegL2:
        mov.b   @r3,r0       ! Transfer PC reg
        jsr     @r1
        nop

        add     #0x1,r3
        dt      r2
        bf      _PRegL2
        nop

        bra     _CommLoop
        nop

        _Skip7:

        cmp/eq  #0x8,r0              ! Command to set hardware bps
        bf      _Skip8               ! And continue execution.
        nop

        bra     _CommLoop
        nop

        _Skip8:

        cmp/eq  #0x9,r0
        bf      _Skip9
        nop

        mov.l   @(_BootEntry,pc),r2
        mov     r15,r3
        add     #0x54,r3
        mov.l   r2,@r3
        bra     _ClrUBC
        nop

        _Skip9:

        bra     _CommLoop              ! Default return to loop (at the mo)
        nop


        !End of Comm routine
        
        _RunUBC:                       ! Step
        mov.l   @(_BARA,pc),r2         ! Set new address from stored pc
        mov     r15,r3                 ! Get stack pointer
        add     #0x54,r3               ! Add offset to stack pointer
        mov.l   @r3,r4                 ! Trasnfer PC to r3
        mov.l   r4,@r2                 ! Set new address value

        mov.l   @(_BRCRWord,pc),r3     ! Reset Cycle reg to allow new break
        mov.l   @(_BRCR,pc),r2
        mov.w   r3,@r2                 ! Write to reg
        mov.w   @r2,r3                 ! Set delay

        bra     _EndUBCInt
        nop

        _ClrUBC:                       ! Clears UBC and runs code

        mov.l   @(_BARA,pc),r2         ! Clear stored address
        xor     r4,r4                  ! Transfer 0 to r3
        mov.l   r4,@r2                 ! Set new address value

        xor     r3,r3                  ! Clr Cycle reg
        mov.l   @(_BRCR,pc),r2
        mov.w   r3,@r2                 ! Write to reg
        mov.w   @r2,r3                 ! Set delay


        _EndUBCInt:

        lds.l  @r15+,pr         ! Restore original regs from stack (or PC modified)
        lds.l  @r15+,mach
        lds.l  @r15+,macl
        ldc.l  @r15+,vbr
        ldc.l  @r15+,gbr
        mov.l  @r15+,r3         ! Remove r15 and destroy
        mov.l  @r15+,r14
	mov.l  @r15+,r13
	mov.l  @r15+,r12
	mov.l  @r15+,r11
	mov.l  @r15+,r10
	mov.l  @r15+,r9
	mov.l  @r15+,r8
	mov.l  @r15+,r7
	mov.l  @r15+,r6
	mov.l  @r15+,r5
	mov.l  @r15+,r4
	mov.l  @r15+,r3
	mov.l  @r15+,r2
	mov.l  @r15+,r1
	mov.l  @r15+,r0
	rte			! Jump back to original instuction
	nop

.align 4
_BARA:         .long 0xFFFFFF40
_BAMRA:        .long 0xFFFFFF44
_BBRA:         .long 0xFFFFFF48
_BBRAWord:     .long 0x00000056
_BRCR:         .long 0xFFFFFF78
_BRCRWord:     .long 0x00001400
_StackP:       .long 0x06002000
_GetMemorySR:  .long _GetMemory
_SetMemorySR:  .long _SetMemory
_Connect2SR:   .long _Connect
_SendRece2SR:  .long _SendRece
_GetLongSR:    .long _GetLong
_SendLongSR:   .long _SendLong
_UBCIntEntrySR:.long _UBCIntEntry
_BootEntry:    .long _entry

	.section	.data
	.align	4


Sega Saturn DeBuGger ver. 0.6 alpha. Readme file.
Linux i386 version.
Codin' by TyRaNiD (c) 2000

What this Program is:
--------------------

This program allows you to debug home-brew Sega Saturn code actually on ur saturn
with the help of a Pro Action Replay cart and a PC Commlink. Why u might want to 
this is open to debate :)

Program Requirements:
---------------------

-=Needed for running and debugging code=-

IBM compatible PC, at least a 386 and a linux install with ncurses 4.2 and
possibly glibc2 or libc5 I dont know which this program uses :/ 

A Sega Saturn. Any model should work from any country in theory.

Action replay cartridge. Also called a gameshark or Game Key depending
on where you live.

PC Commlink. A ISA board for your PC which allows connection to your Action Replay.
Without this the program hasnt got a hope of working. Sold mainly now for the psx
action replay but it should work on the saturn as well.

BIG NOTE !!!!!!!
----------------

This version will only work on EMS's 4in1 PAR+. This is due to the comm regs
being situated in different positions in the memory map on the datel model.
As I dont have a datel PAR/Gameshark then the program is not written for it.
However it would take me a few seconds to change it when it becomes more than
an alpha release, (as I do know the address's ;) )

The program also does not support the parallel port commlink developed by 
Freewing. Mainly due to lack of documentation and desire. Anyway the PP version
would be hella slow. If of course someone really really wants the support then email
me and ill see what I can do.

This version for Linux is more of a test release. I cant assure that it works
100% correctly unlike the dos version where im fairly certain it operate correctly
at least if used correctly.

This program is farrrrrrrrrr from finished yet. However I feel it has a
very long way to go as im surprised on how solid and easy using the UBC seems.
(lulled into a false sense of security no doubt)

USAGE:

First to run this program u need to either be root or set root permission on the
executable to allow u to get i/o permission.

U can set root permission by first being root then type :-

chown root saturn_debugger
chmod 4755 saturn_debugger

To start debugging first run the program from the shell,using the following command 
line parameters.

saturn_debugger filename [options]

filename is the name of the program to load. Required.

[options]

-lX  X is the load address of the program in hex
-pX  X is the port number where X is in 0x3X0 of the commlink port no.
-c   Force coff format. Required if input file doesnt have .cof extension.
-?   Displays help.

Notes on coff support.
I really hope it works with any coff file u want to throw at it but there might
be problems as ive not worked out the format from any official documentation. If
anyone know where I can find the exact specification for the coff format then plz
email me (a direct link please not just its on gccs wb site).

EXAMPLES:

Loading a binary file.

satdis.exe program.bin -l6004000
take note that u cannot add a 0x to the front of the address in this version.
Due to the function im using having different functionality under dos and 
linux.

Loading a coff file.

satdis.exe program.cof

NOTES

make sure the saturn is at the PARs main menu (or itll just hang :))
(Well it could hang if ur port address is incorrect but doh..)
Oh if it does hang u should be able to CTRL+C it.

The load program can be in bin or coff format. U need to specify the -c to use a coff file. 
If the program thinks its loading a coff file when u specify a binary file it will certainly 
crash the saturn and probably the debugger as well.

Config file format "satdebug.ini"

Comments r set by adding a # at the start of the line.

[options]

port=X. Allows u to specify the commlink port address. Can be overridden by passing
the -pX parameter.

Debugger commands (Preliminary key assigns mainly for testing rather than usage :))
-----------------

1     : Memory to Screen Dump. Type in the address and press PGUP and PGDOWN to move
        the memory dumping address by 256 byte blocks. Press ESC to exit.
        Real time memory editor. Pick the range of values in the memory dump to edit
        and press 1. Works like a standard hex editor and will modify values in real 
        time. Press TAB to switch between HEX mode and ASCII mode of editing. Press 
        ESC to quit and if u have modified any memory it will ask u to confirm the 
	write. If u press any other key than Y it will reset the memory as before 
        editing.
2     : Write a byte to memory.
	Type in address and value and it will write to memory
3     : Write a word to memory.
	Type in address and word value is written to memory. U dont have to specify
	a word boundry address as it will be able to write it no matter what.
4     : Write a long word to memory.
	Type in an address and long word value. U dont have to specify a long word
	boundry as it will write anyway.
5     : Modify a register. Type the register number (given by the side of the reg dump)
	and long value. New value will added and screen updated. It will also modify
	the disasm if u have modfied the PC reg.
	Oh and u cant modfify the VBR or the SP because this would screw the debugger up :)
	Be very careful with this. U could crash the saturn by puttin in a bad value i.e.
	a non long boundry reg when its the address for a long read.
	Also u cant set the I in SR flags to any value greater than 0xE (or any value with 
        the LSB set). This stops u from killing the debugger by disabling the ability to 
	call the UBC int which is hardware set at priority 15. (why u would want to do 
	this in the first place is beyond me).
6     : Dump memory block to a file. Type filename, start address and end
        address. (Note. Memory will be read up to end addr -1 ).
7     : Load file to memory. Type filename and start address. Precautions must be taken
	as with any of the memory routines to ensure u dont overwrite important data.
	Possibly only truly useful to quickly set a grafix screen to a know state.
8     : Run program. Execution can be cancelled at any time by pressing a key.
9     : Step into instruction. Executes 1 op and steps into a jump, branch.
0     : Step over. If executed on a sub routine jump will skip over it.
        Execution can be broken at any time by pressing a key. (note will destroy
        the call back breakpoint).
UP,
DOWN,
PGUP,
PGDOWN: Moves the disassembly by a certain amount. Up and down cursors move the disasm by
	1 instruction where pgup and down will move by 10 in the respective direction.
HOME  : Resets the disassembly to the current instruction position.
ALT+X : Quit the program and remove the UBC handler. Will allow program to properly 
	execute at full speed.
ALT+D : Dump from register value. Put in the regsiter dump and it will do a memory dump
	at the memory address pointed to by the register. R15 and VBR are included in this
	and have the reg nos of 15 and 16 respectivly. Note registers which cannot be used
	directly for memory addressing are not included. These being MACL,MACH and SR.
SPACE : Set a breakpoint on current instruction. Press again on instruction to remove.
ALT+L : Display current set breakpoints. When list produced allow setting and deleting
        breakpoints without finding them in the disasm.
ALT+I : Inline Asm if u have the gcc assembler installed. Replaces the currently selected
        instruction. N/A in the linux version as I dont have a cross compiler yet.
ALT+U : Undos the previous inline asm. If the asm command has compiled wrong then just
	use this command to reset the instruction. N/A
ALT+R : Reset the current program to its original location. Reloads program so it might
	take some time to restart.
'?'   : Display online help. Not very descriptive just useful if uve forgotten the key u 
	need.

The memory routines above will only accept certain address ranges at the moment.
To be honest I dont think there are any other main areas which rnt included.

Mem Areas
---------

0x(2)0000000 -> 0x(2)0080000 : Boot ROM     	(R)
0x(2)0180000 -> 0x(2)0190000 : Backup RAM   	(R)
0x(2)0200000 -> 0x(2)0300000 : Lower Work RAM  	(R/W)
0x(2)5C00000 -> 0x(2)5C80000 : VDP1 VRAM	(R/W)
0x(2)5E00000 -> 0x(2)5E80000 : VDP2 VRAM	(R/W)
0x(2)5F00000 -> 0x(2)5F01000 : VDP2 CRAM       	(R/W)
0x(2)6000000 -> 0x(2)6100000 : Upper Work RAM  	(R/W)

Obviously R and W specify the access available for the memory. I have disable write access
to the BIOS and the backup RAM for obvious reason (well I hope they r obvious if not go sit
in a corner until u figure it out).

And also the (2) signifies a cache through address so u can either. However memory dumping
and writing is best done on non cached addresses as this will reflect the programs likely
data.

MORE BIG NOTES!!!!!!! And bugs hehe.
------------------------------------

Here are know problems with the underlying system. Mostly out of my control.

1) Program wont seem to execute delayed ops, interrupt protected instructions
i.e. lds etc, and will not work in areas where the SRs I mask is set to 15.
The debugger should return correctly at the first possible instruction available
will just not be able to debug those instuctions. Due to limitations in the UBC. 
(Out of my control)

2) U can only load programs into the higher work ram, (Not really a problem
just a point to note). Will be remedied later on and allow loading to lower
as well.

3) If the size of the program + Start Address > 0x6040000 u will most likely
trash the UBC handler so try and make sure it dont. Ill move it to a better
position when its closer to finishing (basically when I know the rough total
code size of the bootloader).

4) I have not extensively tested the step over command so it might not work
inall cases. One case where it will not work is if a subroutine sets the SR
I mask to 15 and doesnt reset it till after the return. As no break would
occur on the instruction required it wouldnt work. However as this is blatent
bad coding practice so dont even go there Ricky.

5) U can only move the disasm within the upper work ram. This will only cause a problem 
if the program jumps to a bios routine or a lower work ram address then u will not be able
to move the disasm window. It should still execute however.

6) I think if u modify the instruction u are currently on using the mem commands it will
still execute the original instruction. Only way round this I think would be to do a cache
purge. If u need to get across a branch modify the sr reg instead or sommit.

7) When using the memory dump if u try and move to a address outside the boundries
of the memory block ur in it will give u an invalid memory range error and cancel 
the dump. Just irritating more than anything else. 

Things to do now.
-----------------

1) Add some actual functionality to the program. i.e. memory dump, run over
sub routine, modify registers etc. Need to add some meat to the bootloader.
DONE SOME OF IT. Still need to add breakpoints etc.

2) Make the disasm and reg display better and include the missing sh2 regs
(VBR, GBR, MACL, MACH). Didn't add them out of lazyness.
DONE sorta. Still needs alot of work doing to it.

3) Display a text screen on the saturn when the bootloader has started just
to give an indication that everythings working correctly.(Better than no
change at all which occurs at the moment). Probably can reuse PARS predefined
font :) Possibly wont bother. I might just add a load percentage bar on the pc
side to indicate the program is loading.

4) Add a command to call back from inside a procedure (using the PR reg).
Should in theory be able to drop back to the last sub routine jump.

5) Stuff, stuff and more stuff :) Of course source level debugging far in
da future (i.e. when I have time to rip off the DJGPP rhide debugger parser
;), but at least it looks like it can be done with some hard work). 

DISCLAIMER.
----------

I do not provide any warranty on the security of your data on both ur saturn or
pc. While this program is extremely unlikely to damage anything there is always 
the tiny chance of it doing so. If used responsibly and correctly nothing will 
ever happen and thus I take no responsibilty for any damage cause through the use
of this proggy.

This program is not affilated or approved by either Sega or the manufacturer of
your Action Replay. Plz do not contact them with any problems u have as they will
invariably tell u to go f*#k urself.

Revision History:
----------------

0.6 Linux : Linux port of 0.6a. Wahey.
	    Inline Asm has been removed in this version and the colors arnt the same
	    as the dos version.
	    The colors are not the same as the dos version for reason best left to the
	    writers of curses.
	    The keys for controlling the debugger have been changed once again due to 
	    differing ideas between curses and dos key codes and operation.
	    I dont know how compatible this version will be between different linux installs
	    and machines.
0.6 Alpha : Real time memory editing facility. HEX/ASCII editor format.
            Added a program reset using the ALT + R command.
	    Finally added cached through memory addresses to the memory routines.
            User cant set a non word boundry PC,PR or invalid SR reg value now.
	    Added a breakpoint editor to disable and set new breakpoints without 
            directly setting them in the disasm window.
	    Added easy to read bit by bit display of the SR reg (with bit names above).
	    Added a load progress bar on the pc side to indicate a program is indeed 
	    loading. 
	    Added a load file to memory command. NOTE F5 and F6 commands reversed.
	    Added an undo command for the last inline asm command. Useful if uve
	    have assembled the wrong instruction.
	    Rnt u lucky boyz and girls. I finally found how to delete filez so no more
	    files left about cause of gas :)
	    Fixed a few things which might have caused problems in last release on
            some systems (what systems? No one is using the program :))
0.5 Alpha : A super cheat added. Inline assembler function as long as u have
            the gcc assembler gas installed and setup in the config file :)
            Config file system make it less hassle to pass the port number etc.
            Preliminary coff file support so u dont even have to type in the
            programs load/start address :)
	    Modified the program parameters slightly.
	    Made sure any configuration errors where possible occur before the loading
            of the boot code. Means u shouldnt have to reset ur saturn if u type in an
            invalid load file.
0.41 Alpha: Fixed a bug which caused most of the programs to crash. Its been there
	    since 0.2a. Whoops..
0.4 Alpha : First public release.
	    Added breakpoint commands and some pretty colours :).
	    Added online help command. Wahey :P
0.3 Alpha : Added more commands. Write byte,word,long. Modify registers.
6/2/2000    Reordered the key assigns to make them slightly easier. (well so
	    dus dont have to press the shift key :) ).
	    Added movement of disasm window. However its not much use at the mo.
	    Slightly adjusted the memory dump to allow u to go back through the address
	    range as well as forward.
	    Added a memory dump from register value command. 
	    Fixed a (possible!) bug in the step over command which might occur with
	    a lds instruction modifying the pr regsiter. Now will only cause a step 
	    over if the pr reg value is equal to the step pc reg.

0.2 Alpha : Added some proper commands.Step in, step over, mem to file, mem
3/2/2000    dump. Prelim key assigns (will change soon as they r crappy).
            Made the interface abit neater.
            Fixed some so called bugs/niggles which needed addressing first,
            i.e. adding all the regs in the Read Reg command (fucked alot up:)
            Added a new comm routine command which speeds up continuous
            execution by approx 2000% hehe :)

0.1 Alpha : Initial test release.
2/2/2000

Program written using DJGPP C compiler (for DOS side) and GCC SH2 assembler 
(for Saturn side).

This program is all my own code and so is copyrighted by me :) It does not use
any commercial libraries or code from either the pc or saturn side so im not 
stepping on anybodies toes.

Thanx and Greetz:
-----------------

Thanx to dus for program testing and all the other benefits u bestow upon me :)
Thanx to PI for very helpful PAR documentation/sample code/and for showing that u
can even run programs on a saturn at all.
Thanx to Charles Doty for his long gone saturn coding site :( Where r u man ? 
Thanx to SEGA for producing a machine which seems fairly easy to make a debugger 
for :)
Thanx to Datel UK for producing the PAR cart in the first place.
Thanx to MGhandi (www.kinox.org) for being the only emusite webmaster who
bothered to put up a mention of my debugger.

Greetz to Azuco, of A-Saturn.
Greetz to Alucard and Tunek. 
Greetz to the EFnet crew, Gato, Vitor, Ame, Albx, Cowering, and all the other ppl
ive met along my way into obscurity.  

If u feel uve been missed out of the list then I dont care, haha.

Contact info:
-------------

Email : TyRaNiD@020.co.uk. For ya bug reporting, suggestions, gripes and general 
lamentations.
EFnet : nick is [TyRaNiD]. U might be able to find me maybe. Come to #x68000 for
japanese computer emulation and manylong nights of chatting bollox.

Thought for the day..
---------------------

F#'k u Y�&k*e S'#m.

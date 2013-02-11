# MAKEFILE for Saturn Debugger ver 0.7a

saturn_debugger : main.o sh2diss.o conio.o ports.o iasm.o
		  gcc -s -o saturn_debugger main.o sh2diss.o \
		        iasm.o conio.o ports.o -lncurses

main.o : main.c
	 gcc -c main.c
sh2diss.o : sh2diss.c
	    gcc -c sh2diss.c
iasm.o  : iasm.c
	  gcc -c iasm.c
conio.o : conio.c
	  gcc -c conio.c
ports.o : ports.c
	  gcc -c ports.c 

clean : 
	rm saturn_debugger main.o sh2diss.o conio.o ports.o iasm.o

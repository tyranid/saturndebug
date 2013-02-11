# MAKEFILE for Saturn Debugger ver 0.7a

saturn_debugger : main.o sh2diss.o iasm.o
		  gcc -s -O2 -o satdis.exe main.o sh2diss.o \
		        iasm.o

main.o : main.c
	 gcc -O2 -c main.c
sh2diss.o : sh2diss.c
	    gcc -O2 -c sh2diss.c
iasm.o  : iasm.c
	  gcc -O2 -c iasm.c

clean : 
	rm satdis.exe main.o sh2diss.o iasm.o

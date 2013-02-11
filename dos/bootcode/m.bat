as bootload.s -o bootload.o
ld bootload.o -T sal.lnk -o bootload.cof -Map bootload.map
objcopy -O binary bootload.cof bootload.bin

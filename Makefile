
all: gamebleugh

gamebleugh: gamebleugh.o mmu.o cpu.o decode.o ops.o gpu.o
	gcc -lsixel -o gamebleugh gamebleugh.o mmu.o cpu.o decode.o ops.o gpu.o

.o:
	gcc -c $<

clean:
	rm -f gamebleugh
	rm -f *.o


all:
	gcc  -m32 -no-pie -nostdlib -o sum sum.c
	gcc -m32 -o loader loader.c

clean:
	-@rm -f sum loader

CC=gcc
CFLAGS= -O2

blcount:
	$(CC) $(CFLAGS) *.c -o blcount

install: blcount
	cp blcount /usr/bin/
	cp bible.txt /usr/share/bible.txt

clean:
	rm -f *.exe blcount *.out *.o

uninstall:
	rm -f /usr/bin/blcount
	rm -f /usr/share/bible.txt

gitu: clean
	git add .;git commit -m "F";git push

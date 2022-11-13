CC=gcc
CFLAGS= -O2


blcount:
	$(CC) $(CFLAGS) *.c -o blcount

drblcount:
	$(CC) $(CFLAGS) *.c -D BH_VERSE_STRUCTURE -DBIBLE_FILE_LOC_LOCAL="\"drb.txt\"" -DBIBLE_FILE_LOC_INSTALLED="\"/usr/share/drb.txt\"" -o drblcount

cpdvlcount:
	$(CC) $(CFLAGS) *.c -D BH_VERSE_STRUCTURE -DBIBLE_FILE_LOC_LOCAL="\"cpdv.txt\"" -DBIBLE_FILE_LOC_INSTALLED="\"/usr/share/cpdv.txt\"" -o cpdvlcount

install: blcount
	cp blcount /usr/bin/
	cp bible.txt /usr/share/bible.txt

clean:
	rm -f *.exe blcount drblcount cpdvlcount *.out *.o

uninstall:
	rm -f /usr/bin/blcount
	rm -f /usr/share/bible.txt

gitu: clean
	git add .;git commit -m "F";git push

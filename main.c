#include <stdio.h>
#include <math.h>
#include "stringutil.h"
#ifndef BIBLE_FILE_LOC_LOCAL
#define BIBLE_FILE_LOC_LOCAL "bible.txt"
#endif
#ifndef BIBLE_FILE_LOC_INSTALLED
#define BIBLE_FILE_LOC_INSTALLED "/usr/share/bible.txt"
#endif
FILE* bible_file = NULL;
typedef struct{
	char* text;
	char* bookname;
	long chapter;
	long verse;
	long vowels;
	long consonants;
	long letters;
	long characters;
	long spaces;
	long odd_characters;
	long capitals;
	long lowercases;
} bibleverse;

/**/
bibleverse create_verse_struct(char* orig_text){
	bibleverse returnme;
	static const bibleverse empty = {0};
	returnme = empty; /*initialize to NULL*/
	long loc;
	/*skip the first two tabs.*/
	/*first, find the tab after the book number (1-66)*/
	loc = strfind(orig_text, "\t"); if(loc <1) goto need_to_free;
	/*we have found the tab that is immediately after the book number, before the book abbreviation.*/
	loc++;orig_text += loc; /*skip that tab!*/
	
	loc = strfind(orig_text, "\t"); if(loc <1) goto need_to_free;
	/*we have found the tab that is immediately after the book abbreviation, before the book name*/
	loc++;orig_text += loc; /*skip that tab!*/
	

	/*We have the location of the book name in the string, but not the end of it.*/
	loc = strfind(orig_text, "\t"); if(loc <1) goto need_to_free;
	orig_text[loc] = '\0';
	returnme.bookname = strcatalloc("",orig_text); /*FROM HERE ON OUT, WE HAVE ALLOCATED SOMETHING! MUST USE NEED_FREE*/
	orig_text[loc] = '\t'; /*return it to its previous state.*/
	loc++;orig_text += loc; /*skip the tab after the book name*/
	/*get the chaper number and verse number.*/
	/*chapter number is immediately after the tab after the book name.*/
	/*there is then another tab between chapter number and verse number.*/
	returnme.chapter = strtol(orig_text,0,10);
	loc = strfind(orig_text, "\t"); if(loc <1) goto need_to_free;
	loc++;orig_text+=loc;
	returnme.verse = strtol(orig_text,0,10);

	/*after verse number there is another tab before the actual text of the verse.*/
	loc = strfind(orig_text, "\t"); if(loc <1) goto need_to_free;
	loc++;orig_text += loc;
	/*and finally, we have our text!*/
	if(strlen(orig_text) < 3) goto need_to_free;
	returnme.text = strcatalloc("",orig_text);
	/*printf("VERSE TEXT:\r\n%s\r\nBOOK: %s, CHAPTER: %ld VERSE: %ld\r\n",returnme.text, returnme.bookname, returnme.chapter, returnme.verse);*/
	return returnme;
	
	/*This portion is only executed in case of an error.*/
	need_to_free:
		if(returnme.text) free(returnme.text);
		if(returnme.bookname) free(returnme.bookname);
		puts("[VERSE WITH INVALID SYNTAX DETECTED!]\r\n");
	return empty;
}



void verse_print(bibleverse* v, int mode){
	if(mode == 0) /*basic*/
		printf("%s\t\t%ld:%ld\r\n",v->bookname,v->chapter,v->verse);
	else if(mode == 1)
		printf("%s\t\t%ld:%ld\r\n%s\r\n",v->bookname,v->chapter,v->verse,v->text);
}

/*bibleverse* qualifying_verses = NULL;*/
long long nverses = 0;
long long nverses_lettercount = 0;
long long nverses_vowelcount = 0;
long long nverses_consonantcount = 0;

char* vowels;
char* consonants;
long long vowel_count;
long long consonant_count;
long long letter_count;
int y_is_vowel = 0;
int soft_recognition = 0;
int commandline_asked_for_match_mode = 0;
char* vowels_y_is_vowel = "aeiouyAEIOUY";
char* vowels_y_is_not_vowel = "aeiouAEIOU";
char* consonants_y_is_vowel = "bcdfghjklmnpqrstvwxzBCDFGHJKLMNPQRSTVWXZ";
char* consonants_y_is_not_vowel = "bcdfghjklmnpqrstvwxyzBCDFGHJKLMNPQRSTVWXYZ";


void verse_count_characters(bibleverse* v, char* lvowels, char* lconsonants){
	if(!v) return;
	if(!v->text) return;
	char* q = v->text;
	/*if the passed in lists of characters are empty, use sensible defaults.*/
	if(!y_is_vowel){
		if(!lvowels) lvowels = vowels_y_is_not_vowel;
		if(!lconsonants) lconsonants = consonants_y_is_not_vowel;
	} else if(y_is_vowel){
		if(!lvowels) lvowels = vowels_y_is_vowel;
		if(!lconsonants) lconsonants = consonants_y_is_vowel;
	}
	v->vowels =0;
	v->consonants =0;
	v->characters =0;
	v->letters =0;
	v->odd_characters =0;
	v->spaces =0;
	while(*q != '\0')
	{
		int is_vowel = 0;
		int is_consonant = 0;
		
		if(*q == '\0') break;

		if(strchr(lvowels,*q) != NULL) 			{v->vowels++;		is_vowel = 1;}
		if(strchr(lconsonants,*q) != NULL) 		{v->consonants++;	is_consonant = 1;}

		if(*q == ' ') 
			v->spaces++;
		else if(!is_vowel && !is_consonant) 
			v->odd_characters++;

		if(is_consonant || is_vowel){
			if((*q <= 'Z') && (*q >= 'A')) v->capitals++;
			if((*q <= 'z') && (*q >= 'a')) v->lowercases++;
			v->letters++;
		}
		if(is_consonant && is_vowel) printf("[BOTH CONSONANT AND VOWEL ERROR? HUH? '%c']",*q); /*Why didn't I put it inside the above if clause? idk /shrug*/
		v->characters++;
		q++;
	}
	
}

void biblesearch(int searchmode, char* bookname, long chapter, long verse){ /*mode 0 is to seek verses matching a criterion,*/
	char* line = NULL;
	unsigned long lenout;
	bibleverse currentverse;
	if(searchmode != 0 && searchmode != 1)
		{puts("\r\n Invalid search mode.\r\n"); exit(1);}
	/*line grabbing*/
	while(!feof(bible_file)){
		if(line) free(line);
		line = read_until_terminator_alloced(bible_file, &lenout, 0xa, 1000);
		if(line)if(strlen(line) < 4){ /*Do not even attempt to parse a line */
			free(line); line = NULL; continue;
		}
		currentverse = create_verse_struct(line);
		if(!currentverse.text) goto attempt_free; /*invalid line?*/
		if(!currentverse.bookname) goto attempt_free; /*invalid bookname?*/
		/*invalid chapter and verse?*/
		if(currentverse.chapter < 1) goto attempt_free;
		if(currentverse.verse < 1) goto attempt_free;
		/*We have a valid verse struct, count vowels and whatnot!*/
		verse_count_characters(&currentverse, vowels, consonants);
		/*
			if the verse matches ANY of the criterion, print out the 
		*/
		if(searchmode == 0){
			if(currentverse.vowels == vowel_count || currentverse.consonants == consonant_count || currentverse.letters == letter_count)
			{
				int printmode = ((currentverse.vowels == vowel_count) && (currentverse.consonants == consonant_count) && (currentverse.letters == letter_count))?1:0;

				if(printmode == 0) 
				if(soft_recognition)
				{
					if(currentverse.vowels == vowel_count) printf("[vowels]");
					if(currentverse.consonants == consonant_count) printf("[consonants]");
					if(currentverse.letters == letter_count) printf("[consonants]");
				}
				if((soft_recognition && (printmode == 0)) || (printmode == 1)  )
				verse_print(&currentverse,printmode);

				if(vowel_count == currentverse.vowels) nverses_vowelcount++;
				if(consonant_count == currentverse.consonants) nverses_consonantcount++;
				if(letter_count == currentverse.letters) nverses_lettercount++;
				if(vowel_count == currentverse.vowels) if(consonant_count == currentverse.consonants) nverses++;
			}
		}
		else if(searchmode == 1){
			/*for each string, convert every character to uppercase.*/
			char* bn = bookname;
			while(*bn != '\0') {if( (*bn <= 'z') && (*bn >= 'a') ) {*bn -= 'a'; *bn += 'A';} bn++;}
			bn = currentverse.bookname;
			while(*bn != '\0') {if( (*bn <= 'z') && (*bn >= 'a') ) {*bn -= 'a'; *bn += 'A';} bn++;}
			/*compare the two.*/
			if(strcmp(bookname,currentverse.bookname)) goto attempt_free;
			/*The book names are the same. compare the chapter and verse.*/
			if((chapter == currentverse.chapter) && (verse == currentverse.verse))
			{
				verse_count_characters(&currentverse,NULL,NULL); /*the function will pick sensible defaults.*/
				vowel_count = currentverse.vowels;
				consonant_count = currentverse.consonants;
				letter_count = currentverse.letters;
				printf(
					"%s %ld:%ld has %lld vowels, %lld consonants, and %lld letters. Text:\r\n%s\r\n~~~~~~~~~~~~~~~~~~~~~~~\r\n\n\n\n",
					bookname,chapter,verse,
					vowel_count,consonant_count,letter_count,
					currentverse.text
				);
				if(line){free(line);line = NULL;}
				if(currentverse.text){free(currentverse.text);currentverse.text = NULL;}
				if(currentverse.bookname){free(currentverse.bookname);currentverse.bookname = NULL;}
				return;
			}
		}
		attempt_free:;
		if(line){free(line);line = NULL;}
		if(currentverse.text){free(currentverse.text);currentverse.text = NULL;}
		if(currentverse.bookname){free(currentverse.bookname);currentverse.bookname = NULL;}
	}
	if(searchmode == 0){
		printf("\r\n~~~~\r\nIn Total, there were %lld verses that matched the criterion\r\n~~~~\r\n",nverses);
		printf("\r\n~~~~\r\nMatched lettercount: %lld, Matched Vowelcount: %lld,Matched consonant count: %lld\r\n~~~~\r\n", nverses_lettercount, nverses_vowelcount,nverses_consonantcount);
	} else if(searchmode == 1){
		puts("\r\nUnable to find the requested chapter and verse.\r\n");
	}
}

void open_bible_file(){
	bible_file = fopen(BIBLE_FILE_LOC_LOCAL, "r");
	if(bible_file) return;
	bible_file = fopen(BIBLE_FILE_LOC_INSTALLED, "r");
	if(bible_file) return;
	printf("\r\nUnable to locate bible file either in the current working directory or the installation location.\r\n");
	exit(1);
}


int main(int argc, char** argv){
	open_bible_file();
	help:;
	if(argc < 3){
		puts("KJV Bible Text Numerics Analyzer (CC0 software)");
		puts("Written for assistance in proving the inspiration of the King James Version by the God of Israel.");
		puts("Jesus the Nazarene is the Lord.");
		puts("bible.txt complimentary of bibleprotector.com");
		puts("To study KJV variants, simply alter bible.txt to match your KJV edition.");
		puts("[Analyzing the original spelling of the 1611 is currently not implemented]");
		puts("See http://www.bibleprotector.com/editions.htm for a list of recorded variants.");
		puts("blcount vowel_count consonant_count [y_is_vowel=0]");
		puts("You can also use a particular bible verse as a template to get letter/consonant/vowel counts.");
		puts("blcount -1 bookname chapternumber versenumber [y_is_vowel=0]");
		puts("Try analyzing Genesis 1:1, 2 Corinthians 10:9, or Revelation 22:21!");
		return 1;
	}
	vowel_count = strtoll(argv[1],0,0);
	if(vowel_count == -1){
		/*All of the following arguments are: bookname, chapternumber, versenumber, [y_is_vowel]*/
		commandline_asked_for_match_mode = 1;
		/*Do we have enough arguments?*/
		if(argc < 5) goto help;
		if(argc > 5) y_is_vowel = strtol(argv[5],0,0); else y_is_vowel = 0;
		/*Harvest data from a particular chapter and verse.*/
		biblesearch(1,argv[2],strtol(argv[3],0,10), strtol(argv[4],0,10));
		fclose(bible_file);
		open_bible_file();
	}
	if(commandline_asked_for_match_mode == 0)
	{
		consonant_count = strtoll(argv[2],0,0);
		letter_count = vowel_count + consonant_count;
		y_is_vowel = 0; /*Default: No.*/
		if(argc >3 && !commandline_asked_for_match_mode) /*Not  */
			y_is_vowel = strtol(argv[3],0,0);
	}
	if(y_is_vowel){
		consonants = consonants_y_is_vowel;
		vowels = vowels_y_is_vowel;
	}
	else{
		consonants = consonants_y_is_not_vowel;
		vowels = vowels_y_is_not_vowel;
	}

/*Setup variables for line grabbing.*/
	biblesearch(0, NULL, 0,0);
	return 0;
}

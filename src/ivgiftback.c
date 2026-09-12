/*
Use research to make an initial amemone gift.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <time.h>
#include <math.h>
#include <sys/stat.h>

/*
Translates ASCII (in the final version, Unicode) into Gen IV Encoding.
Right now, this will only work for non-Japanese copies of the game.
Numbers, uppercase letters and lowercase letters will be translated using offsets.
Everything else will be translated using a premade hashmap.
*/
void writeText(unsigned char* buffer, unsigned int* index, const unsigned int max_bytes, const char* msg) {
	unsigned int i = *index;
	const size_t msg_len = strlen(msg);
	
	for (size_t j = 0; j < max_bytes; j++) {
		uint16_t input = 0;
		if (j < msg_len) {
			char c = msg[j];
			if ('0' <= c && c <= '9') { //numeric
				input = 0x0121 + (c - '0');
			}
			else if ('A' <= c && c <= 'Z') { //uppercase 
				input = 0x012b + (c - 'A');
			}
			else if ('a' <= c && c <= 'z') { //lowercase 
				input = 0x0145 + (c - 'a');
			}
			else { //hashmap
				//dont have a hashmap so compare basic punctuation for now.
				if (c == '.') {
					input = 0x01ae;
				}
				else if (c == ',') {
					input = 0x01ad;
				}
				else if (c == '!') {
					input = 0x01ab;
				}
				else if (c == '?') {
					input = 0x01ac;
				}
				else if (c == ':') {
					input = 0x01c4;
				}
				else if (c == ';') {
					input = 0x01c5;
				}
				else if (c == ' ') {
					input = 0x01de;
				}
				else if (c == '\n') {
					input = 0xe000; //undocumented newline character
				}
			}
		}
		else {
			//we have gone over our character limit, so we write ffff
			input = 0xffff;
		}

		//once an input has been received, write it.
		buffer[i++] = (unsigned char) (input & 0x00ff);
		buffer[i++] = (unsigned char) (input >> 8);
	}

	//update index
	*index = i;
}	

int main(void) {
	printf("Hello, welcome to IVgift!\n");
	
	//begin by making a folder
	mkdir("gifts", 0777);
	
	//then, a file in said folder
	const char* file_name_base = "amemone_shroomish";
	
	//we have one character buffer for both pcd and pgt.
	//since pgt matches first part of pcd.
	//pcd file is 856 bytes.
	
	unsigned char buffer[1000] = {0};
	unsigned char* ptr = buffer;
	unsigned int i = 0;

	//its a pokemon, with the OT being the receiver of the pokemon.
	buffer[i++] = 0x01;
	buffer[i++] = 0x00;

	buffer[i++] = 0x00;
	buffer[i++] = 0x00;
	
	buffer[i++] = 0x01; //IVgift OT
	buffer[i++] = 0x00;
	
	buffer[i++] = 0x00;
	buffer[i++] = 0x00;
	
	//now we write our encrypted pokemon data.
	//its always 236 bytes.
	unsigned char pkmn[236] = {0};
	FILE* pkmn_file = fopen("encryptedPokemon/amemone_shroomish.ek4", "rb");
	if (!pkmn_file) {
		fprintf(stderr, "could not open encrypted pokemon file\n");
		return 1;
	}
	fread(pkmn, sizeof(unsigned char), 236, pkmn_file);
	fclose(pkmn_file);

	//now copy into our file
	for (int j = 0; j < 236; j++) {
		buffer[i++] = pkmn[j];
	}
		
	
	//now we can print our custom message
	buffer[i++] = 'T';
	buffer[i++] = 'H';
	buffer[i++] = 'X';
	buffer[i++] = ' ';
	buffer[i++] = '4';
	buffer[i++] = ' ';
	buffer[i++] = 'I';
	buffer[i++] = 'V';
	buffer[i++] = 'G';
	buffer[i++] = 'I';
	buffer[i++] = 'F';
	buffer[i++] = 'T';
	buffer[i++] = '!';
	buffer[i++] = ' ';
	buffer[i++] = '<';
	buffer[i++] = '3';

	//at this point, we can write our file directly to the .pgt file
	FILE* gift_pgt = fopen("gifts/amemone_shroomish.pgt", "wb");
	fwrite(buffer, sizeof(unsigned char), i, gift_pgt);
	fclose(gift_pgt);
	
	printf("PGT made\n");

	//now we have our title text.
	//72 bytes, but each character code is 2 bytes.
	//so we have 36 characters to work with.
	//write a basic translation function.
	
	const char* title_msg = "AMEMONE the Shroomish has arrived!?"; //to be safe, 35 characters.
	writeText(buffer, &i, 36, title_msg);
	
	printf("title written\n");

	//now we write (game compatibility flags) 0000 (wonder card id) 0d00 <- MYSTERY BYTE
	
	uint16_t game_flags = 0;
	const uint16_t
	DIAMOND = 1 << 2,
	PEARL = 1 << 3,
	PLATINUM = 1 << 4,
	HG = 1 << 15,
	SS = 1 << 0;
	
	//make it compatible with everything for now.
	game_flags = DIAMOND | PEARL | PLATINUM | HG | SS;
	buffer[i++] = (unsigned char) (game_flags >> 8);
	buffer[i++] = (unsigned char) (game_flags & 0x00ff);
	
	printf("flags written\n");

	buffer[i++] = 0x00;
	buffer[i++] = 0x00;
	
	//wonder card id, set to the assumed max of 2046.
	const uint16_t wc_id = 2046;
	buffer[i++] = (unsigned char) (wc_id >> 8);
	buffer[i++] = (unsigned char) (wc_id & 0x00ff);
	
	//mystery byte. you can experiment here.
	//normally 0x0d00
	buffer[i++] = 0xff;
	buffer[i++] = 0xff;
	
	printf("wc id and mystery bytes written\n");

	//description text, maximum of 0x347 - 0x154 = 499 bytes or 500 bytes or something.
	//half of it, 250 character limit.
	const char* desc_msg = "Somehow, AMEMONE the Shroomish has\ntraveled through space and time\nto join you on your adventure!\nThank you for using IVgift! :D";
	writeText(buffer, &i, 250, desc_msg);
	
	printf("description written\n");

	//now we have: (distribution count) (pokemon icon left) (pokemon icon middle) (pokemon icon right) 0000 0000 (date received) 0000
	
	//count goes up to 255 = 0x00ff
	buffer[i++] = 0xff;
	buffer[i++] = 0x00;
	
	printf("wc count written\n");

	//for icons, make them all shroomish.
	//its the pokedex number, 0x011d
	const uint16_t dex_num = 0x011d;
	//left
	buffer[i++] = (unsigned char) (dex_num & 0x00ff);
	buffer[i++] = (unsigned char) (dex_num >> 8);
	//middle
	buffer[i++] = (unsigned char) (dex_num & 0x00ff);
	buffer[i++] = (unsigned char) (dex_num >> 8);
	//right
	buffer[i++] = (unsigned char) (dex_num & 0x00ff);
	buffer[i++] = (unsigned char) (dex_num >> 8);
		
	printf("icons written\n");
	
	buffer[i++] = 0x00;
	buffer[i++] = 0x00;
	
	buffer[i++] = 0x00;
	buffer[i++] = 0x00;

	//date
	//use the standard library.
	struct tm epoch; //beginning of time for the date
	struct tm date; //out current date for the event.
	memset(&epoch, 0, sizeof(struct tm));
	memset(&date, 0, sizeof(struct tm));

	//some constants
	const uint32_t
	linux_epoch_year = 1900,
	linux_month_dif = 1;

	//epoch begins on 01/01/2000
	epoch.tm_year = 2000 - 1900;
	epoch.tm_mon = 1 - 1; //months are 0 indexed
	epoch.tm_mday = 1; //days are 1 indexed.
	//hour, minute, and second are 0
	epoch.tm_isdst = -1; //daylight savings, let system decide.
	
	//for our shroomish event, lets use 23/03/2006 for no reason
	date.tm_year = 2006 - 1900;
	date.tm_mon = 3 - 1; //months are 0 indexed
	date.tm_mday = 23 - 1; //days are 1 indexed.
	//hour, minute, and second are 0
	date.tm_isdst = -1; //daylight savings, let system decide.
	

	printf("date structs made\n");
	
	time_t epoch_t = mktime(&epoch);
	time_t date_t = mktime(&date);
	
	printf("time_t made\n");

	double dt = difftime(date_t, epoch_t);
	//add half a day in seconds to round safely and to protect against DST,
	//and divide by seconds in a day
	uint16_t days = (uint16_t) round((dt + 43200) / 86400);
	
	printf("days calculated\n");

	buffer[i++] = (uint8_t) (days & 0x00ff);
	buffer[i++] = (uint8_t) (days >> 8);
	

	printf("date written\n");
	
	buffer[i++] = 0x00;
	buffer[i++] = 0x00;

	//finally, write directly into the .pcd file
	FILE* gift_pcd = fopen("gifts/amemone_shroomish.pcd", "wb");
	fwrite(buffer, sizeof(unsigned char), i, gift_pcd);
	fclose(gift_pcd);

	printf("PCD made\n");

	//There is one more file format to know of: .myg
	//This is used by Nintendo Wifi Connection.
	//We can use the code in https://github.com/AdmiralCurtiss/MysteryGiftConvert to help us here.
	
	//According to the repository, we can copy the 80 (0x50) bytes from our PCD, starting at 0x104.
	//Then write out the PCD.

	unsigned char myg_buffer[1500] = {0};
	unsigned int myg_index = 0;
	for (unsigned int j = 0x104; myg_index < 0x50; j++) {
		myg_buffer[myg_index++] = buffer[j];
	}
	for (unsigned int j = 0; j < 856; j++) {
		myg_buffer[myg_index++] = buffer[j];
	}

	FILE* gift_myg = fopen("gifts/amemone_shroomish.myg", "wb");
	fwrite(myg_buffer, sizeof(unsigned char), myg_index, gift_myg);
	fclose(gift_myg);

	printf("MYG made\n");

	return 0;
}

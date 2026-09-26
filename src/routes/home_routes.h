#include "../cottage/cottage.h"
#include "../base64/base64.h"
#include "pokemon_data.h"

#define SwapEndian(x) (x >> 8) | (x << 8)

typedef enum GiftType {
   POKEMON = 1,
   EGG,
   ITEM
} GiftType;

typedef struct WonderCard {
   GiftType type;
   bool pokemon_OT_flag;
   uint8_t* ek4;
   uint16_t item_id;
   char* title;
   char* description;
   uint16_t icon_left;
   uint16_t icon_middle;
   uint16_t icon_right;
   uint16_t distrib_count;
   uint16_t wc_id;
   char* distrib_date;
   uint16_t game_flags;

   uint16_t index;
} WonderCard;


WonderCard readPGT(uint16_t* bin) {
   const uint16_t size = 260;
   uint16_t index = 0;
   WonderCard card;
   WonderCard error;
   memset(&card, 0, sizeof(WonderCard));
   memset(&error, 0, sizeof(WonderCard));
   if (!bin) return card;

   //Gift type;
   switch (bin[index++]) {
      case 0x0001: {
         card.type = POKEMON;
         break;
      }
      case 0x0002: {
         card.type = EGG;
         break;
      }
      case 0x0003: {
         card.type = ITEM;
         break;
      }
      default: {
         //Must be one of these 3.
         return error;
      }
   }

   index++; //0x0000

   //Gift specific data.
   if (card.type == ITEM) {
      //This field contains our item index.
      card.item_id = bin[index++];
   }
   else {
      //This field contains our OT check.
      //00 means true
      card.pokemon_OT_flag = !(bin[index++]);
   }

   index++; //0x000

   //Pokemon data.
   if (card.type != ITEM) {
      //An encrypted pokemon ek4 file is 236 bytes.
      card.ek4 = (uint8_t*) calloc(236, sizeof(uint8_t));
      if (!card.ek4) return error;
      for (int i = 0; i < 236;) {
         // uint16_t bin_low = (uint16_t)bin[inde++];
         // uint16_t bin_high = (uint16_t)bin[j++];
         // uint16_t bin_short = (bin_high << 8) | (bin_low & 0x00ff);
         uint16_t bin_short = bin[index++];
         uint16_t bin_high = (bin_short >> 8) & 0x00ff;
         uint16_t bin_low = bin_short & 0x00ff;
         card.ek4[i++] = bin_low;
         card.ek4[i++] = bin_high;
      }
   }

   //Remainder bytes.
   index += (16 >> 1);

   card.index = index;

   return card;

}

uint32_t writeUnicodeChar(uint8_t* str, uint32_t input) {
   if (!str) return 0;

   uint32_t bytes = 0; //How many bytes are we writing?
   uint32_t code = 0; //Stores our current code

   /*
   The input can be of a range of values of different byte lengths:
      0x000000 -> 0x00007F : 1 byte(s)
      0x000080 -> 0x0007FF : 2 byte(s)
      0x000800 -> 0x00FFFF : 3 byte(s)
      0x010000 -> 0x10FFFF : 4 byte(s)
   */
   const uint32_t
   byte_1_start = 0x000000,
   byte_1_end   = 0x00007F,

   byte_2_start = 0x000080,
   byte_2_end   = 0x0007FF,

   byte_3_start = 0x000800,
   byte_3_end   = 0x00FFFF,

   byte_4_start = 0x010000,
   byte_4_end   = 0x10FFFF;

   if (byte_1_start <= input && input <= byte_1_end) { //ASCII
      bytes = 1;
      str[0] = (uint8_t)input;
      goto end;
   }
   if (byte_2_start <= input && input <= byte_2_end) {
      bytes = 2;
      str[1] = (uint8_t) (((input >> 0) & 0b00111111) | 0b10000000);
      str[0] = (uint8_t) (((input >> 6) & 0b00011111) | 0b11000000);
      goto end;
   }
   if (byte_3_start <= input && input <= byte_3_end) {
      bytes = 3;
      str[2] = (uint8_t) (((input >> 0)  & 0b00111111) | 0b10000000);
      str[1] = (uint8_t) (((input >> 6)  & 0b00111111) | 0b10000000);
      str[0] = (uint8_t) (((input >> 12) & 0b00001111) | 0b11100000);
      goto end;
   }
   if (byte_4_start <= input && input <= byte_4_end) {
      bytes = 4;
      str[3] = (uint8_t) (((input >> 0)  & 0b00111111) | 0b10000000);
      str[2] = (uint8_t) (((input >> 6)  & 0b00111111) | 0b10000000);
      str[1] = (uint8_t) (((input >> 12) & 0b00111111) | 0b10000000);
      str[0] = (uint8_t) (((input >> 18) & 0b00000111) | 0b11110000);
      goto end;
   }

   //If error ocurred, the input was invalid,
   //so don't write anything.

   end:
   return bytes;
}

char* readWonderCardText(uint16_t* bin, uint16_t* index, const uint32_t max_chars) {
   if (!bin || !index) return NULL;

   unsigned int i = *index;
   //UTF8 max character size is 4 bytes
   char* buffer = (char*) calloc((4 * max_chars) + 1, sizeof(char));
   uint8_t* ubuffer = (uint8_t*)buffer;
   unsigned int buffer_index = 0;
   uint16_t code = 0;
   bool null_found = false;

   for (uint32_t j = 0; j < max_chars; j++) {
      code = bin[i++];

      //Map gen 4 text to UTF16
      uint32_t input = 0;

      if (code == 0xffff || null_found) { //Gen 4 null terminator
         //We continue the loop to drain the text buffer.
         null_found = true;
         continue;
      }

      else if (0x0121 <= code && code <= (0x0121 + '9' - '0')) { //numeric
         input = '0' + (code - 0x0121);
      }
      else if (0x012b <= code && code <= (0x012b + 'Z' - 'A')) { //uppercase
         input = 'A' + (code - 0x012b);
      }
      else if (0x0145 <= code && code <= (0x0145 + 'z' - 'a')) { //lowercase
         input = 'a' + (code - 0x0145);
      }
      else if (0x0002 <= code && code <= (0x0002 + 0x3093 - 0x3041)) { //hiragana
         input = 0x3041 + (code - 0x0002);
      }
      else if (0x0052 <= code && code <= (0x0052 + 0x30f3 - 0x30a1)) { //katakana
         input = 0x30a1 + (code - 0x0052);
      }
      else if (0x015f <= code && code <= (0x015f + 0x00ff - 0x00c0)) { //accented latin range
         input = 0x00c0 + (code - 0x015f);
      }
      else if (0x0113 <= code && code <= (0x0113 + 0x2452 - 0x244b)) { //pokemon-specific emoticons
         input = 0x244b + (code - 0x0113);
      }
      else if (0x01d3 <= code && code <= (0x01d3 + 0x2603 - 0x2600)) { //weather emoticons
         input = 0x2600 + (code - 0x01d3);
      }

      else { //special characters
         switch (code) {
            // ASCII characters
            case 0x01de: input = ' '; break;
            case 0x01ab: input = '!'; break;
            case 0x01b4: input = '"'; break; // Reverts to standard ASCII quote
            case 0x01c0: input = '#'; break;
            case 0x01d2: input = '%'; break;
            case 0x01c2: input = '&'; break; // Unused in game (?)
            case 0x01b3: input = '\''; break; // Reverts to standard ASCII apostrophe
            case 0x01b9: input = '('; break;
            case 0x01ba: input = ')'; break;
            case 0x01bf: input = '*'; break;
            case 0x01bd: input = '+'; break;
            case 0x01ad: input = ','; break;
            case 0x01be: input = '-'; break;
            case 0x01ae: input = '.'; break;
            case 0x01b1: input = '/'; break;            
            case 0x01c4: input = ':'; break;
            case 0x01c5: input = ';'; break;
            case 0x01c1: input = '='; break;
            case 0x01d0: input = '@'; break;
            case 0x01e8: input = '_'; break; // Unused in game (?)
            case 0x01c3: input = '~'; break;
            
            // input characters
            case 0x01a9: input = 0x00a1; break; // ¡ symbol
            case 0x01a8: input = 0x00a4; break; // pokedollar symbol
            case 0x01b7: input = 0x00ab; break; // guillemet left symbol
            case 0x01b0: input = 0x00b7; break; // middle dot symbol
            case 0x01a4: input = 0x00ba; break; // masculine ordinal indicator symbol
            case 0x01b8: input = 0x00bb; break; // guillemet right symbol
            case 0x01aa: input = 0x00bf; break; // ¿ symbol
            case 0x01b6: input = 0x201e; break; // „ symbol
            case 0x01af: input = 0x2026; break; // triple dot symbol
            
            // Arrows and Special Icons
            case 0x011b: input = 0x2190; break; // left arrow
            case 0x011c: input = 0x2191; break; // up arrow
            case 0x011d: input = 0x2193; break; // down arrow
            case 0x011e: input = 0x2192; break; // right arrow
            case 0x01dd: input = 0x23fe; break; // zz sleep symbol
            case 0x011f: input = 0x25ba; break; // big triangle symbol
            
            case 0x01c6: input = 0x2660; break; // spade symbol
            case 0x01c7: input = 0x2663; break; // club symbol
            case 0x01c8: input = 0x2665; break; // heart symbol
            case 0x01c9: input = 0x2666; break; // diamond symbol
            case 0x01ca: input = 0x2605; break; // star symbol
            
            case 0x01d7: input = 0x263a; break; // happy emoticon
            case 0x01d8: input = 0x263b; break; // thrilled emoticon
            case 0x01d9: input = 0x2638; break; // angry emoticon
            case 0x01da: input = 0x2639; break; // upset emoticon
            
            case 0x01bb: input = 0x2642; break; // male symbol
            case 0x01bc: input = 0x2640; break; // female symbol
            case 0x01d1: input = 0x266a; break; // music note symbol
            case 0x01db: input = 0x2934; break; // curly up arrow
            case 0x01dc: input = 0x2935; break; // curly down arrow
            
            case 0x00e8: input = 0x300c; break; // japanese border left symbol
            case 0x00e9: input = 0x300d; break; // japanese border right symbol
            case 0x00ea: input = 0x300e; break; // japanese bold border left symbol
            case 0x00eb: input = 0x300f; break; // japanese bold border right symbol

            case 0xe000: input = '\n'; break;   // undocumented on bulbapedia

            // Default fallback for error or unknown character
            case 0x01ac:
            default:     input = '?'; break;
         }
      }

      //We have an input, so we write it to our buffer.
      buffer_index += writeUnicodeChar(&ubuffer[buffer_index], input);
   }

   //update index and buffer
   *index = i;
   
   char* buffer_small = (char*) realloc(buffer, (buffer_index + 1) * sizeof(char));
   if (!buffer_small) buffer_small = buffer; //fallback
   return buffer_small;
}

WonderCard readPCD(uint16_t* bin) {
   const uint16_t size = 856;
   //Generate an initial pgt
   WonderCard card = readPGT(bin);
   if (!bin) return card;

   uint16_t index = card.index;

   //Text box values
   const uint32_t 
   title_limit = 36,
   desc_limit  = 250;

   //Title
   card.title = readWonderCardText(bin, &index, title_limit);

   //Game flags (reversed)
   card.game_flags = SwapEndian(bin[index]); index++;

   index++; //0x0000

   //Wonder card id
   card.wc_id = bin[index]; index++;

   //Mystery byte, ignore.
   index++;

   //Description
   card.description = readWonderCardText(bin, &index, desc_limit);

   //Distribution count
   card.distrib_count = bin[index++];

   //Pokemon icons
   card.icon_left = bin[index++];
   card.icon_middle = bin[index++];
   card.icon_right = bin[index++];

   index += 2; //0x0000 * 2

   //Date
   //We need to convert this number into a struct tm.
   //Generally handled by inputting the number of seconds since the DS epoch.
   //Unix epoch is 1970-01-01, so 10957 days since is the DS epoch
   
   const uint64_t 
   epoch_seconds = (uint64_t)(10957 * 24 * 60 * 60),
   bin_date = bin[index++],
   date_seconds = epoch_seconds + (bin_date * 24 * 60 * 60);

   //localtime is dependant on timezone. 
   //to prevent this, use gmtime_r (linux and mac only)
   struct tm date = {0};
   gmtime_r((time_t*)(&date_seconds), &date);

   card.distrib_date = (char*) calloc((strlen("YYYY-MM-DD") * 2) + 1, sizeof(char));
   sprintf(card.distrib_date, "%i-%02i-%02i", date.tm_year + 1900, date.tm_mon + 1, date.tm_mday);
   
   index++; //0x0000

   //done


   card.index = index;
   return card;
}

WonderCard readMYG(uint16_t* bin) {
   const uint16_t size = 936;

   //an MYG file does not add any new information to the card.
   //So we read from offset 0x50
   //uint16_t so halve it.
   
   return readPCD(&bin[0x50 >> 1]);
}


WonderCard WonderCardInit(char* file) {
   WonderCard card;
   memset(&card, 0, sizeof(WonderCard));
   if (!file) return card;
   
   //Get the file length, see if valid.
   const uint32_t
   pgt_size = 260,
   pcd_size = 856,
   myg_size = 936;

   char* file_url = urlDecode(file);
   while (strchr(file_url, '%') != NULL) {
      char* file_url_deeper = urlDecode(file_url);
      free(file_url);
      file_url = file_url_deeper;
   }


   size_t file_size = base64_decode_size(file_url);
   fprintf(stderr, "file size: %lu\n", file_size);
   uint8_t* file_contents = base64_decode_binary(file_url);
   //I prefer using uint16 here
   uint16_t* file_contents_16 = (uint16_t*)file_contents;
   if (file_size == pgt_size) {
      card = readPGT(file_contents_16);
   }
   else if (file_size == pcd_size) {
      card = readPCD(file_contents_16);
   }
   else if (file_size == myg_size) {
      card = readMYG(file_contents_16);
   }

   if (file_contents) free(file_contents);
   if (file_url) free(file_url);
   return card;
}

void WonderCardFree(WonderCard card) {
   if (card.ek4) free(card.ek4);
   if (card.title) free(card.title);
   if (card.description) free(card.description);
   if (card.distrib_date) free(card.distrib_date);
}

void WonderCardDisplay(WonderCard card) {
   fprintf(stderr,
   "CARD DETAILS:\n"
   "TYPE: %i\n"
   "OT FLAG: %i\n"
   "EK4: %s\n"
   "ITEM ID: %u\n"
   "TITLE: %s\n"
   "DESCRIPTION: %s\n"
   "ICON LEFT: %u\n"
   "ICON MIDDLE: %u\n"
   "ICON RIGHT: %u\n"
   "DISTRIB COUNT: %u\n"
   "WC ID: %u\n"
   "DISTRIB DATE: %s\n"
   "GAME FLAGS: 0x%x\n",
   card.type, card.pokemon_OT_flag, card.ek4, card.item_id, card.title, card.description, card.icon_left, card.icon_middle, card.icon_right, card.distrib_count, card.wc_id, card.distrib_date, card.game_flags);
}

bool prepareWonderCardEdit(int client, char* file, siteVar** global) {
   WonderCard card = {0};
   card = WonderCardInit(file);
   WonderCardDisplay(card);
   //We now need to send over a new HTML file.
   //Insert our read data.
   switch (card.type) {
      case POKEMON: {
         siteVarCompositeInsertNew(global, "edit_type_pokemon", STRING, 1, (char*[]){"checked"});
         break;
      }
      case EGG: {
         siteVarCompositeInsertNew(global, "edit_type_egg", STRING, 1, (char*[]){"checked"});
         break;
      }
      case ITEM: {
         siteVarCompositeInsertNew(global, "edit_type_item", STRING, 1, (char*[]){"checked"});
         break;
      }
      default: {
         return false;
      }
   }

   if (card.type == ITEM) {
      siteVarCompositeInsertNew(global, "edit_item_id", UINT, 1, (uint_cot[]){card.item_id});
      siteVarCompositeInsertNew(global, "edit_item_sprite", STRING, 1, (string_cot[]){(string_cot)items2[card.item_id]});
      siteVarCompositeInsertNew(global, "edit_item_name", STRING, 1, (string_cot[]){(string_cot)items[card.item_id]});
   }
   else {
      if (card.pokemon_OT_flag) siteVarCompositeInsertNew(global, "edit_OT_flag", STRING, 1, (char*[]){"checked"});
      
      if (card.ek4) siteVarCompositeInsertNew(global, "edit_ek4_status", STRING, 1, (string_cot[]){"<div style=\"color: green;\">Data is valid!</div>"});
      else siteVarCompositeInsertNew(global, "edit_ek4_status", STRING, 1, (string_cot[]){"<div style=\"color: red;\">Data is not valid</div>"});
      
      //we need to base64 encode our payload.
      char* ek4_encoded = base64_encode_binary(card.ek4, 236);
      if (ek4_encoded) {
         siteVarCompositeInsertNew(global, "edit_ek4_base64", STRING, 1, (string_cot[]){ek4_encoded});
         free(ek4_encoded);
      }
      else return false;
   }

   if (card.title)       siteVarCompositeInsertNew(global, "edit_title", STRING, 1, (string_cot[]){card.title}      );
   if (card.description) siteVarCompositeInsertNew(global, "edit_desc" , STRING, 1, (string_cot[]){card.description});

   //Icons
   if (card.icon_left) {
      char sprite_buffer[256] = {0};
      sprintf(sprite_buffer, "pokemon %s", pokemons[card.icon_left]);

      siteVarCompositeInsertNew(global, "edit_icon_left_id"    , UINT  , 1, (uint_cot[])  {card.icon_left}                      );
      siteVarCompositeInsertNew(global, "edit_icon_left_sprite", STRING, 1, (string_cot[]){sprite_buffer}                       );
      siteVarCompositeInsertNew(global, "edit_icon_left_name"  , STRING, 1, (string_cot[]){(string_cot)pokemons[card.icon_left]});
   }
   if (card.icon_middle) {
      char sprite_buffer[256] = {0};
      sprintf(sprite_buffer, "pokemon %s", pokemons[card.icon_middle]);

      siteVarCompositeInsertNew(global, "edit_icon_middle_id"    , UINT  , 1, (uint_cot[])  {card.icon_middle}                      );
      siteVarCompositeInsertNew(global, "edit_icon_middle_sprite", STRING, 1, (string_cot[]){sprite_buffer}                       );
      siteVarCompositeInsertNew(global, "edit_icon_middle_name"  , STRING, 1, (string_cot[]){(string_cot)pokemons[card.icon_middle]});
   }
   if (card.icon_right) {
      char sprite_buffer[256] = {0};
      sprintf(sprite_buffer, "pokemon %s", pokemons[card.icon_right]);

      siteVarCompositeInsertNew(global, "edit_icon_right_id"    , UINT  , 1, (uint_cot[])  {card.icon_right}                      );
      siteVarCompositeInsertNew(global, "edit_icon_right_sprite", STRING, 1, (string_cot[]){sprite_buffer}                       );
      siteVarCompositeInsertNew(global, "edit_icon_right_name"  , STRING, 1, (string_cot[]){(string_cot)pokemons[card.icon_right]});
   }

   siteVarCompositeInsertNew(global, "edit_distrib_count", UINT, 1, (uint_cot[]){card.distrib_count});
   if (card.distrib_count >= 255)
      siteVarCompositeInsertNew(global, "edit_distrib_infinite", STRING, 1, (string_cot[]){"checked"});

   siteVarCompositeInsertNew(global, "edit_wc_id", UINT, 1, (uint_cot[]){card.wc_id});
   
   if (card.distrib_date) 
      siteVarCompositeInsertNew(global, "edit_date", STRING, 1, (string_cot[]){card.distrib_date});

   const uint16_t
	DIAMOND = 1 << 2,
	PEARL = 1 << 3,
	PLATINUM = 1 << 4,
	HG = 1 << 15,
	SS = 1 << 0;
   
   if (card.game_flags & DIAMOND)
      siteVarCompositeInsertNew(global, "edit_flag_d", STRING, 1, (string_cot[]){"checked"});
   if (card.game_flags & PEARL)
      siteVarCompositeInsertNew(global, "edit_flag_p", STRING, 1, (string_cot[]){"checked"});
   if (card.game_flags & PLATINUM)
      siteVarCompositeInsertNew(global, "edit_flag_pt", STRING, 1, (string_cot[]){"checked"});
   if (card.game_flags & HG)
      siteVarCompositeInsertNew(global, "edit_flag_hg", STRING, 1, (string_cot[]){"checked"});
   if (card.game_flags & SS)
      siteVarCompositeInsertNew(global, "edit_flag_ss", STRING, 1, (string_cot[]){"checked"});

   //global is now complete, we just need to send it now.
   WonderCardFree(card);
   return true;
}

NewRouteFunction(homeGet) {
   return defaultGet(request, clientfd, extraData, "assets/web/templates/home.html");
}
NewRouteFunction(homePost) {
   fprintf(stderr, "POST REQUEST CONTENTS:\n%s\nWOOOO\n", request.payload);
   stringMap* payload = payloadToMap(request.payload, true);
   if (!payload || !request.payload) return false;

   char* action = strMapGet(payload, "action");
   bool status = false;
   
   if (!action) status = false;
   else if (strcmp(action, "switch_info") == 0) {
      status = sendRedirect("/info", clientfd);
   }
   else if (strcmp(action, "switch_new") == 0) {
      status = sendRedirect("/create", clientfd);
   }
   else if (strcmp(action, "switch_edit") == 0) {
      //We have a file payload now.
      char* file = strMapGet(payload, "wc_data_raw");

      siteVar* global = siteVarClone(extraData);
      fprintf(stderr, "GLOBAL NAME IS %s\n\n", global->name);

      char* data = NULL;
      siteVar* variables = NULL;
      HttpResponse response = {0};
      if (HttpResponseInit(&response, HTTP_1_1, HttpStatus_OK).status == COT_ERROR) {
         // siteVarFree(global);
         status = sendRedirect("/", clientfd);
         siteVarFree(global);
         goto end;
      }

      bool wc_status = prepareWonderCardEdit(clientfd, file, &global);
      if (!wc_status) {
         //Something bad happened, so return an alert.
         data = (char*) calloc(256, sizeof(char));
         //snprintf(data, 200, "<script>alert('Wonder Card data could not be processed');</script>");
         snprintf(data, 256, "<script>alert('Wonder Card data could not be processed'); window.location.href='/';</script>");
         goto switch_edit_end;
      }

      //get the openHTML data.
      //check if we have important data
      // siteVar* special_chars = siteVarCompositeAccessReference(global, "special_chars");
      // if (special_chars) {
      //    fprintf(stderr, "special chars is valid, %s\n\n", special_chars->name);
      // }
      
      variables = siteVarInit("variables", COMPOSITE, 0, NULL);
      siteVarCompositeInsert(&variables, global);
      
      if (openHTML(&data, "assets/web/templates/create.html", variables).status == COT_ERROR) {
         HttpResponseFree(response);
         goto end;
      }

      switch_edit_end:
      HttpResponseAddPayload(&response, data, strlen(data));
      
      HttpResponseAddOption(&response, "Content-Type", "text/html; charset=UTF-8");
      HttpResponseAddOption(&response, "Connection", "close");
      
      if (wc_status) HttpResponseAddOption(&response, "HX-Push-Url", "/create");

      // status = sendRedirect("/create", clientfd);
      status = sendCustom(response, clientfd);

      //if (!wc_status) sendRedirect("/", clientfd);

      HttpResponseFree(response);
      if (data) free(data);
      if (status) fprintf(stderr, "sendCustom worked\n");
      siteVarFree(variables);
      siteVarFree(global);
      fprintf(stderr, "homePost variables freed\n");
      //return status;   
   }
   

   end:
   if (payload) strMapFree(payload);
   payload = NULL;
   fprintf(stderr, "payload freed\n");
   return status;
}
NewRouteFunction(homePut) {
   return false;
}
NewRouteFunction(homeDelete) {
   return false;
}

/*
Enter the following into your main code, under where you setup your routes:
RouteEntry home = {
    .routeGet = homeGet,
    .routePost = homePost,
    .routePut = homePut,
    .routeDelete = homeDelete
};


newRoute(YOUR SITE PATH HERE, home);
*/

#include "../cottage/cottage.h"
#include "../base64/base64.h"

uint16_t* generatePGT(stringMap* src) {
   if (!src) return NULL;
   
   //A two-byte buffer works best.
   uint16_t buffer[1000] = {0};
   unsigned int index = 0;

   //Gift type
   char* type = strMapGet(src, "type");
   if (!type) return NULL; //required field.
   
   bool isItem = false;
   if (strcmp(type, "pokemon") == 0) {
      buffer[index++] = 0x0100;
   }
   else if (strcmp(type, "egg") == 0) {
      buffer[index++] = 0x0200;
   }
   else if (strcmp(type, "item") == 0) {
      buffer[index++] = 0x0300;
      isItem = true;
   }
   else return NULL; //Must be one of these 3

   index++; //0x0000

   //Gift specific data.
   if (isItem) { //Gets the item ID.
      char* item_id = strMapGet(src, "item_id");
      if (item_id) {
         //its a number so convert.
         //may turn out wrong due to endianess.
         uint16_t item_id_num = atoi(item_id);
         buffer[index++] = item_id_num;
      }
      else index++; //0x0000
   }
   else { //OT check
      char* OC_check = strMapGet(src, "OC_check");
      if (OC_check && strcmp(OC_check, "true") == 0) {
         //If set, means yes, so remains at 0x0000
         index++;
      }
      else buffer[index++] = 0x0100;
   }

   index++; //0x0000

   //Pokemon data.
   if (!isItem) {
      //An encrypted pokemon ek4 file is 236 bytes.
      char* pokemon_data_raw = strMapGet(src, "pokemon_data_raw");
      if (!pokemon_data_raw) return NULL; //required

      //base64 and url encoded right now.
      char* url_decoded = urlDecode(pokemon_data_raw);
      if (!url_decoded) return NULL;
      fprintf(stderr, "generatePGT: url_decoded is \n\n%s\n\n", url_decoded);
      
      //with this url_decoded functionality, we can see the size of the file.
      size_t bin_size = base64_decode_size(url_decoded);
      if (bin_size != 236) {
         free(url_decoded);
         return NULL;
      }

      //Size is good, so decode
      char* bin = base64_decode(url_decoded);
      if (!bin) {
         free(url_decoded);
         return NULL;
      }

      //with this, we can copy
      for (int j = 0; j < 236;) {
         uint16_t bin_high = (uint16_t)bin[j++];
         uint16_t bin_low = (uint16_t)bin[j++];
         uint16_t bin_short = (bin_high << 8) | (bin_low);

         buffer[index++] = bin_short;
      }

      free(url_decoded);
      free(bin);

   }

   //special IVgift message
   const char* thank_you = "THX 4 IVGIFT! <3";
   for (int j = 0; j < strlen(thank_you);) {
         uint16_t high = (uint16_t)thank_you[j++];
         uint16_t low = (uint16_t)thank_you[j++];
         uint16_t thx = (high << 8) | (low);

         buffer[index++] = thx;
   }

   buffer[index] = 0; //null terminate
   //with that, we are good to go.
   uint16_t* return_buffer = (uint16_t*) calloc(index + 1, sizeof(uint16_t));
   if (!return_buffer) return NULL; //allocation error

   memcpy(return_buffer, buffer, (index + 1) * sizeof(uint16_t));

   return return_buffer;
}

/*
https://www.martinpickering.com/posts/unicode-in-c/
https://en.wikipedia.org/wiki/UTF-8
*/
uint32_t extractUnicodeChar(uint8_t* str, uint32_t* output) {
   if (!str || !output) return 0;
   
   uint32_t bytes = 0; //How many characters does this unicode take up?
   uint32_t code = 0; //Stores our current code

   /*
   Imagine our code in the format
   (uuuu vvvv) (wwww xxxx) (yyyy zzzz)
   each one of these groups is a byte
   */

   if ((str[0] & 0b1000000) == 0) { //ASCII
      bytes = 1;
      code = str[0];
      goto end;
   }
   if ((str[0] & 0b1110000) == 0b11000000) {
      bytes = 2;
      code = 
      (str[1] & 0b00111111) |
      ((str[0] & 0b00011111) << 6);
      goto end;
   }
   if ((str[0] & 0b1111000) == 0b11100000) {
      bytes = 3;
      code = 
      (str[2] & 0b00111111) |
      ((str[1] & 0b00111111) << 6) |
      ((str[0] & 0b00001111) << 12);
      goto end;
   }
   if ((str[0] & 0b1111100) == 0b11110000) {
      bytes = 4;
      code = 
      (str[3] & 0b00111111) |
      ((str[2] & 0b00111111) << 6) |
      ((str[1] & 0b00111111) << 12) |
      ((str[0] & 0b00000111) << 18);
      goto end;
   }

   //Error ocurred
   bytes = 1;
   code = '?';

   end:
   *output = code;
   return bytes;
}

void writeWonderCardText(uint16_t* buffer, unsigned int* index, const uint32_t max_characters, const char* msg) {
   if (!buffer || !index) return;

   if (!msg) {
      //an empty message can mean "fill with padding"
      memset(&buffer[*index], 0xff, max_characters * sizeof(uint16_t));
      *index += max_characters;
      return;
   }

   unsigned int i = *index;
   uint8_t* umsg = (uint8_t*) msg;
   uint32_t unicode = 0;

   for (uint32_t j = 0; umsg != NULL || j < max_characters; j++) {
      if (umsg == NULL) { //null terminator padding.
         buffer[i++] = 0xffff;
         continue;
      }
      //Grab the next unicode character.
      umsg += extractUnicodeChar(umsg, &unicode);

      uint16_t input = 0;
      if ('0' <= unicode && unicode <= '9') { //numeric
         input = 0x0121 + (unicode - '0');
      }
      else if ('A' <= unicode && unicode <= 'Z') { //uppercase 
         input = 0x012b + (unicode - 'A');
      }
      else if ('a' <= unicode && unicode <= 'z') { //lowercase 
         input = 0x0145 + (unicode - 'a');
      }
      else if (0x3041 <= unicode && unicode <= 0x3093) { //hiragana
         input = 0x0002 + (unicode - 0x3041);
      }
      else if (0x30a1 <= unicode && unicode <= 0x30f3) { //katakana
         input = 0x0053 + (unicode - 0x30a1);
      }
      else { //special characters (TO FINISH)
         switch (unicode) {
            //ASCII characters
            case ' ' : input = 0x01de; break;
            case '!' : input = 0x01ab; break;
            case '"' : input = 0x01b4; break; //There are multiple symbols in gen4, so not authentic.
            case '#' : input = 0x01c0; break;
            //No '$' symbol
            case '%' : input = 0x01d2; break;
            case '&' : input = 0x01c2; break; //Unused in game (?)
            case '\'': input = 0x01b3; break; //There are multiple symbols in gen4, so not authentic.
            case '(' : input = 0x01b9; break;
            case ')' : input = 0x01ba; break;
            case '*' : input = 0x01bf; break;
            case '+' : input = 0x01bd; break;
            case ',' : input = 0x01ad; break;
            case '-' : input = 0x01be; break;
            case '.' : input = 0x01ae; break;
            case '/' : input = 0x01b1; break;            
            case ':' : input = 0x01c4; break;
            case ';' : input = 0x01c5; break;
            //No '<' symbol
            case '=' : input = 0x01c1; break;
            //No '>' symbol
            case '@' : input = 0x01d0; break;
            //No '[' symbol
            //No '\' symbol
            //No ']' symbol
            //No '^' symbol
            case '_' : input = 0x01e8; break; //Unused in game (?)
            //No '`' symbol, may be 0x01b2 but unsure
            
            //Unicode punctuation

            case '~' : input = 0x0000; break;
            case ' ' : input = 0x0000; break; // ¡ symbol
            case ' ' : input = 0x0000; break; // ¢ symbol
            case ' ' : input = 0x0000; break; // £ symbol
            case ';' : input = 0x0000; break; // pokedollar symbol
            case ';' : input = 0x0000; break; // yen symbol
            case ';' : input = 0x0000; break; // copyright symbol
            case ';' : input = 0x0000; break; //…
            case ';' : input = 0x0000; break;
            case ';' : input = 0x0000; break;
            case ';' : input = 0x0000; break;
            case ';' : input = 0x0000; break;
            case ';' : input = 0x0000; break;
            case ';' : input = 0x0000; break;
            

            case '\n': input = 0xe000; break; //undocumented on bulbapedia
            case ''  :  break;
            case ''  :  break;
            case ''  :  break;

            //default error as '?'
            case '?' :
            default  : input = 0x01ac; break;
         }
      }

      //write out the input
      buffer[i++] = input;
   }
   
   //update index
   *index = i;
}

uint16_t* generatePCD(stringMap* src, uint16_t* pgt) {
   if (!src) return NULL;

   uint16_t buffer[1000] = {0};
   unsigned int index = 0;
   
   //Read in the pgt
   bool pgt_available = !!(pgt);
   if (!pgt_available) {
      pgt = generatePGT(src);
   }
   //A PGT file is 260 bytes.
   index = 260 >> 1;
   memcpy(buffer, pgt, index << 1);
   if (!pgt_available && pgt) free(pgt);

   //Text box values
   const uint32_t 
   title_limit = 36,
   desc_limit  = 250;

   //Title
   char* title_input = strMapGet(src, "title_input"); 
   writeWonderCardText(buffer, &index, title_limit, title_input);

   //Game flags
   char* flag_d = strMapGet(src, "flag_d");
   char* flag_p = strMapGet(src, "flag_p");
   char* flag_pt = strMapGet(src, "flag_pt"); 
   char* flag_hg = strMapGet(src, "flag_hg");
   char* flag_ss = strMapGet(src, "flag_ss");

   const uint16_t
	DIAMOND = 1 << 2,
	PEARL = 1 << 3,
	PLATINUM = 1 << 4,
	HG = 1 << 15,
	SS = 1 << 0;

   if (flag_d)  buffer[index] |= DIAMOND;
   if (flag_p)  buffer[index] |= PEARL;
   if (flag_pt) buffer[index] |= PLATINUM;
   if (flag_hg) buffer[index] |= HG;
   if (flag_ss) buffer[index] |= SS;
   index++;

   index ++; //0x0000

   //Wonder card id
   uint16_t wc_id_num = 0;
   char* wc_id = strMapGet(src, "wc_id");
   if (wc_id) wc_id_num = atoi(wc_id);
   buffer[index++] = wc_id_num;

   //Mystery byte, can probably be whatever you want.
   buffer[index++] = 0x0d00;

   //Description text
   char* desc_input = strMapGet(src, "desc_input");
   writeWonderCardText(buffer, &index, desc_limit, desc_input);

   //Distribution count
   uint16_t distrib_count_num = 0;
   char* distrib_count = strMapGet(src, "distrib_count");
   if (distrib_count) distrib_count_num = atoi(distrib_count);
   buffer[index++] = distrib_count_num;

   //Pokemon icons
   uint16_t icon_left = 0;
   char* pokemon_container_left = strMapGet(src, "pokemon_container_left");
   if (pokemon_container_left) icon_left = atoi(pokemon_container_left);
   buffer[index++] = icon_left;

   uint16_t icon_middle = 0;
   char* pokemon_container_middle = strMapGet(src, "pokemon_container_middle");
   if (pokemon_container_middle) icon_middle = atoi(pokemon_container_middle);
   buffer[index++] = icon_middle;

   uint16_t icon_right = 0;
   char* pokemon_container_right = strMapGet(src, "pokemon_container_right");
   if (pokemon_container_right) icon_right = atoi(pokemon_container_right);
   buffer[index++] = icon_right;

   index += 2; //0x0000 * 2

   //Date


   return NULL;
}

uint16_t* generateMYG(stringMap* src, uint16_t* pcd) {
   return NULL;
}

NewRouteFunction(generate_apiGet) {
   return false;
}
NewRouteFunction(generate_apiPost) {
   /*
   This will receive the WC data.
   Probably safe to store in a stringMap.
   */
   stringMap* payload = payloadToMap(request.payload, true);
   if (!payload || !request.payload) return false;

   //DEBUG print out payload.
   fprintf(stderr, "GENERATE PAYLOAD:\n\n");
   for (int i = 0; i < payload->capacity; i++) {
      if (payload->items[i]) {
         fprintf(stderr, "%s: %s\n", payload->items[i]->key, payload->items[i]->value);
      }
   }
   fprintf(stderr, "\n");

   //With our payload, we can now build our files.
   uint16_t* pgt = generatePGT(payload);
   if (pgt) fprintf(stderr, "PGT SUCCESS\n\n");


   return sendRedirect("/", clientfd);
   //return false;
}
NewRouteFunction(generate_apiPut) {
   return false;
}
NewRouteFunction(generate_apiDelete) {
   return false;
}

/*
Enter the following into your main code, under where you setup your routes:
RouteEntry generate_api = {
    .routeGet = generate_apiGet,
    .routePost = generate_apiPost,
    .routePut = generate_apiPut,
    .routeDelete = generate_apiDelete
};


newRoute(YOUR SITE PATH HERE, generate_api);
*/

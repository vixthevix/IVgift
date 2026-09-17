#include "../cottage/cottage.h"
#include "../base64/base64.h"
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include <zip.h>

#define SwapEndian(x) (x >> 8) | (x << 8)

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
      buffer[index++] = 0x0001;
   }
   else if (strcmp(type, "egg") == 0) {
      buffer[index++] = 0x0002;
   }
   else if (strcmp(type, "item") == 0) {
      buffer[index++] = 0x0003;
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
      else buffer[index++] = 0x0001;
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
         uint16_t bin_low = (uint16_t)bin[j++];
         uint16_t bin_high = (uint16_t)bin[j++];
         uint16_t bin_short = (bin_high << 8) | (bin_low & 0x00ff);

         buffer[index++] = bin_short;
      }

      free(url_decoded);
      free(bin);

   }

   //special IVgift message
   const char* thank_you = "THX 4 IVGIFT! <3";
   for (int j = 0; j < strlen(thank_you);) {
         uint16_t low = (uint16_t)thank_you[j++];
         uint16_t high = (uint16_t)thank_you[j++];
         uint16_t thx = (high << 8) | (low & 0x00ff);

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

   if ((str[0] & 0b10000000) == 0) { //ASCII
      bytes = 1;
      code = (uint32_t) (str[0]);
      goto end;
   }
   if ((str[0] & 0b11100000) == 0b11000000) {
      bytes = 2;
      code = 
      ( (uint32_t) (str[1] & 0b00111111)) |
      ( (uint32_t) (str[0] & 0b00011111) << 6);
      goto end;
   }
   if ((str[0] & 0b11110000) == 0b11100000) {
      bytes = 3;
      code = 
      ( (uint32_t) (str[2] & 0b00111111)) |
      ( (uint32_t) (str[1] & 0b00111111) << 6) |
      ( (uint32_t) (str[0] & 0b00001111) << 12);
      goto end;
   }
   if ((str[0] & 0b11111000) == 0b11110000) {
      bytes = 4;
      code = 
      ( (uint32_t) (str[3] & 0b00111111)) |
      ( (uint32_t) (str[2] & 0b00111111) << 6) |
      ( (uint32_t) (str[1] & 0b00111111) << 12) |
      ( (uint32_t) (str[0] & 0b00000111) << 18);
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

   if (!msg || strlen(msg) == 0) {
      //an empty message can mean "fill with padding"
      memset(&buffer[*index], 0xff, max_characters * sizeof(uint16_t));
      *index += max_characters;
      return;
   }

   unsigned int i = *index;
   uint8_t* umsg = (uint8_t*) msg;
   uint32_t unicode = 0;

   newResultError("writeWonderCardText: setup done.");
   setlocale(LC_ALL, "en_US.UTF-8");

   for (uint32_t j = 0; *umsg != 0 || j < max_characters; j++) {
      if (*umsg == 0) { //end of umsg, so null terminator padding.
         fprintf(stderr, "writeWonderCardText: umsg null, j: %u\n", j);
         buffer[i++] = 0xffff;
         continue;
      }
      //Grab the next unicode character.
      uint32_t bytes = extractUnicodeChar(umsg, &unicode); 
      umsg += bytes;
      fprintf(stderr, "Current unicode: %u, j: %u, bytes: %u\n", unicode, j, bytes);

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

            // case '~' : input = 0x0000; break;
            // case ' ' : input = 0x0000; break; // ¡ symbol
            // case ' ' : input = 0x0000; break; // ¢ symbol
            // case ' ' : input = 0x0000; break; // £ symbol
            // case ';' : input = 0x0000; break; // pokedollar symbol
            // case ';' : input = 0x0000; break; // yen symbol
            // case ';' : input = 0x0000; break; // copyright symbol
            // case ';' : input = 0x0000; break; //…
            // case ';' : input = 0x0000; break;
            // case ';' : input = 0x0000; break;
            // case ';' : input = 0x0000; break;
            // case ';' : input = 0x0000; break;
            // case ';' : input = 0x0000; break;
            // case ';' : input = 0x0000; break;
            

            case '\n': input = 0xe000; break; //undocumented on bulbapedia

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
      if (!pgt) return NULL;
   }
   //A PGT file is 260 bytes.
   index = 260 >> 1;
   memcpy(buffer, pgt, index << 1);
   if (!pgt_available && pgt) free(pgt);

   fprintf(stderr, "WROTE PGT FILE TO PCD\n\n");

   //Text box values
   const uint32_t 
   title_limit = 36,
   desc_limit  = 250;

   //Title - CURRENTLY BUGGED SEG FAULT
   char* title_input = strMapGet(src, "title_input");
   if (title_input) fprintf(stderr, "TITLE INPUT GOT: %s\n\n", title_input);
   writeWonderCardText(buffer, &index, title_limit, title_input);

   fprintf(stderr, "PCD TITLE\n\n");

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

   //G0000000 000PPD0S => 000PPD0S G0000000 

   if (flag_d)  buffer[index] |= DIAMOND;
   if (flag_p)  buffer[index] |= PEARL;
   if (flag_pt) buffer[index] |= PLATINUM;
   if (flag_hg) buffer[index] |= HG;
   if (flag_ss) buffer[index] |= SS;

   buffer[index++] = SwapEndian(buffer[index]);

   fprintf(stderr, "PCD FLAGS\n\n");

   index ++; //0x0000

   //Wonder card id
   uint16_t wc_id_num = 0;
   char* wc_id = strMapGet(src, "wc_id");
   if (wc_id) wc_id_num = atoi(wc_id);

   wc_id_num = SwapEndian(wc_id_num);

   buffer[index++] = wc_id_num;

   fprintf(stderr, "PCD WC ID\n\n");

   //Mystery byte, can probably be whatever you want.
   // buffer[index++] = 0x0d00;
   buffer[index++] = 0xffff;

   //Description text
   char* desc_input = strMapGet(src, "desc_input");
   writeWonderCardText(buffer, &index, desc_limit, desc_input);

   fprintf(stderr, "PCD DESC\n\n");

   //Distribution count
   //Check if unlimited
   uint16_t distrib_count_num = 0;
   char* distrib_count = strMapGet(src, "distrib_count");
   char* distrib_infinite = strMapGet(src, "distrib_infinite");
   if (distrib_infinite) distrib_count_num = 255;
   else if (distrib_count) distrib_count_num = atoi(distrib_count);
   buffer[index++] = distrib_count_num;

   fprintf(stderr, "PCD DISTRIB COUNT\n\n");

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

   fprintf(stderr, "PCD ICONS\n\n");

   index += 2; //0x0000 * 2

   //Date
   char* wc_date = strMapGet(src, "wc_date");
   char* current_date = strMapGet(src, "current_date");

   //Time constants
   const uint32_t
   system_year_diff = 1900,
   system_month_diff = 1,
   system_day_diff = 1;

   //Time storage
	struct tm epoch; //beginning of time for the date
	struct tm date; //out current date for the event.
	memset(&epoch, 0, sizeof(struct tm));
	memset(&date, 0, sizeof(struct tm));

   //Set the epoch (on the DS, 01/01/2000)
	epoch.tm_year = 2000 - system_year_diff;
	epoch.tm_mon  = 1    - system_month_diff;
	epoch.tm_mday = 2    - system_day_diff;
	//hour, minute, and second are 0
	epoch.tm_isdst = -1; //daylight savings, let system decide.

   //Check if we use the current date
   if (current_date) {
      time_t t = time(NULL);
      date = *localtime(&t);
   }
   else if (wc_date) {
      //In the format YEAR-MONTH-DAY
      uint32_t year = 0, month = 0, day = 0;
      char* date_dash = NULL;
      char date_buffer[100] = {0};
      uint32_t date_pointer = 0, date_stride = 0;

      //Year
      date_dash = strstr(wc_date, "-");
      if (date_dash) {
         date_stride = 4;
         memcpy(date_buffer, wc_date, date_stride);
         wc_date += date_stride + 1;
         year = atoi(date_buffer);
         memset(date_buffer, 0, 100);
      }
      //Month
      date_dash = strstr(wc_date, "-");
      if (date_dash) {
         date_stride = 2;
         memcpy(date_buffer, wc_date, date_stride);
         wc_date += date_stride + 1;
         month = atoi(date_buffer);
         memset(date_buffer, 0, 100);
      }
      //Day - drain the buffer
      strcpy(date_buffer, wc_date);
      day = atoi(date_buffer);
      memset(date_buffer, 0, 100);

      //we can now fill out our tm struct
      date.tm_year = year  - system_year_diff;
      date.tm_mon  = month - system_month_diff;
      date.tm_mday = day   - system_day_diff;
      date.tm_isdst = -1; //daylight savings, system choice
   }
   else return NULL; //probably invalid

   //date struct set up, so get the difference.
   time_t epoch_t = mktime(&epoch);
   time_t date_t = mktime(&date);
   double dt = difftime(date_t, epoch_t);
	//add half a day in seconds to round safely and to protect against DST,
	//and divide by seconds in a day
	uint16_t days = (uint16_t) round((dt + 43200) / 86400);

   buffer[index++] = days;

   fprintf(stderr, "PCD DAYS\n\n");

   index++; //0x0000

   buffer[index] = 0; //null terminate
   //with that, we are good to go.
   uint16_t* return_buffer = (uint16_t*) calloc(index + 1, sizeof(uint16_t));
   if (!return_buffer) return NULL; //allocation error

   memcpy(return_buffer, buffer, (index + 1) * sizeof(uint16_t));

   return return_buffer;
}

uint16_t* generateMYG(stringMap* src, uint16_t* pcd) {
   if (!src) return NULL;

   uint16_t buffer[1000] = {0};
   unsigned int index = 0;
   
   //Read in the pcd
   bool pcd_available = !!(pcd);
   if (!pcd_available) {
      pcd = generatePCD(src, NULL);
      if (!pcd) return NULL;
   }
   //A PCD file is 856 bytes.
   index = 856 >> 1;
   memcpy(buffer, pcd, index << 1);
   if (!pcd_available && pcd) free(pcd);
   
   //MYG just copies bytes 0x0104 through 0x0154 to the header.
	uint16_t myg_buffer[1500] = {0};
	unsigned int myg_index = 0;
	for (unsigned int j = (0x104 >> 1); myg_index < (0x50 >> 1); j++) {
		myg_buffer[myg_index++] = buffer[j];
	}
	for (unsigned int j = 0; j < index; j++) {
		myg_buffer[myg_index++] = buffer[j];
	}

   myg_buffer[myg_index] = 0; //null terminate
   //with that, we are good to go.
   uint16_t* return_buffer = (uint16_t*) calloc(myg_index + 1, sizeof(uint16_t));
   if (!return_buffer) return NULL; //allocation error

   memcpy(return_buffer, myg_buffer, (myg_index + 1) * sizeof(uint16_t));

   return return_buffer;
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
   const uint32_t
   pgt_size = 260,
   pcd_size = 856,
   myg_size = 936;

   uint16_t* pgt = generatePGT(payload);
   if (pgt) fprintf(stderr, "PGT SUCCESS\n\n");
   uint16_t* pcd = generatePCD(payload, pgt);
   if (pcd) fprintf(stderr, "PCD SUCCESS\n\n");
   uint16_t* myg = generateMYG(payload, pcd);
   if (myg) fprintf(stderr, "MYG SUCCESS\n\n");

   //With our buffers, we write them to a zip file.
   //We can do this with libzip (zip.h)

   //Set up error fixture
   zip_error_t zip_err;
   zip_error_init(&zip_err);

   int zip_status = 0;
   
   //Holds the actual data of our zip file we will create.

   //const uint16_t wc_zip_source_size = 2048;
   //uint8_t* wc_zip_source_full = (uint8_t*) calloc(wc_zip_source_size, 1);

   zip_source_t* wc_source = zip_source_buffer_create(NULL, 0, 0, &zip_err);
   if (!wc_source) {
      fprintf(stderr, "WC_SOURCE FAILED: %s\n", zip_error_strerror(&zip_err));
   }

   //By default, libzip will free data automatically for us at times.
   //To prevent this with our source, we keep it
   zip_source_keep(wc_source);

   //File interface for the zip.
   //TRUNCATE means "even if this already exists, handle it like its brand new and empty".
   zip_t* wc_zip = zip_open_from_source(wc_source, ZIP_TRUNCATE, &zip_err);
   if (!wc_zip) {
      fprintf(stderr, "WC_ZIP FAILED: %s\n", zip_error_strerror(&zip_err));
   }


   //Convert files into zip sources, and attach them to the main zip source.
   zip_source_t* pgt_source = zip_source_buffer(wc_zip, pgt, pgt_size, 0);
   if (!pgt_source) {
      fprintf(stderr, "PGT_SOURCE FAILED: %s\n", zip_strerror(wc_zip));
   }
   
   zip_source_t* pcd_source = zip_source_buffer(wc_zip, pcd, pcd_size, 0);
   if (!pcd_source) {
      fprintf(stderr, "PCD_SOURCE FAILED: %s\n", zip_strerror(wc_zip));
   }
   
   zip_source_t* myg_source = zip_source_buffer(wc_zip, myg, myg_size, 0);
   if (!myg_source) {
      fprintf(stderr, "MYG_SOURCE FAILED: %s\n", zip_strerror(wc_zip));
   }


   //Add our files to the file interface
   //Add HTML for writing a name for these files. otherwise, use 'IVgift'.
   
   zip_status = zip_file_add(wc_zip, "IVgift.pgt", pgt_source, ZIP_FL_OVERWRITE);
   if (zip_status < 0) {
      fprintf(stderr, "PGT_ADD FAILED: %s\n", zip_strerror(wc_zip));
   }
   
   zip_status = zip_file_add(wc_zip, "IVgift.pcd", pcd_source, ZIP_FL_OVERWRITE);
   if (zip_status < 0) {
      fprintf(stderr, "PCD_ADD FAILED: %s\n", zip_strerror(wc_zip));
   }

   zip_status = zip_file_add(wc_zip, "IVgift.myg", myg_source, ZIP_FL_OVERWRITE);
   if (zip_status < 0) {
      fprintf(stderr, "MYG_ADD FAILED: %s\n", zip_strerror(wc_zip));
   }

   //Done with the interface, so close it
   zip_status = zip_close(wc_zip);
   if (zip_status < 0) {
      fprintf(stderr, "ZIP_CLOSE FAILED: %s\n", zip_strerror(wc_zip));
   }

   //With our total source, we read it into a buffer and get the size.
   zip_stat_t wc_stats = {0};
   zip_status = zip_source_stat(wc_source, &wc_stats); //Stores stats about the zip, including size.
   if (zip_status < 0) {
      fprintf(stderr, "ZIP_SOURCE_STAT FAILED: %s\n", "idk");
   }
   
   size_t wc_zip_size = wc_stats.size;
   fprintf(stderr, "WC_ZIP_SIZE: %zu\n\n", wc_zip_size);

   //returns -1 on failure
   zip_status =  zip_source_open(wc_source);
   if (zip_status < 0) {
      fprintf(stderr, "ZIP_SOURCE_OPEN FAILED: %s\n", "idk");
   }

   if (zip_source_seek(wc_source, 0, SEEK_SET) < 0) {
      fprintf(stderr, "ZIP_SOURCE_SEEK FAILED\n");
   }
   
   // //returns -1 on error, and 0 on finish read

   uint8_t* wc_zip_data = (uint8_t*) malloc(wc_zip_size);

   int64_t zip_bytes = 0, zip_index = 0; 
   while (zip_index <= (int64_t)wc_zip_size) {
      zip_bytes = zip_source_read(wc_source, &wc_zip_data[zip_index], wc_zip_size - zip_index);
      if (zip_bytes < 0) {
         fprintf(stderr, "ZIP_SOURCE_READ FAILED: %s\n", "idk");
         break;
      }

      zip_index += zip_bytes;
      fprintf(stderr, "ZIP_SOURCE_READ INDEX: %li\n", zip_index);
      if (zip_bytes == 0) {
         fprintf(stderr, "ZIP_SOURCE_READ SUCCESS\n");
         break;
      }
   }
   
   // zip_status = zip_source_read(wc_source, wc_zip_data, wc_zip_size);
   // if (zip_status < 0) {
   //    fprintf(stderr, "ZIP_SOURCE_READ FAILED: %s\n", "idk");
   // }

   zip_status = zip_source_close(wc_source);
   if (zip_status < 0) {
      fprintf(stderr, "ZIP_SOURCE_CLOSE FAILED: %s\n", "idk");
   }

   //done with source so free it.
   zip_source_free(wc_source);

   //done with error fixture
   zip_error_fini(&zip_err);


   //wc_zip_data has our zip file now.
   //encode it in base64, and send it off.
   char* wc_data_encoded = base64_encode_binary(wc_zip_data, wc_zip_size);

   fprintf(stderr, "ENCODED WC:\n\n%s\n\n", wc_data_encoded);

   HttpResponse response = {0};
   if (HttpResponseInit(&response, HTTP_1_1, HttpStatus_OK).status == COT_ERROR) {
      return sendRedirect("/", clientfd);
   }

   //Build the HTML for the download button to send.

   dataVector vector = dataVectorInit(128);
   dataVectorPushString(&vector, "<div> Card sent </div><br>\n");
   dataVectorPushString(&vector, "<a href=\"data:application/zip;base64,");
   dataVectorPushString(&vector, wc_data_encoded);
   dataVectorPushString(&vector, "\" download=\"ivgift_wondercard.zip\">\n");
   dataVectorPushString(&vector, "<button type=\"button\">Download Card</button>\n");
   dataVectorPushString(&vector, "</a>");

   
   HttpResponseAddPayload(&response, vector.data, strlen(vector.data));
   char payload_size[100] = {0};
   sprintf(payload_size, "%zu", response.payload_size);
   
   HttpResponseAddOption(&response, "Content-Length", payload_size);
   HttpResponseAddOption(&response, "Content-Type", "text/plaintext");
   HttpResponseAddOption(&response, "Connection", "close");

   bool status = sendCustom(response, clientfd);
   HttpResponseFree(response);
   if (vector.data) free(vector.data);



   return status;
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

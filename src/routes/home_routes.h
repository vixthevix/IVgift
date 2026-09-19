#include "../cottage/cottage.h"
#include "../base64/base64.h"

typedef enum GiftType {
   POKEMON = 0,
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
   WonderCard card = {0};
   WonderCard error = {0};
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
   

   return card;
}

WonderCard readMYG(uint16_t* bin) {
   const uint16_t size = 936;

   //an MYG file does not add any new information to the card.
   //So we read from offset 0x50
   
   return readPCD(&bin[0x50]);
}


WonderCard WonderCardInit(char* file) {
   WonderCard card = {0};
   if (!file) return card;
   
   //Get the file length, see if valid.
   const uint32_t
   pgt_size = 260,
   pcd_size = 856,
   myg_size = 936;

   size_t file_size = base64_decode_size(file);
   char* file_url = urlDecode(file);
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
   return card;
}

bool sendWonderCardEdit(int client, char* file) {
   
   
   return false;
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

      status = sendRedirect("/create", clientfd);
   }
   

   end:
   strMapFree(payload);
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

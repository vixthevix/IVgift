#include "../cottage/cottage.h"
#include "pokemon_data.h"

#define SPRITE_API_ITEM 0
#define SPRITE_API_POKEMON 1

HttpResponse sprite_api_build(siteVar* var, const int list_type) {
   HttpResponse response = {0};
   
   if (!var) {
      newResultError("sprite_api_build: args invalid.");
      return response;
   }
   uint_cot* index_container = (uint_cot*) siteVarAccess(var);
   if (!index_container) {
      newResultError("sprite_api_build: cannot get index container.");
      return response;
   }

   uint_cot index = *index_container;
   free(index_container);

   const uint_cot list_size = (list_type == SPRITE_API_ITEM) ? sizeof(items) / sizeof(items[0]) : sizeof(pokemons) / sizeof(pokemons[0]);

   if (index >= list_size) {
      newResultError("sprite_api_build: index too large.");
      return response;
   }

   char payload[256] = {0};

   //We need to also transform the pokemon and item names to have capital letters at the start.



   if (list_type == SPRITE_API_ITEM) {
      char item_name[100] = {0};
      snprintf(item_name, 100, items[index]);
      if ('a' <= item_name[0] && item_name[0] <= 'z')
         item_name[0] = (item_name[0] - 32);
      snprintf(payload, 256, 
         "<span class=\"pokesprite %s\"></span>\n"
         "%s", 
         items2[index], item_name);
   }
   else if (list_type == SPRITE_API_POKEMON) {
      if (index == 0) {
         snprintf(payload, 256, 
            "<span class=\"pokesprite none\"></span>\n"
            "%s", 
            pokemons[index]);
      }
      else {
         char pokemon_name[100] = {0};
         snprintf(pokemon_name, 100, pokemons[index]);
         if ('a' <= pokemon_name[0] && pokemon_name[0] <= 'z')
            pokemon_name[0] = (pokemon_name[0] - 32);
         snprintf(payload, 256, 
            "<span class=\"pokesprite pokemon %s\"></span>\n"
            "%s", 
            pokemons[index], pokemon_name);
      }
   }
   else {
      snprintf(payload, 256, "hello");
   }
   
   if (HttpResponseInit(&response, HTTP_1_1, HttpStatus_OK).status == COT_ERROR) {
      return response;
   }

   HttpResponseAddPayload(&response, payload, strlen(payload));
   char payload_size[100] = {0};
   sprintf(payload_size, "%zu", response.payload_size);


   HttpResponseAddOption(&response, "Content-Length", payload_size);
   HttpResponseAddOption(&response, "Content-Type", "text/plaintext");
   HttpResponseAddOption(&response, "Connection", "close");

   return response;
}

NewRouteFunction(sprite_apiGet) {
   //Deals with indexes, so convert to siteVar
   newResultError("sprite_apiGet: starting...");
   char* offloadPosition = strstr(request.target, "?");

   char* offload = NULL;
   if (offloadPosition && offloadPosition[1]) offload = offloadPosition + 1;

   //decode the url, and then send it.
   char* offloadDecode = urlDecode(offload);
   char* offloadClean = cleanupPath(offloadDecode);

   siteVar* payload = offloadToVariables(offloadClean);
   if (offloadDecode) free(offloadDecode);
   if (offloadClean) free(offloadClean);
   if (!payload) return false;
   
   siteVar* item_id = siteVarCompositeAccess(payload, "item_id");
   
   siteVar* pokemon_container_left = siteVarCompositeAccess(payload, "pokemon_container_left");
   siteVar* pokemon_container_middle = siteVarCompositeAccess(payload, "pokemon_container_middle");
   siteVar* pokemon_container_right = siteVarCompositeAccess(payload, "pokemon_container_right");
   
   HttpResponse response = {0};
   

   if (item_id) {
      response = sprite_api_build(item_id, SPRITE_API_ITEM);
   }
   else if (pokemon_container_left) {
      response = sprite_api_build(pokemon_container_left, SPRITE_API_POKEMON);
   }
   else if (pokemon_container_middle) {
      response = sprite_api_build(pokemon_container_middle, SPRITE_API_POKEMON);
   }
   else if (pokemon_container_right) {
      response = sprite_api_build(pokemon_container_right, SPRITE_API_POKEMON);
   }

   siteVarFree(item_id);
   siteVarFree(pokemon_container_left);
   siteVarFree(pokemon_container_middle);
   siteVarFree(pokemon_container_right);
   siteVarFree(payload);
   
   bool status = sendCustom(response, clientfd);
   HttpResponseFree(response);

   return status;
}
NewRouteFunction(sprite_apiPost) {
   return false;
}
NewRouteFunction(sprite_apiPut) {
   return false;
}
NewRouteFunction(sprite_apiDelete) {
   return false;
}

/*
Enter the following into your main code, under where you setup your routes:
RouteEntry sprite_api = {
    .routeGet = sprite_apiGet,
    .routePost = sprite_apiPost,
    .routePut = sprite_apiPut,
    .routeDelete = sprite_apiDelete
};


newRoute(YOUR SITE PATH HERE, sprite_api);
*/

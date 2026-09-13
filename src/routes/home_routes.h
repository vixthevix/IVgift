#include "../cottage/cottage.h"

NewRouteFunction(homeGet) {
   return defaultGet(request, clientfd, extraData, "assets/web/templates/home.html");
}
NewRouteFunction(homePost) {
   fprintf(stderr, "POST REQUEST CONTENTS:\n%s\n", request.payload);
   stringMap* payload = strMapInit();
   if (!payload || !request.payload) return false;
   
   char* payload_string = request.payload;
   char key[512] = {0};
   char value[512] = {0};
   int index = 0;
   bool is_key = true;
   for (int i = 0; i < strlen(payload_string); i++) {
      if (payload_string[i] == '=') {
         is_key = false;
         index = 0;
         continue;
      }
      if (payload_string[i] == '&') {
         is_key = true;
         index = 0;
         strMapInsert(&payload, key, value);
         memset(key, 0, 512);
         memset(value, 0, 512);
         continue;
      }
      
      if (is_key) key[index++] = payload_string[i];
      else value[index++] = payload_string[i];
   }
   if (!is_key && key[0] && value[0]) {
      strMapInsert(&payload, key, value);
   }

   char* action = strMapGet(payload, "action");
   if (action && strcmp(action, "switch_info") == 0) {
      fprintf(stderr, "yep\n");
      strMapFree(payload);
      return sendRedirect("/info", clientfd);
      
      //return defaultGet(request, clientfd, extraData, "assets/web/templates/info.html");
   }


   strMapFree(payload);
   return false;
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

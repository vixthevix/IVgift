#include "../cottage/cottage.h"

NewRouteFunction(homeGet) {
   return defaultGet(request, clientfd, extraData, "assets/web/templates/home.html");
}
NewRouteFunction(homePost) {
   fprintf(stderr, "POST REQUEST CONTENTS:\n%s\n", request.payload);
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

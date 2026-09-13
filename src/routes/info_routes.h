#include "../cottage/cottage.h"

NewRouteFunction(infoGet) {
   return defaultGet(request, clientfd, extraData, "assets/web/templates/info.html");
}
NewRouteFunction(infoPost) {
   return false;
}
NewRouteFunction(infoPut) {
   return false;
}
NewRouteFunction(infoDelete) {
   return false;
}

/*
Enter the following into your main code, under where you setup your routes:
RouteEntry info = {
    .routeGet = infoGet,
    .routePost = infoPost,
    .routePut = infoPut,
    .routeDelete = infoDelete
};


newRoute(YOUR SITE PATH HERE, info);
*/

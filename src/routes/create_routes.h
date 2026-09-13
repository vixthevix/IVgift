#include "../cottage/cottage.h"

NewRouteFunction(createGet) {
   return false;
}
NewRouteFunction(createPost) {
   return false;
}
NewRouteFunction(createPut) {
   return false;
}
NewRouteFunction(createDelete) {
   return false;
}

/*
Enter the following into your main code, under where you setup your routes:
RouteEntry create = {
    .routeGet = createGet,
    .routePost = createPost,
    .routePut = createPut,
    .routeDelete = createDelete
};


newRoute(YOUR SITE PATH HERE, create);
*/

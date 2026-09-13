#include "../cottage/cottage.h"

NewRouteFunction(editorGet) {
   return false;
}
NewRouteFunction(editorPost) {
   return false;
}
NewRouteFunction(editorPut) {
   return false;
}
NewRouteFunction(editorDelete) {
   return false;
}

/*
Enter the following into your main code, under where you setup your routes:
RouteEntry editor = {
    .routeGet = editorGet,
    .routePost = editorPost,
    .routePut = editorPut,
    .routeDelete = editorDelete
};


newRoute(YOUR SITE PATH HERE, editor);
*/

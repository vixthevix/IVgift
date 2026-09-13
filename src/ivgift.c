/*
IVgift frontend.
Linux program for creating and editing generation IV Pokemon Mystery Gifts.

Visit https://github.com/vixthevix/IVgift for more info.
*/

#define COTTAGE_START
#include "cottage/cottage.h"

#include "routes/home_routes.h"
#include "routes/info_routes.h"
#include "routes/create_routes.h"

/*
Frontend to be made using the cottage framework.
Main menu:
    Information
    Create:
        From Scratch
        From Template (pgt or pcd)
    Edit (pgt, pcd, myg)
*/
int main(void) {
    cottageInit();

    //Routes
    RouteEntry home = {
    .routeGet = homeGet,
    .routePost = homePost,
    .routePut = homePut,
    .routeDelete = homeDelete
    };
    RouteEntry info = {
        .routeGet = infoGet,
        .routePost = infoPost,
        .routePut = infoPut,
        .routeDelete = infoDelete
    };
    RouteEntry create = {
        .routeGet = createGet,
        .routePost = createPost,
        .routePut = createPut,
        .routeDelete = createDelete
    };

    newRoute("/", home);
    newRoute("/info", info);
    newRoute("/create", create);

    //Server setup
    const char* ADDRESS = "0.0.0.0";
    const char* PORT = "8080";
    const uint32_t CLIENT_MAX = 64;
    const int TIMEOUT = 10;
    ServerConfig* server = serverInit(ADDRESS, PORT, CLIENT_MAX);
    if (!server) {
        fprintf(stderr, "Could not set up server.\n");
        return 1;
    }
    fprintf(stderr, "Server running at http://%s:%s\n\n", (strcmp(ADDRESS, "0.0.0.0") == 0) ? "localhost":ADDRESS, PORT);

    while (true) {
        //Number of ready clients
        int ready_count = CotPollPoll(server->poll, TIMEOUT);
        for (int i = 0; i < ready_count; i++) {
            int active_fd = CotPollAccess(server->poll, i);
            if (active_fd == server->server_fd) {
                //new client
                int clientfd = serverAcceptClient(server);
                if (clientfd < 0) continue;
                
                CotPollPush(server->poll, clientfd);
            }
            else {
                //existing client
                int bytes = 0;
                char* clientOffload = serverRecvClient(active_fd, &bytes);
                HttpRequest request = {0};
                if (splitHttpRequest(&request, clientOffload) .status == COT_ERROR) {
                    continue;
                }
                //debugHttpRequest(request);

                //quick extraData
                siteVar* extraData = siteVarInit("global", COMPOSITE, 0, NULL);
                siteVarCompositeInsertNew(&extraData, "peak", UINT, 1, &((uint_cot){67}));

                if (!handleRequest(request, active_fd, extraData, GLOBALROUTES)) {
                    perror("could not handle request\n");
                    sendError(active_fd, ERROR_404);
                }

                //Cleanup
                HttpRequestFree(request);
                siteVarFree(extraData);
                CotPollPop(server->poll, active_fd);
                if (clientOffload) free(clientOffload);
                serverCloseClient(active_fd);
            }
        }
    }

    serverClose(server);
    RouteMapFree(GLOBALROUTES);
    return 0;
}
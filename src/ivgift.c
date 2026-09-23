/*
IVgift frontend.
Linux program for creating and editing generation IV Pokemon Mystery Gifts.

Visit https://github.com/vixthevix/IVgift for more info.
*/

#define COTTAGE_START
#include "cottage/cottage.h"

#include <time.h>

#include "routes/home_routes.h"
#include "routes/info_routes.h"
#include "routes/create_routes.h"
#include "routes/sprite_api_routes.h"
#include "routes/generate_api_routes.h"

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
    RouteEntry sprite_api = {
        .routeGet = sprite_apiGet,
        .routePost = sprite_apiPost,
        .routePut = sprite_apiPut,
        .routeDelete = sprite_apiDelete
    };
    RouteEntry generate_api = {
        .routeGet = generate_apiGet,
        .routePost = generate_apiPost,
        .routePut = generate_apiPut,
        .routeDelete = generate_apiDelete
    };

    newRoute("/", home);
    newRoute("/info", info);
    newRoute("/create", create);
    newRoute("/api/get_sprite", sprite_api);
    newRoute("/api/generate", generate_api);

    //Server setup
    const char* ADDRESS = "0.0.0.0";
    const char* PORT = "8080";
    const uint32_t CLIENT_MAX = 64;
    const int TIMEOUT = 1;
    ServerConfig* server = serverInit(ADDRESS, PORT, CLIENT_MAX);
    if (!server) {
        fprintf(stderr, "Could not set up server.\n");
        return 1;
    }
    fprintf(stderr, "Server running at http://%s:%s\n\n", (strcmp(ADDRESS, "0.0.0.0") == 0) ? "localhost":ADDRESS, PORT);
    
    const char* special_chars[] = {
        "0020",
        "0021",
        "0022",
        "0023",
        "0024",
        "0025",
        "0026",
        "0027",
        "0028",
        "0029",
        "002A",
        "002B",
        "002C",
        "002D",
        "002E",
        "002F",
        "0030",
        "0031",
        "0032",
        "0033",
        "0034",
        "0035",
        "0036",
        "0037",
        "0038",
        "0039",
        "003A",
        "003B",
        "003C",
        "003D",
        "003E",
        "003F",
        "0040",
        "0041",
        "0042",
        "0043",
        "0044",
        "0045",
        "0046",
        "0047",
        "0048",
        "0049",
        "004A",
        "004B",
        "004C",
        "004D",
        "004E",
        "004F",
        "0050",
        "0051",
        "0052",
        "0053",
        "0054",
        "0055",
        "0056",
        "0057",
        "0058",
        "0059",
        "005A",
        "005B",
        "005C",
        "005D",
        "005E",
        "005F",
        "0060",
        "0061",
        "0062",
        "0063",
        "0064",
        "0065",
        "0066",
        "0067",
        "0068",
        "0069",
        "006A",
        "006B",
        "006C",
        "006D",
        "006E",
        "006F",
        "0070",
        "0071",
        "0072",
        "0073",
        "0074",
        "0075",
        "0076",
        "0077",
        "0078",
        "0079",
        "007A",
        "007E",
        "00A1",
        "00A2",
        "00A3",
        "00A4",
        "00A5",
        "00A9",
        "00AB",
        "00B7",
        "00BA",
        "00BB",
        "00BF",
        "00C0",
        "00C1",
        "00C2",
        "00C3",
        "00C4",
        "00C5",
        "00C7",
        "00C8",
        "00C9",
        "00CA",
        "00CB",
        "00CC",
        "00CD",
        "00CE",
        "00CF",
        "00D1",
        "00D2",
        "00D3",
        "00D4",
        "00D5",
        "00D6",
        "00D7",
        "00D9",
        "00DA",
        "00DB",
        "00DC",
        "00DD",
        "00DF",
        "00E0",
        "00E1",
        "00E2",
        "00E3",
        "00E4",
        "00E5",
        "00E7",
        "00E8",
        "00E9",
        "00EA",
        "00EB",
        "00EC",
        "00ED",
        "00EE",
        "00EF",
        "00F1",
        "00F2",
        "00F3",
        "00F4",
        "00F5",
        "00F6",
        "00F7",
        "00F9",
        "00FA",
        "00FB",
        "00FC",
        "00FD",
        "00FF",
        "0178",
        "2018",
        "2019",
        "201C",
        "201D",
        "201E",
        "2026",
        "20AC",
        "2190",
        "2191",
        "2192",
        "2193",
        "23FE",
        "244B",
        "244C",
        "244D",
        "244E",
        "244F",
        "2450",
        "2451",
        "2452",
        "25AF",
        "25B3",
        "25BA",
        "25C7",
        "2600",
        "2601",
        "2602",
        "2603",
        "2605",
        "2638",
        "2639",
        "263A",
        "263B",
        "2640",
        "2642",
        "2660",
        "2663",
        "2665",
        "2666",
        "266A",
        "2934",
        "2935",
        "29BE",
        "29BF",
        "300C",
        "300D",
        "300E",
        "300F",
        "3041",
        "3042",
        "3043",
        "3044",
        "3045",
        "3046",
        "3047",
        "3048",
        "3049",
        "304A",
        "304B",
        "304C",
        "304D",
        "304E",
        "304F",
        "3050",
        "3051",
        "3052",
        "3053",
        "3054",
        "3055",
        "3056",
        "3057",
        "3058",
        "3059",
        "305A",
        "305B",
        "305C",
        "305D",
        "305E",
        "305F",
        "3060",
        "3061",
        "3062",
        "3063",
        "3064",
        "3065",
        "3066",
        "3067",
        "3068",
        "3069",
        "306A",
        "306B",
        "306C",
        "306D",
        "306E",
        "306F",
        "3070",
        "3071",
        "3072",
        "3073",
        "3074",
        "3075",
        "3076",
        "3077",
        "3078",
        "3079",
        "307A",
        "307B",
        "307C",
        "307D",
        "307E",
        "307F",
        "3080",
        "3081",
        "3082",
        "3083",
        "3084",
        "3085",
        "3086",
        "3087",
        "3088",
        "3089",
        "308A",
        "308B",
        "308C",
        "308D",
        "308E",
        "3092",
        "3093",
        "30A1",
        "30A2",
        "30A3",
        "30A4",
        "30A5",
        "30A6",
        "30A7",
        "30A8",
        "30A9",
        "30AA",
        "30AB",
        "30AC",
        "30AD",
        "30AE",
        "30AF",
        "30B0",
        "30B1",
        "30B2",
        "30B3",
        "30B4",
        "30B5",
        "30B6",
        "30B7",
        "30B8",
        "30B9",
        "30BA",
        "30BB",
        "30BC",
        "30BD",
        "30BE",
        "30BF",
        "30C0",
        "30C1",
        "30C2",
        "30C3",
        "30C4",
        "30C5",
        "30C6",
        "30C7",
        "30C8",
        "30C9",
        "30CA",
        "30CB",
        "30CC",
        "30CD",
        "30CE",
        "30CF",
        "30D0",
        "30D1",
        "30D2",
        "30D3",
        "30D4",
        "30D5",
        "30D6",
        "30D7",
        "30D8",
        "30D9",
        "30DA",
        "30DB",
        "30DC",
        "30DD",
        "30DE",
        "30DF",
        "30E0",
        "30E1",
        "30E2",
        "30E3",
        "30E4",
        "30E5",
        "30E6",
        "30E7",
        "30E8",
        "30E9",
        "30EA",
        "30EB",
        "30EC",
        "30ED",
        "30EF",
        "30F2",
        "30F3",
    };

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
                debugHttpRequest(request);

                //Current date in HTML format (D-M-Y)
                time_t t = time(NULL);
                struct tm* tm_t = localtime(&t);
                char curDate[50] = {0};
                //snprintf(curDate, 50, "%d-%02d-%02d", tm_t->tm_mday, tm_t->tm_mon + 1, tm_t->tm_year + 1900);
                snprintf(curDate, 50, "%d-%02d-%d", tm_t->tm_year + 1900, tm_t->tm_mon + 1, tm_t->tm_mday);


                //Global container
                siteVar* global = siteVarInit("global", COMPOSITE, 0, NULL);

                newResultError("main: global start.");

                //Unicode string list for gen 4 font
                siteVar* special_chars_var = siteVarInit("special_chars", STRING, 0, NULL);
                siteVarInsertRange(&special_chars_var, special_chars, sizeof(special_chars)/sizeof(special_chars[0]));
                siteVarCompositeInsert(&global, special_chars_var);

                newResultError("main: unicode complete.");

                //Current date
                siteVarCompositeInsertNew(&global, "server_date", STRING, 1, &(char*){curDate});

                newResultError("main: global complete.");


                if (!handleRequest(request, active_fd, global, GLOBALROUTES)) {
                    perror("could not handle request\n");
                    sendError(active_fd, ERROR_404);
                }

                //Cleanup
                HttpRequestFree(request);
                siteVarFree(special_chars_var);
                siteVarFree(global);
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
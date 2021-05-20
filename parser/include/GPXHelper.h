#include "GPXParser.h"

#ifndef H_HELP

    // A2

    xmlDocPtr GPXDoctoXMLDoc(GPXdoc *doc);
    float lenBetween(Waypoint *wpt1, Waypoint *wpt2);
    void fakeDelete(void *data);
    void add_other_element(xmlNodePtr root, char *str, List *list);
    char* createGPXJSON(char *filename, char *scheme);

    // A3

    int numTrackPoints(Track *trk);
    char *detailedTrackToJSON(Track *trk);
    char *getDetailedTrackInfo(char *filename, char *schema, char *name);
    char *detailedRouteToJSON(Route *rt);
    char *getDetailedRouteInfo(char *filename, char *schema, char *name);
    bool validateFilename(char *filename, char *schema);
    bool jsonToWriteGPXdoc(char *gpxJSON, char *filename, char *schema);
    bool changeTrackName(char *filename, char *name, int id);
    bool changeRouteName(char *filename, char *name, int id);
    bool addRouteToFile(char *filename, char *routeJSON);
    int filenameToNumRoutes(char *filename);
    bool addWaypointToRoute(char *filename, int rte, char *wptJSON);
    char *fileToJSONTracksBetween(char *filename, float lat1, float lat2, float lon1, float lon2, float d);
    char *fileToJSONRoutesBetween(char *filename, float lat1, float lat2, float lon1, float lon2, float d);
    int fileToGetRoutesOfLength(char *filename, float length);
    int fileToGetTracksOfLength(char *filename, float length);

    // A4

    char *WaypointToJSON(const Waypoint* wpt);
    char *WaypointListToJSON(const List *list);
    char *RouteToWptList(char *filename, int routeNum);

#endif // H_HELP

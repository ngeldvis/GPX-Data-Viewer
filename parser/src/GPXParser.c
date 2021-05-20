#include <stdlib.h>
#include <math.h>

#include "GPXParser.h"
#include "GPXHelper.h"
#include "LinkedListAPI.h"

/* Name:        Nigel Davis   
 * Student ID:  1105413 
 * Course:      CIS-2750
\* Due Date:    March 11th, 2021 */

///////////////////////////////////////////////////////////////////////////////////////////////
/////////   .     .            .     .          .    .       .         .   .    .     /////////
/////////     .          .          .   ASSIGNMENT 1     .          .               . /////////
///////// .        .   .         .       .       .             .       .     .        /////////
///////////////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------------------------------------------
// Main Functions
// --------------------------------------------------------------------------------------------------------------

GPXdoc* createGPXdoc(char* fileName) {

    // check if filename is valid
    if(fileName == NULL || !strcmp(fileName, "")) {
        return NULL;
    }

    // check if file is readable
    FILE *fp;
    if((fp = fopen(fileName, "r")) == NULL) {
        return NULL;
    }
    fclose(fp);

    // get the xmlDoc and see if it returns null
    xmlDoc *doc = xmlReadFile(fileName, NULL, 0);
    if(doc == NULL) {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    LIBXML_TEST_VERSION

    // allocate memory for a new gpxDoc
    GPXdoc *gpxDoc = malloc(sizeof(GPXdoc));
    if(gpxDoc == NULL) {
        return NULL;
    }
    gpxDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    gpxDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    gpxDoc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    if(gpxDoc->waypoints == NULL || gpxDoc->routes == NULL || gpxDoc->tracks == NULL) {
        deleteGPXdoc(gpxDoc);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    // get the room element and loop through its attributes
    xmlNode *root = xmlDocGetRootElement(doc);
    if(root == NULL) {
        deleteGPXdoc(gpxDoc);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    if(root->ns == NULL) {
        deleteGPXdoc(gpxDoc);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }
    if(root->ns->href == NULL) {
        deleteGPXdoc(gpxDoc);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }
    strcpy(gpxDoc->namespace, (char *) root->ns->href);

    xmlAttr *attr;
    for(attr = root->properties; attr != NULL; attr = attr->next) {

        char *attrName = (char *) attr->name;
        char *cont = (char *) ((attr->children)->content);

        if(!strcmp(attrName, "version")) {
            gpxDoc->version = atof(cont);
        } else if(!strcmp(attrName, "creator")) {
            gpxDoc->creator = malloc(strlen(cont)+1);
            if(gpxDoc->creator == NULL) {
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            strcpy(gpxDoc->creator, cont);
        }
    }

    xmlNode *cur_node = NULL;
    for (cur_node = root->children; cur_node != NULL; cur_node = cur_node->next) {

        char *name = (char *) cur_node->name;

        // waypoint element
        if(!strcmp(name, "wpt")) {
            Waypoint *wpt = malloc(sizeof(Waypoint));
            if(wpt == NULL) {
                deleteWaypoint(wpt);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
            if(wpt->otherData == NULL) {
                deleteWaypoint(wpt);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            wpt->name = malloc(1);
            if(wpt->name == NULL) {
                deleteWaypoint(wpt);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            strcpy(wpt->name, "");

            // waypoint attributes
            xmlAttr *wptAttr = NULL;
            for (wptAttr = cur_node->properties; wptAttr != NULL; wptAttr = wptAttr->next) {
                xmlNode *value = wptAttr->children;
                char *attrName = (char *) wptAttr->name;
                char *cont = (char *)(value->content);
                if(!strcmp(attrName, "lat")) {
                    wpt->latitude = atof(cont);
                } else if(!strcmp(attrName, "lon")) {
                    wpt->longitude = atof(cont);
                }
            }

            // waypoint children
            xmlNode *child = NULL;
            for(child = cur_node->children; child != NULL; child = child->next) {
                if(child->type == XML_ELEMENT_NODE) {
                    if(child->children == NULL) {
                        deleteWaypoint(wpt);
                        deleteGPXdoc(gpxDoc);
                        xmlFreeDoc(doc);
                        xmlCleanupParser();
                        return NULL;
                    }

                    // waypoint name
                    if(!strcmp((char *) child->name, "name")) {
                        wpt->name = realloc(wpt->name, strlen((char *)(child->children)->content)+1);
                        if(wpt->name == NULL) {
                            deleteWaypoint(wpt);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        strcpy(wpt->name, (char *)(child->children)->content);
                        
                    // other waypoint data
                    } else {
                        if(child->children != NULL) {
                            GPXData *otherdata = malloc(sizeof(GPXData) + strlen((char *)(child->children)->content)+1);
                            if(otherdata == NULL) {
                                deleteGpxData(otherdata);
                                deleteWaypoint(wpt);
                                deleteGPXdoc(gpxDoc);
                                xmlFreeDoc(doc);
                                xmlCleanupParser();
                                return NULL;
                            }
                            strcpy(otherdata->name, (char *) child->name);
                            strcpy(otherdata->value, (char *)(child->children)->content);
                            insertBack(wpt->otherData, otherdata);
                        } else {
                            deleteWaypoint(wpt);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                    }
                }
            }
            insertBack(gpxDoc->waypoints, wpt);

        // route element
        } else if(!strcmp(name, "rte")) {
            Route *rte = malloc(sizeof(Route));
            if(rte == NULL) {
                deleteRoute(rte);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
            rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
            if(rte->waypoints == NULL || rte->otherData == NULL) {
                deleteRoute(rte);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            rte->name = malloc(1);
            if(rte->name == NULL) {
                deleteRoute(rte);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            strcpy(rte->name, "");

            // route children
            xmlNode *child = NULL;
            for(child = cur_node->children; child != NULL; child = child->next) {
                if(child->type == XML_ELEMENT_NODE) {

                    // route waypoints
                    if(!strcmp((char *) child->name, "rtept")) {
                        Waypoint *rtewpt = malloc(sizeof(Waypoint));
                        if(rtewpt == NULL) {
                            deleteWaypoint(rtewpt);
                            deleteRoute(rte);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        rtewpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                        if(rtewpt->otherData == NULL) {
                            deleteWaypoint(rtewpt);
                            deleteRoute(rte);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        rtewpt->name = malloc(1);
                        if(rtewpt->name == NULL) {
                            deleteWaypoint(rtewpt);
                            deleteRoute(rte);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        strcpy(rtewpt->name, "");

                        // route waypoint attributes
                        xmlAttr *rtewptAttr = NULL;
                        for (rtewptAttr = child->properties; rtewptAttr != NULL; rtewptAttr = rtewptAttr->next) {
                            xmlNode *value = rtewptAttr->children;
                            char *attrName = (char *) rtewptAttr->name;
                            char *cont = (char *)(value->content);
                            if(!strcmp(attrName, "lat")) {
                                rtewpt->latitude = atof(cont);
                            } else if(!strcmp(attrName, "lon")) {
                                rtewpt->longitude = atof(cont);
                            }
                        }

                        // route waypoint children
                        xmlNode *wptchild = NULL;
                        for(wptchild = child->children; wptchild != NULL; wptchild = wptchild->next) {
                            if(wptchild->type == XML_ELEMENT_NODE) {
                                if(wptchild->children == NULL) {
                                    deleteWaypoint(rtewpt);
                                    deleteRoute(rte);
                                    deleteGPXdoc(gpxDoc);
                                    xmlFreeDoc(doc);
                                    xmlCleanupParser();
                                    return NULL;
                                }

                                // route waypoint name
                                if(!strcmp((char *) wptchild->name, "name")) {
                                    rtewpt->name = realloc(rtewpt->name, strlen((char *)(wptchild->children)->content)+1);
                                    if(rtewpt->name == NULL) {
                                        deleteWaypoint(rtewpt);
                                        deleteRoute(rte);
                                        deleteGPXdoc(gpxDoc);
                                        xmlFreeDoc(doc);
                                        xmlCleanupParser();
                                        return NULL;
                                    }
                                    strcpy(rtewpt->name, (char *)(wptchild->children)->content);

                                // other route waypoint data
                                } else {
                                    GPXData *otherdata = malloc(sizeof(GPXData) + strlen((char *)(wptchild->children)->content)+1);
                                    if(otherdata == NULL) {
                                        deleteWaypoint(rtewpt);
                                        deleteGpxData(otherdata);
                                        deleteRoute(rte);
                                        deleteGPXdoc(gpxDoc);
                                        xmlFreeDoc(doc);
                                        xmlCleanupParser();
                                        return NULL;
                                    }
                                    strcpy(otherdata->name, (char *) wptchild->name);
                                    strcpy(otherdata->value, (char *)(wptchild->children)->content);
                                    insertBack(rtewpt->otherData, otherdata);
                                }
                            }
                        }
                        insertBack(rte->waypoints, rtewpt);
                    } else {
                        if(child->children == NULL) {
                            deleteRoute(rte);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }

                        // route name
                        if(!strcmp((char *) child->name, "name")) {
                            rte->name = realloc(rte->name, strlen((char *)(child->children)->content)+1);
                            if(rte->name == NULL) {
                                deleteRoute(rte);
                                deleteGPXdoc(gpxDoc);
                                xmlFreeDoc(doc);
                                xmlCleanupParser();
                                return NULL;
                            }
                            strcpy(rte->name, (char *) (child->children)->content);
                            
                        // other route data
                        } else {
                            GPXData *otherdata = malloc(sizeof(GPXData) + strlen((char *)(child->children)->content)+1);
                            if(otherdata == NULL) {
                                deleteGpxData(otherdata);
                                deleteRoute(rte);
                                deleteGPXdoc(gpxDoc);
                                xmlFreeDoc(doc);
                                xmlCleanupParser();
                                return NULL;
                            }
                            strcpy(otherdata->name, (char *) child->name);
                            strcpy(otherdata->value, (char *)(child->children)->content);
                            insertBack(rte->otherData, otherdata);
                        } 
                    }
                }
            }
            insertBack(gpxDoc->routes, rte);
        // track element
        } else if(!strcmp(name, "trk")) {
            Track *trk = malloc(sizeof(Track));
            if(trk == NULL) {
                deleteTrack(trk);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            trk->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
            trk->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
            if(trk->segments == NULL || trk->otherData == NULL) {
                deleteTrack(trk);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            trk->name = malloc(1);
            if(trk->name == NULL) {
                deleteTrack(trk);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            strcpy(trk->name, "");

            // track children
            xmlNode *child = NULL;
            for(child = cur_node->children; child != NULL; child = child->next) {
                if(child->type == XML_ELEMENT_NODE) {
                    if(child->children == NULL) {
                        deleteTrack(trk);
                        deleteGPXdoc(gpxDoc);
                        xmlFreeDoc(doc);
                        xmlCleanupParser();
                        return NULL;
                    }

                    // track name
                    if(!strcmp((char *) child->name, "name")) {
                        trk->name = realloc(trk->name, strlen((char *) (child->children)->content)+1);
                        if(trk->name == NULL) {
                            deleteTrack(trk);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        strcpy(trk->name, (char *) (child->children)->content);

                    // track segment
                    } else if(!strcmp((char *) child->name, "trkseg")) {
                        TrackSegment *trkseg = malloc(sizeof(TrackSegment));
                        if(trkseg == NULL) {
                            deleteTrackSegment(trkseg);
                            deleteTrack(trk);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        trkseg->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
                        if(trkseg->waypoints == NULL) {
                            deleteTrackSegment(trkseg);
                            deleteTrack(trk);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        xmlNode *trkpts = NULL;

                        //track segment children
                        for(trkpts = child->children; trkpts != NULL; trkpts = trkpts->next) {
                            if(trkpts->type == XML_ELEMENT_NODE) {
                                Waypoint *trkpt = malloc(sizeof(Waypoint));
                                if(trkpt == NULL) {
                                    deleteWaypoint(trkpt);
                                    deleteTrackSegment(trkseg);
                                    deleteTrack(trk);
                                    deleteGPXdoc(gpxDoc);
                                    xmlFreeDoc(doc);
                                    xmlCleanupParser();
                                    return NULL;
                                }
                                trkpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                                if(trkpt->otherData == NULL) {
                                    deleteWaypoint(trkpt);
                                    deleteTrackSegment(trkseg);
                                    deleteTrack(trk);
                                    deleteGPXdoc(gpxDoc);
                                    xmlFreeDoc(doc);
                                    xmlCleanupParser();
                                    return NULL;
                                }
                                trkpt->name = malloc(1);
                                if(trkpt->name == NULL) {
                                    deleteWaypoint(trkpt);
                                    deleteTrackSegment(trkseg);
                                    deleteTrack(trk);
                                    deleteGPXdoc(gpxDoc);
                                    xmlFreeDoc(doc);
                                    xmlCleanupParser();
                                    return NULL;
                                }
                                strcpy(trkpt->name, "");

                                // track segment waypoint attributes
                                xmlAttr *trkptAttr = NULL;
                                for(trkptAttr = trkpts->properties; trkptAttr != NULL; trkptAttr = trkptAttr->next) {
                                    xmlNode *value = trkptAttr->children;
                                    char *attrName = (char *) trkptAttr->name;
                                    char *cont = (char *)(value->content);
                                    if(!strcmp(attrName, "lat")) {
                                        trkpt->latitude = atof(cont);
                                    } else if(!strcmp(attrName, "lon")) {
                                        trkpt->longitude = atof(cont);
                                    }
                                }

                                // track segment waypoint children
                                xmlNode *wptchild = NULL;
                                for(wptchild = trkpts->children; wptchild != NULL; wptchild = wptchild->next) {
                                    if(wptchild->type == XML_ELEMENT_NODE) {
                                        if(wptchild->children == NULL) {
                                            deleteWaypoint(trkpt);
                                            deleteTrackSegment(trkseg);
                                            deleteTrack(trk);
                                            deleteGPXdoc(gpxDoc);
                                            xmlFreeDoc(doc);
                                            xmlCleanupParser();
                                            return NULL;
                                        }


                                        // track segment waypoint name
                                        if(!strcmp((char *) wptchild->name, "name")) {
                                            trkpt->name = realloc(trkpt->name, strlen((char *)(wptchild->children)->content)+1);
                                            if(trkpt->name == NULL) {
                                                deleteWaypoint(trkpt);
                                                deleteTrackSegment(trkseg);
                                                deleteTrack(trk);
                                                deleteGPXdoc(gpxDoc);
                                                xmlFreeDoc(doc);
                                                xmlCleanupParser();
                                                return NULL;
                                            }
                                            strcpy(trkpt->name, (char *)(wptchild->children)->content);

                                        // track segment waypoint data
                                        } else {
                                            GPXData *otherdata = malloc(sizeof(GPXData) + strlen((char *)(wptchild->children)->content)+1);
                                            if(otherdata == NULL) {
                                                deleteGpxData(otherdata);
                                                deleteWaypoint(trkpt);
                                                deleteTrackSegment(trkseg);
                                                deleteTrack(trk);
                                                deleteGPXdoc(gpxDoc);
                                                xmlFreeDoc(doc);
                                                xmlCleanupParser();
                                                return NULL;
                                            }
                                            strcpy(otherdata->name, (char *) wptchild->name);
                                            strcpy(otherdata->value, (char *)(wptchild->children)->content);
                                            insertBack(trkpt->otherData, otherdata);
                                        }
                                    }
                                }
                                insertBack(trkseg->waypoints, trkpt);
                            }
                        }
                        insertBack(trk->segments, trkseg);

                    // other track data
                    } else {
                        GPXData *otherdata = malloc(sizeof(GPXData) + strlen((char *)(child->children)->content)+1);
                        if(otherdata == NULL) {
                            deleteGpxData(otherdata);
                            deleteTrack(trk);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        strcpy(otherdata->name, (char *) child->name);
                        strcpy(otherdata->value, (char *)(child->children)->content);
                        insertBack(trk->otherData, otherdata);
                    }
                }
            }
            insertBack(gpxDoc->tracks, trk);
        }
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return gpxDoc;
}

char* GPXdocToString(GPXdoc* doc) {
    if(doc == NULL) {
        return NULL;
    }
    char *str = malloc(strlen(doc->creator) + strlen(doc->namespace) + 60);
    if(str == NULL) {
        return NULL;
    }
    sprintf(str, "[ GPX DOC ]\n\ncreator: %s\nversion: %f\nnamespace: %s\n", 
        doc->creator,
        doc->version,
        doc->namespace
    );
    if(getNumWaypoints(doc) > 0) {
        char *wptString = toString(doc->waypoints);
        str = realloc(str, strlen(str) + 16 + strlen(str) + strlen(wptString) + 2);
        if(str == NULL) {
            return NULL;
        }
        strcat(str, "\n[ WAYPOINTS ]\n");
        strcat(str, wptString);
        free(wptString);
        strcat(str, "\n");
    }
    if(getNumRoutes(doc) > 0) {
        char *rteString = toString(doc->routes);
        str = realloc(str, strlen(str) + 13 + strlen(str) + strlen(rteString) + 2);
        if(str == NULL) {
            return NULL;
        }
        strcat(str, "\n[ ROUTES ]\n");
        strcat(str, rteString);
        free(rteString);
        strcat(str, "\n");
    }
    if(getNumTracks(doc) > 0) {
        char *trkString = toString(doc->tracks);
        str = realloc(str, strlen(str) + 11 + strlen(str) + strlen(trkString) + 2);
        if(str == NULL) {
            return NULL;
        }
        strcat(str, "\n[ TRACKS ]\n");
        strcat(str, trkString);
        free(trkString);
    }
    strcat(str, "\n");
    return str;
}

void deleteGPXdoc(GPXdoc* doc) {
    if(doc == NULL) {
        return;
    }
    if(doc->waypoints != NULL) {
        freeList(doc->waypoints);
    }
    if(doc->routes != NULL) {
        freeList(doc->routes);
    }
    if(doc->tracks != NULL) {
        freeList(doc->tracks);
    }
    if(doc->creator != NULL) {
        free(doc->creator);
    }
    free(doc);
}

// --------------------------------------------------------------------------------------------------------------
// Get Number of Functions
// --------------------------------------------------------------------------------------------------------------

int getNumWaypoints(const GPXdoc* doc) {
    if(doc == NULL) {
        return 0;
    }
    return getLength(doc->waypoints);
}

int getNumRoutes(const GPXdoc* doc) {
    if(doc == NULL) {
        return 0;
    }
    return getLength(doc->routes);
}

int getNumTracks(const GPXdoc* doc) {
    if(doc == NULL) {
        return 0;
    }
    return getLength(doc->tracks);
}

int getNumSegments(const GPXdoc* doc) {
    if(doc == NULL) {
        return 0;
    }
    int count = 0;
    ListIterator iter = createIterator(doc->tracks);
	Track* elem;
	while((elem = nextElement(&iter)) != NULL) {
        count += getLength(elem->segments);
	}
    return count;
}

int getNumGPXData(const GPXdoc* doc) {
    if(doc == NULL) {
        return 0;
    }
    int count = 0;
    // Waypoints
    ListIterator wptIter = createIterator(doc->waypoints);
	Waypoint* wpt;
	while((wpt = nextElement(&wptIter)) != NULL) {
        if(strcmp(wpt->name, "")) {
            count++;
        }
        count += getLength(wpt->otherData);
	}
    // Routes
    ListIterator rteIter = createIterator(doc->routes);
    Route* rte;
	while((rte = nextElement(&rteIter)) != NULL) {
        if(strcmp(rte->name, "")) {
            count++;
        }
        count += getLength(rte->otherData);
        // Route Waypoints
        ListIterator rteptIter = createIterator(rte->waypoints);
        Waypoint* rtept;
        while((rtept = nextElement(&rteptIter)) != NULL) {
            if(strcmp(rtept->name, "")) {
                count++;
            }
            count += getLength(rtept->otherData);
        }
	}
    // Tracks
    ListIterator trkIter = createIterator(doc->tracks);
	Track* trk;
	while((trk = nextElement(&trkIter)) != NULL) {
        if(strcmp(trk->name, "")) {
            count++;
        }
        count += getLength(trk->otherData);
        // Track Segments
        ListIterator trksegIter = createIterator(trk->segments);
        TrackSegment* trkseg;
        while((trkseg = nextElement(&trksegIter)) != NULL) {
            // Track Segment Waypoints
            ListIterator trkptIter = createIterator(trkseg->waypoints);
            Waypoint* trkpt;
            while((trkpt = nextElement(&trkptIter)) != NULL) {
                if(strcmp(trkpt->name, "")) {
                    count++;
                }
                count += getLength(trkpt->otherData);
            }
        }
	}
    return count;
}

// --------------------------------------------------------------------------------------------------------------
// Getter Functions
// --------------------------------------------------------------------------------------------------------------

Waypoint* getWaypoint(const GPXdoc* doc, char* name) {
    if(doc == NULL || name == NULL) {
        return NULL;
    }
    ListIterator iter = createIterator(doc->waypoints);
	Waypoint* elem;
	while((elem = nextElement(&iter)) != NULL) {
        if(!strcmp(elem->name, name)) {
            return elem;
        }
	}
    return NULL;
}

Track* getTrack(const GPXdoc* doc, char* name) {
    if(doc == NULL || name == NULL) {
        return NULL;
    }
    ListIterator iter = createIterator(doc->tracks);
	Track* elem;
	while((elem = nextElement(&iter)) != NULL) {
        if(!strcmp(elem->name, name)) {
            return elem;
        }
	}
    return NULL;
}

Route* getRoute(const GPXdoc* doc, char* name) {
    if(doc == NULL || name == NULL) {
        return NULL;
    }
    ListIterator iter = createIterator(doc->routes);
	Route* elem;
	while((elem = nextElement(&iter)) != NULL) {
        if(!strcmp(elem->name, name)) {
            return elem;
        }
	}
    return NULL;
}

// --------------------------------------------------------------------------------------------------------------
// List API Helper Functions
// --------------------------------------------------------------------------------------------------------------

// GPXData Functions

void deleteGpxData(void* data) {
    if(data == NULL) {
        return;
    }
    GPXData *other = (GPXData *) data;
    free(other);
}

char* gpxDataToString(void* data) {
    GPXData *gpxData = (GPXData *) data;
    char *str = malloc(strlen(gpxData->name) + strlen(gpxData->value) + 3);
    if(str == NULL) {
        return NULL;
    }
    sprintf(str, "%s=%s", gpxData->name, gpxData->value);
    return str;
}

int compareGpxData(const void *first, const void *second) {
    return 1;
}

// Waypoint Functions 

void deleteWaypoint(void* data) {
    if(data == NULL) {
        return;
    }
    Waypoint *wpt = (Waypoint *) data;
    if(wpt->otherData != NULL) {
        freeList(wpt->otherData);
    }
    if(wpt->name != NULL) {
        free(wpt->name);
    }
    free(wpt);
}

char* waypointToString(void* data) {
    Waypoint *wpt = (Waypoint *) data;
    char *str = malloc(strlen(wpt->name) + 60);
    if(str == NULL) {
        return NULL;
    }
    sprintf(str, "wpt:\nname=%s, lat=%f, lon=%f", wpt->name, wpt->latitude, wpt->longitude);
    if(getLength(wpt->otherData) > 0) {
        char *gpxString = toString(wpt->otherData);
        str = realloc(str, strlen(str) + 13 + strlen(str) + strlen(gpxString) + 1);
        if(str == NULL) {
            return NULL;
        }
        strcat(str, "\nOther data:");
        strcat(str, gpxString);
        free(gpxString);
    }
    return str;
}

int compareWaypoints(const void *first, const void *second) {
    return 1;
}

// Route Functions

void deleteRoute(void* data) {
    if(data == NULL) {
        return;
    }
    Route *rte = (Route *) data;
    if(rte->waypoints != NULL) {
        freeList(rte->waypoints);
    }
    if(rte->otherData != NULL) {
        freeList(rte->otherData);
    }
    if(rte->name != NULL) {
        free(rte->name);
    }
    free(rte);
}

char* routeToString(void* data) {
    Route *rte = (Route *) data;
    char *str = malloc(strlen(rte->name) + 12);
    if(str == NULL) {
        return NULL;
    }
    sprintf(str, "rte:\nname=%s", rte->name);
    if(getLength(rte->otherData) > 0) {
        char *gpxString = toString(rte->otherData);
        str = realloc(str, strlen(str) + 13 + strlen(str) + strlen(gpxString) + 1);
        if(str == NULL) {
            return NULL;
        }
        strcat(str, "\nOther data:");
        strcat(str, gpxString);
        free(gpxString);
    }
    char *wptString = toString(rte->waypoints);
    str = realloc(str, strlen(str) + strlen(wptString) + 1);
    if(str == NULL) {
        return NULL;
    }
    strcat(str, wptString);
    free(wptString);
    return str;
}

int compareRoutes(const void *first, const void *second) {
    return 1;
}

// Track Segment Functions

void deleteTrackSegment(void* data) {
    if(data == NULL) {
        return;
    }
    TrackSegment *trkseg = (TrackSegment *) data;
    if(trkseg->waypoints != NULL) {
        freeList(trkseg->waypoints);
    }
    free(trkseg);
}

char* trackSegmentToString(void* data) {
    TrackSegment *trkseg = (TrackSegment *) data;
    char *wptString = toString(trkseg->waypoints);
    char *str = malloc(strlen(wptString) + 10);
    if(str == NULL) {
        return NULL;
    }
    sprintf(str, "trkseg:%s", wptString);
    free(wptString);
    return str;
}

int compareTrackSegments(const void *first, const void *second) {
    return 1;
}

// Track Functions

void deleteTrack(void* data) {
    if(data == NULL) {
        return;
    }
    Track *trk = (Track *) data;
    if(trk->otherData != NULL) {
        freeList(trk->otherData);
    }
    if(trk->segments != NULL) {
        freeList(trk->segments);
    }
    if(trk->name != NULL) {
        free(trk->name);
    }
    free(trk);
}

char* trackToString(void* data) {
    Track *trk = (Track *) data;
    char *str = malloc(strlen(trk->name) + 12);
    if(str == NULL) {
        return NULL;
    }
    sprintf(str, "trk:\nname=%s", trk->name);
    if(getLength(trk->otherData) > 0) {
        char *gpxString = toString(trk->otherData);
        str = realloc(str, strlen(str) + 13 + strlen(str) + strlen(gpxString) + 1);
        if(str == NULL) {
            return NULL;
        }
        strcat(str, "\nOther data:");
        strcat(str, gpxString);
        free(gpxString);
    }
    char *trksegString = toString(trk->segments);
    str = realloc(str, strlen(str) + strlen(trksegString) + 1);
    if(str == NULL) {
        return NULL;
    }
    strcat(str, trksegString);
    free(trksegString);
    return str;
}

int compareTracks(const void *first, const void *second) {
    return 1;
}

void fakeDelete(void *data) {
    return;
}

// SECTION A2

///////////////////////////////////////////////////////////////////////////////////////////////
/////////   .     .            .     .          .    .       .         .   .    .     /////////
/////////     .          .          .   ASSIGNMENT 2     .          .               . /////////
///////// .        .   .         .       .       .             .       .     .        /////////
///////////////////////////////////////////////////////////////////////////////////////////////

// ANCHOR Module 1

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile) {

    // check if filename is valid
    if(fileName == NULL || !strcmp(fileName, "")) {
        return NULL;
    }
    // check if gpxSchemaFile is valid
    if(gpxSchemaFile == NULL || !strcmp(gpxSchemaFile, "")) {
        return NULL;
    }

    // check if file is readable
    FILE *fp;
    if((fp = fopen(fileName, "r")) == NULL) {
        return NULL;
    }
    fclose(fp);

    // get the xmlDoc and see if it returns null
    xmlDoc *doc = xmlReadFile(fileName, NULL, 0);
    if(doc == NULL) {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    LIBXML_TEST_VERSION

    // ANCHOR - createValid validator
    // validate gpxDoc

    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt1;

    xmlLineNumbersDefault(1);

    ctxt1 = xmlSchemaNewParserCtxt(gpxSchemaFile);
    xmlSchemaSetParserErrors(ctxt1, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt1);
    if(schema == NULL) {
        xmlSchemaFreeParserCtxt(ctxt1);
        xmlSchemaCleanupTypes();
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }
    xmlSchemaFreeParserCtxt(ctxt1);

    xmlSchemaValidCtxtPtr ctxt2;
    ctxt2 = xmlSchemaNewValidCtxt(schema);
    int ret = xmlSchemaValidateDoc(ctxt2, doc);
    if(ret != 0) {
        xmlSchemaFreeValidCtxt(ctxt2);
        if (schema != NULL) {
            xmlSchemaFree(schema);
        }
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }
    xmlSchemaFreeValidCtxt(ctxt2);
    if (schema != NULL) {
		xmlSchemaFree(schema);
    }
	xmlSchemaCleanupTypes();

    // create GPX Doc

    // allocate memory for a new gpxDoc
    GPXdoc *gpxDoc = malloc(sizeof(GPXdoc));
    if(gpxDoc == NULL) {
        return NULL;
    }
    gpxDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    gpxDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    gpxDoc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    if(gpxDoc->waypoints == NULL || gpxDoc->routes == NULL || gpxDoc->tracks == NULL) {
        deleteGPXdoc(gpxDoc);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    // get the room element and loop through its attributes
    xmlNode *root = xmlDocGetRootElement(doc);
    if(root == NULL) {
        deleteGPXdoc(gpxDoc);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    if(root->ns->href == NULL) {
        deleteGPXdoc(gpxDoc);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }
    strcpy(gpxDoc->namespace, (char *) root->ns->href);

    xmlAttr *attr;
    for(attr = root->properties; attr != NULL; attr = attr->next) {

        char *attrName = (char *) attr->name;
        char *cont = (char *) ((attr->children)->content);

        if(!strcmp(attrName, "version")) {
            gpxDoc->version = atof(cont);
        } else if(!strcmp(attrName, "creator")) {
            gpxDoc->creator = malloc(strlen(cont)+1);
            if(gpxDoc->creator == NULL) {
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            strcpy(gpxDoc->creator, cont);
        }
    }

    xmlNode *cur_node = NULL;
    for (cur_node = root->children; cur_node != NULL; cur_node = cur_node->next) {

        char *name = (char *) cur_node->name;

        // waypoint element
        if(!strcmp(name, "wpt")) {
            Waypoint *wpt = malloc(sizeof(Waypoint));
            if(wpt == NULL) {
                deleteWaypoint(wpt);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
            if(wpt->otherData == NULL) {
                deleteWaypoint(wpt);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            wpt->name = malloc(1);
            if(wpt->name == NULL) {
                deleteWaypoint(wpt);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            strcpy(wpt->name, "");

            // waypoint attributes
            xmlAttr *wptAttr = NULL;
            for (wptAttr = cur_node->properties; wptAttr != NULL; wptAttr = wptAttr->next) {
                xmlNode *value = wptAttr->children;
                char *attrName = (char *) wptAttr->name;
                char *cont = (char *)(value->content);
                if(!strcmp(attrName, "lat")) {
                    wpt->latitude = atof(cont);
                } else if(!strcmp(attrName, "lon")) {
                    wpt->longitude = atof(cont);
                }
            }

            // waypoint children
            xmlNode *child = NULL;
            for(child = cur_node->children; child != NULL; child = child->next) {
                if(child->type == XML_ELEMENT_NODE) {
                    if(child->children == NULL) {
                        deleteWaypoint(wpt);
                        deleteGPXdoc(gpxDoc);
                        xmlFreeDoc(doc);
                        xmlCleanupParser();
                        return NULL;
                    }

                    // waypoint name
                    if(!strcmp((char *) child->name, "name")) {
                        wpt->name = realloc(wpt->name, strlen((char *)(child->children)->content)+1);
                        if(wpt->name == NULL) {
                            deleteWaypoint(wpt);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        strcpy(wpt->name, (char *)(child->children)->content);
                        
                    // other waypoint data
                    } else {
                        if(child->children != NULL) {
                            GPXData *otherdata = malloc(sizeof(GPXData) + strlen((char *)(child->children)->content)+1);
                            if(otherdata == NULL) {
                                deleteGpxData(otherdata);
                                deleteWaypoint(wpt);
                                deleteGPXdoc(gpxDoc);
                                xmlFreeDoc(doc);
                                xmlCleanupParser();
                                return NULL;
                            }
                            strcpy(otherdata->name, (char *) child->name);
                            strcpy(otherdata->value, (char *)(child->children)->content);
                            insertBack(wpt->otherData, otherdata);
                        } else {
                            deleteWaypoint(wpt);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                    }
                }
            }
            insertBack(gpxDoc->waypoints, wpt);

        // route element
        } else if(!strcmp(name, "rte")) {
            Route *rte = malloc(sizeof(Route));
            if(rte == NULL) {
                deleteRoute(rte);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
            rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
            if(rte->waypoints == NULL || rte->otherData == NULL) {
                deleteRoute(rte);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            rte->name = malloc(1);
            if(rte->name == NULL) {
                deleteRoute(rte);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            strcpy(rte->name, "");

            // route children
            xmlNode *child = NULL;
            for(child = cur_node->children; child != NULL; child = child->next) {
                if(child->type == XML_ELEMENT_NODE) {

                    // route waypoints
                    if(!strcmp((char *) child->name, "rtept")) {
                        Waypoint *rtewpt = malloc(sizeof(Waypoint));
                        if(rtewpt == NULL) {
                            deleteWaypoint(rtewpt);
                            deleteRoute(rte);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        rtewpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                        if(rtewpt->otherData == NULL) {
                            deleteWaypoint(rtewpt);
                            deleteRoute(rte);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        rtewpt->name = malloc(1);
                        if(rtewpt->name == NULL) {
                            deleteWaypoint(rtewpt);
                            deleteRoute(rte);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        strcpy(rtewpt->name, "");

                        // route waypoint attributes
                        xmlAttr *rtewptAttr = NULL;
                        for (rtewptAttr = child->properties; rtewptAttr != NULL; rtewptAttr = rtewptAttr->next) {
                            xmlNode *value = rtewptAttr->children;
                            char *attrName = (char *) rtewptAttr->name;
                            char *cont = (char *)(value->content);
                            if(!strcmp(attrName, "lat")) {
                                rtewpt->latitude = atof(cont);
                            } else if(!strcmp(attrName, "lon")) {
                                rtewpt->longitude = atof(cont);
                            }
                        }

                        // route waypoint children
                        xmlNode *wptchild = NULL;
                        for(wptchild = child->children; wptchild != NULL; wptchild = wptchild->next) {
                            if(wptchild->type == XML_ELEMENT_NODE) {
                                if(wptchild->children == NULL) {
                                    deleteWaypoint(rtewpt);
                                    deleteRoute(rte);
                                    deleteGPXdoc(gpxDoc);
                                    xmlFreeDoc(doc);
                                    xmlCleanupParser();
                                    return NULL;
                                }

                                // route waypoint name
                                if(!strcmp((char *) wptchild->name, "name")) {
                                    rtewpt->name = realloc(rtewpt->name, strlen((char *)(wptchild->children)->content)+1);
                                    if(rtewpt->name == NULL) {
                                        deleteWaypoint(rtewpt);
                                        deleteRoute(rte);
                                        deleteGPXdoc(gpxDoc);
                                        xmlFreeDoc(doc);
                                        xmlCleanupParser();
                                        return NULL;
                                    }
                                    strcpy(rtewpt->name, (char *)(wptchild->children)->content);

                                // other route waypoint data
                                } else {
                                    GPXData *otherdata = malloc(sizeof(GPXData) + strlen((char *)(wptchild->children)->content)+1);
                                    if(otherdata == NULL) {
                                        deleteWaypoint(rtewpt);
                                        deleteGpxData(otherdata);
                                        deleteRoute(rte);
                                        deleteGPXdoc(gpxDoc);
                                        xmlFreeDoc(doc);
                                        xmlCleanupParser();
                                        return NULL;
                                    }
                                    strcpy(otherdata->name, (char *) wptchild->name);
                                    strcpy(otherdata->value, (char *)(wptchild->children)->content);
                                    insertBack(rtewpt->otherData, otherdata);
                                }
                            }
                        }
                        insertBack(rte->waypoints, rtewpt);
                    } else {
                        if(child->children == NULL) {
                            deleteRoute(rte);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }

                        // route name
                        if(!strcmp((char *) child->name, "name")) {
                            rte->name = realloc(rte->name, strlen((char *)(child->children)->content)+1);
                            if(rte->name == NULL) {
                                deleteRoute(rte);
                                deleteGPXdoc(gpxDoc);
                                xmlFreeDoc(doc);
                                xmlCleanupParser();
                                return NULL;
                            }
                            strcpy(rte->name, (char *) (child->children)->content);
                            
                        // other route data
                        } else {
                            GPXData *otherdata = malloc(sizeof(GPXData) + strlen((char *)(child->children)->content)+1);
                            if(otherdata == NULL) {
                                deleteGpxData(otherdata);
                                deleteRoute(rte);
                                deleteGPXdoc(gpxDoc);
                                xmlFreeDoc(doc);
                                xmlCleanupParser();
                                return NULL;
                            }
                            strcpy(otherdata->name, (char *) child->name);
                            strcpy(otherdata->value, (char *)(child->children)->content);
                            insertBack(rte->otherData, otherdata);
                        } 
                    }
                }
            }
            insertBack(gpxDoc->routes, rte);
        // track element
        } else if(!strcmp(name, "trk")) {
            Track *trk = malloc(sizeof(Track));
            if(trk == NULL) {
                deleteTrack(trk);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            trk->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
            trk->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
            if(trk->segments == NULL || trk->otherData == NULL) {
                deleteTrack(trk);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            trk->name = malloc(1);
            if(trk->name == NULL) {
                deleteTrack(trk);
                deleteGPXdoc(gpxDoc);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return NULL;
            }
            strcpy(trk->name, "");

            // track children
            xmlNode *child = NULL;
            for(child = cur_node->children; child != NULL; child = child->next) {
                if(child->type == XML_ELEMENT_NODE) {
                    if(child->children == NULL) {
                        deleteTrack(trk);
                        deleteGPXdoc(gpxDoc);
                        xmlFreeDoc(doc);
                        xmlCleanupParser();
                        return NULL;
                    }

                    // track name
                    if(!strcmp((char *) child->name, "name")) {
                        trk->name = realloc(trk->name, strlen((char *) (child->children)->content)+1);
                        if(trk->name == NULL) {
                            deleteTrack(trk);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        strcpy(trk->name, (char *) (child->children)->content);

                    // track segment
                    } else if(!strcmp((char *) child->name, "trkseg")) {
                        TrackSegment *trkseg = malloc(sizeof(TrackSegment));
                        if(trkseg == NULL) {
                            deleteTrackSegment(trkseg);
                            deleteTrack(trk);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        trkseg->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
                        if(trkseg->waypoints == NULL) {
                            deleteTrackSegment(trkseg);
                            deleteTrack(trk);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        xmlNode *trkpts = NULL;

                        //track segment children
                        for(trkpts = child->children; trkpts != NULL; trkpts = trkpts->next) {
                            if(trkpts->type == XML_ELEMENT_NODE) {
                                Waypoint *trkpt = malloc(sizeof(Waypoint));
                                if(trkpt == NULL) {
                                    deleteWaypoint(trkpt);
                                    deleteTrackSegment(trkseg);
                                    deleteTrack(trk);
                                    deleteGPXdoc(gpxDoc);
                                    xmlFreeDoc(doc);
                                    xmlCleanupParser();
                                    return NULL;
                                }
                                trkpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                                if(trkpt->otherData == NULL) {
                                    deleteWaypoint(trkpt);
                                    deleteTrackSegment(trkseg);
                                    deleteTrack(trk);
                                    deleteGPXdoc(gpxDoc);
                                    xmlFreeDoc(doc);
                                    xmlCleanupParser();
                                    return NULL;
                                }
                                trkpt->name = malloc(1);
                                if(trkpt->name == NULL) {
                                    deleteWaypoint(trkpt);
                                    deleteTrackSegment(trkseg);
                                    deleteTrack(trk);
                                    deleteGPXdoc(gpxDoc);
                                    xmlFreeDoc(doc);
                                    xmlCleanupParser();
                                    return NULL;
                                }
                                strcpy(trkpt->name, "");

                                // track segment waypoint attributes
                                xmlAttr *trkptAttr = NULL;
                                for(trkptAttr = trkpts->properties; trkptAttr != NULL; trkptAttr = trkptAttr->next) {
                                    xmlNode *value = trkptAttr->children;
                                    char *attrName = (char *) trkptAttr->name;
                                    char *cont = (char *)(value->content);
                                    if(!strcmp(attrName, "lat")) {
                                        trkpt->latitude = atof(cont);
                                    } else if(!strcmp(attrName, "lon")) {
                                        trkpt->longitude = atof(cont);
                                    }
                                }

                                // track segment waypoint children
                                xmlNode *wptchild = NULL;
                                for(wptchild = trkpts->children; wptchild != NULL; wptchild = wptchild->next) {
                                    if(wptchild->type == XML_ELEMENT_NODE) {
                                        if(wptchild->children == NULL) {
                                            deleteWaypoint(trkpt);
                                            deleteTrackSegment(trkseg);
                                            deleteTrack(trk);
                                            deleteGPXdoc(gpxDoc);
                                            xmlFreeDoc(doc);
                                            xmlCleanupParser();
                                            return NULL;
                                        }


                                        // track segment waypoint name
                                        if(!strcmp((char *) wptchild->name, "name")) {
                                            trkpt->name = realloc(trkpt->name, strlen((char *)(wptchild->children)->content)+1);
                                            if(trkpt->name == NULL) {
                                                deleteWaypoint(trkpt);
                                                deleteTrackSegment(trkseg);
                                                deleteTrack(trk);
                                                deleteGPXdoc(gpxDoc);
                                                xmlFreeDoc(doc);
                                                xmlCleanupParser();
                                                return NULL;
                                            }
                                            strcpy(trkpt->name, (char *)(wptchild->children)->content);

                                        // track segment waypoint data
                                        } else {
                                            GPXData *otherdata = malloc(sizeof(GPXData) + strlen((char *)(wptchild->children)->content)+1);
                                            if(otherdata == NULL) {
                                                deleteGpxData(otherdata);
                                                deleteWaypoint(trkpt);
                                                deleteTrackSegment(trkseg);
                                                deleteTrack(trk);
                                                deleteGPXdoc(gpxDoc);
                                                xmlFreeDoc(doc);
                                                xmlCleanupParser();
                                                return NULL;
                                            }
                                            strcpy(otherdata->name, (char *) wptchild->name);
                                            strcpy(otherdata->value, (char *)(wptchild->children)->content);
                                            insertBack(trkpt->otherData, otherdata);
                                        }
                                    }
                                }
                                insertBack(trkseg->waypoints, trkpt);
                            }
                        }
                        insertBack(trk->segments, trkseg);

                    // other track data
                    } else {
                        GPXData *otherdata = malloc(sizeof(GPXData) + strlen((char *)(child->children)->content)+1);
                        if(otherdata == NULL) {
                            deleteGpxData(otherdata);
                            deleteTrack(trk);
                            deleteGPXdoc(gpxDoc);
                            xmlFreeDoc(doc);
                            xmlCleanupParser();
                            return NULL;
                        }
                        strcpy(otherdata->name, (char *) child->name);
                        strcpy(otherdata->value, (char *)(child->children)->content);
                        insertBack(trk->otherData, otherdata);
                    }
                }
            }
            insertBack(gpxDoc->tracks, trk);
        }
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return gpxDoc;
}

// ANCHOR - After creatValid

void add_other_element(xmlNodePtr root, char *str, List *list) {
    ListIterator otherIter = createIterator(list);
    GPXData* other;
    while((other = nextElement(&otherIter)) != NULL) {
        if(!strcmp(other->name, str)) {
            xmlNewChild(root, NULL, BAD_CAST other->name, BAD_CAST other->value);
        }
    }
}

xmlDocPtr GPXDoctoXMLDoc(GPXdoc *doc) {
    if(doc == NULL) {
        return NULL;
    }

    xmlDocPtr xmldoc = xmlNewDoc(BAD_CAST "1.0");
    if(xmldoc == NULL) {
        return NULL;
    }
    xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "gpx");
    xmlDocSetRootElement(xmldoc, root);
    xmlNsPtr ns = xmlNewNs(root, BAD_CAST doc->namespace, NULL);
    xmlSetNs(root, ns);

    char *ver = malloc(20);
    sprintf(ver, "%.1lf", doc->version);
    xmlNewProp(root, BAD_CAST "version", BAD_CAST ver);
    free(ver);
    xmlNewProp(root, BAD_CAST "creator", BAD_CAST doc->creator);

    // Waypoints
    ListIterator wptIter = createIterator(doc->waypoints);
	Waypoint* wpt;
	while((wpt = nextElement(&wptIter)) != NULL) {
        xmlNodePtr wptnode = xmlNewChild(root, NULL, BAD_CAST "wpt", NULL);
        // waypoint attributes
        char *lat = malloc(20);
        sprintf(lat, "%.6lf", wpt->latitude);
        xmlNewProp(wptnode, BAD_CAST "lat", BAD_CAST lat);
        free(lat);
        char *lon = malloc(20);
        sprintf(lon, "%.6lf", wpt->longitude);
        xmlNewProp(wptnode, BAD_CAST "lon", BAD_CAST lon);
        free(lon);

        // waypoint children

        /*  <ele> xsd:decimal </ele> [0..1] ?
            <time> xsd:dateTime </time> [0..1] ?
            <magvar> degreesType </magvar> [0..1] ?
            <geoidheight> xsd:decimal </geoidheight> [0..1] ?
            <name> xsd:string </name> [0..1] ?
            <cmt> xsd:string </cmt> [0..1] ?
            <desc> xsd:string </desc> [0..1] ?
            <src> xsd:string </src> [0..1] ?
            <link> linkType </link> [0..*] ?
            <sym> xsd:string </sym> [0..1] ?
            <type> xsd:string </type> [0..1] ?
            <fix> fixType </fix> [0..1] ?
            <sat> xsd:nonNegativeInteger </sat> [0..1] ?
            <hdop> xsd:decimal </hdop> [0..1] ?
            <vdop> xsd:decimal </vdop> [0..1] ?
            <pdop> xsd:decimal </pdop> [0..1] ?
            <ageofdgpsdata> xsd:decimal </ageofdgpsdata> [0..1] ?
            <dgpsid> dgpsStationType </dgpsid> [0..1] ?
            <extensions> extensionsType </extensions> [0..1] ? */

        add_other_element(wptnode, "ele", wpt->otherData);
        add_other_element(wptnode, "time", wpt->otherData);
        add_other_element(wptnode, "magvar", wpt->otherData);
        add_other_element(wptnode, "geoidheight", wpt->otherData);
        if(strcmp(wpt->name, "") != 0) {
            xmlNewChild(wptnode, NULL, BAD_CAST "name", BAD_CAST wpt->name);
        }
        add_other_element(wptnode, "cmt", wpt->otherData);
        add_other_element(wptnode, "desc", wpt->otherData);
        add_other_element(wptnode, "link", wpt->otherData);
        add_other_element(wptnode, "type", wpt->otherData);
        add_other_element(wptnode, "fix", wpt->otherData);
        add_other_element(wptnode, "sat", wpt->otherData);
        add_other_element(wptnode, "hdop", wpt->otherData);
        add_other_element(wptnode, "vdop", wpt->otherData);
        add_other_element(wptnode, "pdop", wpt->otherData);
        add_other_element(wptnode, "ageofdgpsdata", wpt->otherData);
        add_other_element(wptnode, "dgpsid", wpt->otherData);
        add_other_element(wptnode, "extensions", wpt->otherData);      

        {
            ListIterator otherIter = createIterator(wpt->otherData);
            GPXData* other;
            while((other = nextElement(&otherIter)) != NULL) {
                if(strcmp(other->name, "ele") != 0 && strcmp(other->name, "time") != 0 &&
                   strcmp(other->name, "magvar") != 0 && strcmp(other->name, "geoidheight") != 0 &&
                   strcmp(other->name, "cmt") != 0 && strcmp(other->name, "desc") != 0 &&
                   strcmp(other->name, "link") != 0 && strcmp(other->name, "type") != 0 &&
                   strcmp(other->name, "fix") != 0 && strcmp(other->name, "sat") != 0 &&
                   strcmp(other->name, "hdop") != 0 && strcmp(other->name, "vdop") != 0 &&
                   strcmp(other->name, "pdop") != 0 && strcmp(other->name, "ageofdgpsdata") != 0 &&
                   strcmp(other->name, "dgpsid") != 0 && strcmp(other->name, "extensions") != 0) {
                    xmlNewChild(wptnode, NULL, BAD_CAST other->name, BAD_CAST other->value);
                }
            }
        }  
    }

    // Routes
    ListIterator rteIter = createIterator(doc->routes);
	Route* rte;
	while((rte = nextElement(&rteIter)) != NULL) {
        xmlNodePtr rtenode = xmlNewChild(root, NULL, BAD_CAST "rte", NULL);
        // route children
        if(strcmp(rte->name, "") != 0) {
            xmlNewChild(rtenode, NULL, BAD_CAST "name", BAD_CAST rte->name);
        }
        // other route data
        ListIterator otherIter = createIterator(rte->otherData);
        GPXData* other;
        while((other = nextElement(&otherIter)) != NULL) {
            xmlNewChild(rtenode, NULL, BAD_CAST other->name, BAD_CAST other->value);
        }
        // Route waypoints
        ListIterator rteptIter = createIterator(rte->waypoints);
        Waypoint* rtept;
        while((rtept = nextElement(&rteptIter)) != NULL) {
            xmlNodePtr rteptnode = xmlNewChild(rtenode, NULL, BAD_CAST "rtept", NULL);
            // route waypoint attributes
            char *lat = malloc(20);
            sprintf(lat, "%.6lf", rtept->latitude);
            xmlNewProp(rteptnode, BAD_CAST "lat", BAD_CAST lat);
            free(lat);
            char *lon = malloc(20);
            sprintf(lon, "%.6lf", rtept->longitude);
            xmlNewProp(rteptnode, BAD_CAST "lon", BAD_CAST lon);
            free(lon);

            // waypoint children

            add_other_element(rteptnode, "ele", rtept->otherData);
            add_other_element(rteptnode, "time", rtept->otherData);
            add_other_element(rteptnode, "magvar", rtept->otherData);
            add_other_element(rteptnode, "geoidheight", rtept->otherData);
            if(strcmp(rtept->name, "") != 0) {
                xmlNewChild(rteptnode, NULL, BAD_CAST "name", BAD_CAST rtept->name);
            }
            add_other_element(rteptnode, "cmt", rtept->otherData);
            add_other_element(rteptnode, "desc", rtept->otherData);
            add_other_element(rteptnode, "link", rtept->otherData);
            add_other_element(rteptnode, "type", rtept->otherData);
            add_other_element(rteptnode, "fix", rtept->otherData);
            add_other_element(rteptnode, "sat", rtept->otherData);
            add_other_element(rteptnode, "hdop", rtept->otherData);
            add_other_element(rteptnode, "vdop", rtept->otherData);
            add_other_element(rteptnode, "pdop", rtept->otherData);
            add_other_element(rteptnode, "ageofdgpsdata", rtept->otherData);
            add_other_element(rteptnode, "dgpsid", rtept->otherData);
            add_other_element(rteptnode, "extensions", rtept->otherData);  

            {
                ListIterator otherIter = createIterator(rtept->otherData);
                GPXData* other;
                while((other = nextElement(&otherIter)) != NULL) {
                    if(strcmp(other->name, "ele") != 0 && strcmp(other->name, "time") != 0 &&
                       strcmp(other->name, "magvar") != 0 && strcmp(other->name, "geoidheight") != 0 &&
                       strcmp(other->name, "cmt") != 0 && strcmp(other->name, "desc") != 0 &&
                       strcmp(other->name, "link") != 0 && strcmp(other->name, "type") != 0 &&
                       strcmp(other->name, "fix") != 0 && strcmp(other->name, "sat") != 0 &&
                       strcmp(other->name, "hdop") != 0 && strcmp(other->name, "vdop") != 0 &&
                       strcmp(other->name, "pdop") != 0 && strcmp(other->name, "ageofdgpsdata") != 0 &&
                       strcmp(other->name, "dgpsid") != 0 && strcmp(other->name, "extensions") != 0) {
                        xmlNewChild(rteptnode, NULL, BAD_CAST other->name, BAD_CAST other->value);
                    }
                }
            } 
        }
    }

    // Tracks
    ListIterator trkIter = createIterator(doc->tracks);
	Track* trk;
	while((trk = nextElement(&trkIter)) != NULL) {
        xmlNodePtr trknode = xmlNewChild(root, NULL, BAD_CAST "trk", NULL);
        // track children
        if(strcmp(trk->name, "") != 0) {
            xmlNewChild(trknode, NULL, BAD_CAST "name", BAD_CAST trk->name);
        }
        // other track data
        ListIterator otherIter = createIterator(trk->otherData);
        GPXData* other;
        while((other = nextElement(&otherIter)) != NULL) {
            xmlNewChild(trknode, NULL, BAD_CAST other->name, BAD_CAST other->value);
        }
        // Track segments
        ListIterator trksegIter = createIterator(trk->segments);
        TrackSegment* trkseg;
        while((trkseg = nextElement(&trksegIter)) != NULL) {
            xmlNodePtr trksegnode = xmlNewChild(trknode, NULL, BAD_CAST "trkseg", NULL);
            // track points in the segment
            ListIterator trkptIter = createIterator(trkseg->waypoints);
            Waypoint* trkpt;
            while((trkpt = nextElement(&trkptIter)) != NULL) {
                xmlNodePtr trkptnode = xmlNewChild(trksegnode, NULL, BAD_CAST "trkpt", NULL);
                // track point attributes
                char *lat = malloc(20);
                sprintf(lat, "%.6lf", trkpt->latitude);
                xmlNewProp(trkptnode, BAD_CAST "lat", BAD_CAST lat);
                free(lat);
                char *lon = malloc(20);
                sprintf(lon, "%.6lf", trkpt->longitude);
                xmlNewProp(trkptnode, BAD_CAST "lon", BAD_CAST lon);
                free(lon);

                // track point children

                add_other_element(trkptnode, "ele", trkpt->otherData);
                add_other_element(trkptnode, "time", trkpt->otherData);
                add_other_element(trkptnode, "magvar", trkpt->otherData);
                add_other_element(trkptnode, "geoidheight", trkpt->otherData);
                if(strcmp(trkpt->name, "") != 0) {
                    xmlNewChild(trkptnode, NULL, BAD_CAST "name", BAD_CAST trkpt->name);
                }
                add_other_element(trkptnode, "cmt", trkpt->otherData);
                add_other_element(trkptnode, "desc", trkpt->otherData);
                add_other_element(trkptnode, "link", trkpt->otherData);
                add_other_element(trkptnode, "type", trkpt->otherData);
                add_other_element(trkptnode, "fix", trkpt->otherData);
                add_other_element(trkptnode, "sat", trkpt->otherData);
                add_other_element(trkptnode, "hdop", trkpt->otherData);
                add_other_element(trkptnode, "vdop", trkpt->otherData);
                add_other_element(trkptnode, "pdop", trkpt->otherData);
                add_other_element(trkptnode, "ageofdgpsdata", trkpt->otherData);
                add_other_element(trkptnode, "dgpsid", trkpt->otherData);
                add_other_element(trkptnode, "extensions", trkpt->otherData); 

                {
                    ListIterator otherIter = createIterator(trkpt->otherData);
                    GPXData* other;
                    while((other = nextElement(&otherIter)) != NULL) {
                        if(strcmp(other->name, "ele") != 0 && strcmp(other->name, "time") != 0 &&
                           strcmp(other->name, "magvar") != 0 && strcmp(other->name, "geoidheight") != 0 &&
                           strcmp(other->name, "cmt") != 0 && strcmp(other->name, "desc") != 0 &&
                           strcmp(other->name, "link") != 0 && strcmp(other->name, "type") != 0 &&
                           strcmp(other->name, "fix") != 0 && strcmp(other->name, "sat") != 0 &&
                           strcmp(other->name, "hdop") != 0 && strcmp(other->name, "vdop") != 0 &&
                           strcmp(other->name, "pdop") != 0 && strcmp(other->name, "ageofdgpsdata") != 0 &&
                           strcmp(other->name, "dgpsid") != 0 && strcmp(other->name, "extensions") != 0) {
                            xmlNewChild(trkptnode, NULL, BAD_CAST other->name, BAD_CAST other->value);
                        }
                    }
                }
            }
        }
    }
    xmlCleanupParser();
    return xmldoc;
}

bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile) {

    if(doc == NULL) {
        return false;
    }
    if(gpxSchemaFile == NULL || !strcmp(gpxSchemaFile, "")) {
        return false;
    }

    // make sure doc meets the requirements specified in GPXParser.h

    if(doc->creator == NULL || doc->waypoints == NULL || doc->routes == NULL || doc->tracks == NULL) {
        return false;
    }
    if(!strcmp(doc->creator, "") || !strcmp(doc->namespace, "")) {
        return false;
    }

    // Waypoints
    ListIterator wptIter = createIterator(doc->waypoints);
	Waypoint* wpt;
	while((wpt = nextElement(&wptIter)) != NULL) {
        // make sure elements of waypoint are not null
        if(wpt->name == NULL || wpt->otherData == NULL) {
            return false;
        }
        ListIterator otherIter = createIterator(wpt->otherData);
        GPXData* other;
        while((other = nextElement(&otherIter)) != NULL) {
            // make sure elements of other data in waypoints arent empty strings
            if(!strcmp(other->name, "") || !strcmp(other->value, "")) {
                return false;
            }
        }
	}

    // Routes
    ListIterator rteIter = createIterator(doc->routes);
	Route* rte;
	while((rte = nextElement(&rteIter)) != NULL) {
        // make sure elements of route are not null
        if(rte->name == NULL || rte->waypoints == NULL || rte->otherData == NULL) {
            return false;
        }
        // Route waypoints
        ListIterator rteptIter = createIterator(rte->waypoints);
        Waypoint* rtept;
        while((rtept = nextElement(&rteptIter)) != NULL) {
            // make sure elements of route waypoint are not null
            if(rtept->name == NULL || rtept->otherData == NULL) {
                return false;
            }
            ListIterator otherIter = createIterator(rtept->otherData);
            GPXData* other;
            while((other = nextElement(&otherIter)) != NULL) {
                // make sure elements of other data in route waypoints arent empty strings
                if(!strcmp(other->name, "") || !strcmp(other->value, "")) {
                    return false;
                }
            }
        }
        // Route other data
        ListIterator otherIter = createIterator(rte->otherData);
        GPXData* other;
        while((other = nextElement(&otherIter)) != NULL) {
            // make sure elements of other data in route arent empty strings
            if(!strcmp(other->name, "") || !strcmp(other->value, "")) {
                return false;
            }
        }
    }

    // Tracks
    ListIterator trkIter = createIterator(doc->tracks);
	Track* trk;
	while((trk = nextElement(&trkIter)) != NULL) {
        // make sure elements of track are not null
        if(trk->name == NULL || trk->segments == NULL || trk->otherData == NULL) {
            return false;
        }
        // Track segments
        ListIterator trksegIter = createIterator(trk->segments);
        TrackSegment* trkseg;
        while((trkseg = nextElement(&trksegIter)) != NULL) {
            // make sure elements of track segment are not null
            if(trkseg->waypoints == NULL) {
                return false;
            }
            // Track segments waypoints (trkpts)
            ListIterator trkptIter = createIterator(trkseg->waypoints);
            Waypoint* trkpt;
            while((trkpt = nextElement(&trkptIter)) != NULL) {
                // make sure elements of track segment waypoints are not null
                if(trkpt->name == NULL || trkpt->otherData == NULL) {
                    return false;
                }
                ListIterator otherIter = createIterator(trkpt->otherData);
                GPXData* other;
                while((other = nextElement(&otherIter)) != NULL) {
                    // make sure elements of other data in track segment waypoints arent empty strings
                    if(!strcmp(other->name, "") || !strcmp(other->value, "")) {
                        return false;
                    }
                }
            }
        }
        // Track other data
        ListIterator otherIter = createIterator(trk->otherData);
        GPXData* other;
        while((other = nextElement(&otherIter)) != NULL) {
            // make sure elements of other data in route arent empty strings
            if(!strcmp(other->name, "") || !strcmp(other->value, "")) {
                return false;
            }
        }
    }

    xmlDoc *xmldoc = GPXDoctoXMLDoc(doc); 
    if(xmldoc == NULL) {
        return false;
    }

    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt1;

    xmlLineNumbersDefault(1);
    ctxt1 = xmlSchemaNewParserCtxt(gpxSchemaFile);
    xmlSchemaSetParserErrors(ctxt1, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt1);
    if(schema == NULL) {
        xmlSchemaFreeParserCtxt(ctxt1);
        xmlSchemaCleanupTypes();
        xmlFreeDoc(xmldoc);
        xmlCleanupParser();
        return false;
    }
    xmlSchemaFreeParserCtxt(ctxt1);

    xmlSchemaValidCtxtPtr ctxt2;
    ctxt2 = xmlSchemaNewValidCtxt(schema);
    int ret = xmlSchemaValidateDoc(ctxt2, xmldoc);
    if(ret != 0) {
        xmlSchemaFreeValidCtxt(ctxt2);
        if (schema != NULL) {
            xmlSchemaFree(schema);
        }
        xmlSchemaCleanupTypes();
        xmlFreeDoc(xmldoc);
        xmlCleanupParser();
        return false;
    }
    xmlSchemaFreeValidCtxt(ctxt2);
    if (schema != NULL) {
		xmlSchemaFree(schema);
    }
	xmlSchemaCleanupTypes();
    
    xmlFreeDoc(xmldoc);
    xmlCleanupParser();

    return true;
}

bool writeGPXdoc(GPXdoc* doc, char* filename) {
    if(doc == NULL) {
        return false;
    }
    if(filename == NULL || !strcmp(filename, "")) {
        return false;
    }
    int len = strlen(filename);
    if(len < 5) {
        return false;
    }
    if(filename[len-4] != '.' || filename[len-3] != 'g' || filename[len-2] != 'p' || filename[len-1] != 'x') {
        return false;
    }
    xmlDocPtr xmldoc = GPXDoctoXMLDoc(doc);
    int rv = xmlSaveFormatFileEnc(filename, xmldoc, "UTF-8", 1);
    xmlFreeDoc(xmldoc);
    xmlCleanupParser();
    if(rv == -1) {
        return false;
    }
    return true;
}

// ANCHOR Module 2

float lenBetween(Waypoint *wpt1, Waypoint *wpt2) {
    if(wpt1 == NULL || wpt2 == NULL) {
        return 0;
    }
    // convert to radians
    double lat1_r = wpt1->latitude * M_PI / 180;
    double lon1_r = wpt1->longitude * M_PI / 180;
    double lat2_r = wpt2->latitude * M_PI / 180;
    double lon2_r = wpt2->longitude * M_PI / 180;
    // calculate distance
    double R = 6371000;
    double a = pow(sin((lat2_r - lat1_r)/2), 2) + (cos(lat1_r) * cos(lat2_r)) * pow(sin((lon2_r - lon1_r)/2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    float rv = R * c;
    if(rv < 0) {
        rv *= -1;
    }
    return rv;
}

float getRouteLen(const Route *rt) {
    if(rt == NULL) {
        return 0;
    }
    if(getLength(rt->waypoints) < 2) {
        return 0;
    }
    float rte_len = 0;
    ListIterator rteptIter = createIterator(rt->waypoints);
    Waypoint* rtept1 = nextElement(&rteptIter);
    Waypoint* rtept2;
    while((rtept2 = nextElement(&rteptIter)) != NULL) {
        rte_len += lenBetween(rtept1, rtept2);
        rtept1 = rtept2;
    }
    return rte_len;
}

float getTrackLen(const Track *tr) {
    if(tr == NULL) {
        return 0;
    }
    float trk_len = 0;
    ListIterator trksegIter = createIterator(tr->segments);
    TrackSegment* trkseg;
    Waypoint *endpt = NULL;
    while((trkseg = nextElement(&trksegIter)) != NULL) {
        if(getLength(trkseg->waypoints) >= 1) {
            ListIterator trkptIter = createIterator(trkseg->waypoints);
            Waypoint *trkpt1 = nextElement(&trkptIter);
            if(endpt != NULL) {
                trk_len += lenBetween(endpt, trkpt1);
            }
            Waypoint *trkpt2;
            while((trkpt2 = nextElement(&trkptIter)) != NULL) {
                trk_len += lenBetween(trkpt1, trkpt2);
                trkpt1 = trkpt2;
            }
            endpt = trkpt1;
        }
    }
    return trk_len;
}

float round10(float len) {
    return ((((int) len) % 10) < 5) ? (((int) len) / 10) * 10 : ((((int) len) + 5) / 10) * 10;
}

// float round10(float len) {
//     int int_len = (int) len;
//     int r = int_len % 10;
//     int rounded;
//     if(r < 5) { // remainder is less than 5
//         rounded = (int_len / 10) * 10;
//     } else { // remainder is greater or equal to 5
//         rounded = ((int_len + 5) / 10) * 10;
//     }
//     return (float) rounded;
// }

int numRoutesWithLength(const GPXdoc* doc, float len, float delta) {
    if(doc == NULL) {
        return 0;
    }
    int num_routes = 0;
    float max_length = len + delta;
    float min_length = len - delta;
    if(min_length < 0) {
        min_length = 0;
    }
    ListIterator rteIter = createIterator(doc->routes);
	Route* rte;
	while((rte = nextElement(&rteIter)) != NULL) {
        float rtelen = getRouteLen(rte);
        if(rtelen >= min_length && rtelen <= max_length) {
            num_routes++;
        }
    }
    return num_routes;
}

int numTracksWithLength(const GPXdoc* doc, float len, float delta) {
    if(doc == NULL) {
        return 0;
    }
    int num_tracks = 0;
    float max_length = len + delta;
    float min_length = len - delta;
    if(min_length < 0) {
        min_length = 0;
    }
    ListIterator trkIter = createIterator(doc->tracks);
	Track* trk;
	while((trk = nextElement(&trkIter)) != NULL) {
        float trklen = getTrackLen(trk);
        if(trklen >= min_length && trklen <= max_length) {
            num_tracks++;
        }
    }
    return num_tracks;
}

bool isLoopRoute(const Route* route, float delta) {
    if(route == NULL || delta < 0) {
        return false;
    }
    if(getLength(route->waypoints) < 4) {
        return false;
    }
    Waypoint *start = getFromFront(route->waypoints);
    Waypoint *end = getFromBack(route->waypoints);
    if(lenBetween(start, end) <= delta) {
        return true;
    }
    return false;
}
        
bool isLoopTrack(const Track *tr, float delta) {
    if(tr == NULL || delta < 0) {
        return false;
    }
    int total_wpts = 0;
    ListIterator trksegIter = createIterator(tr->segments);
    TrackSegment *firstseg = nextElement(&trksegIter);
    total_wpts += getLength(firstseg->waypoints);
    Waypoint *start = getFromFront(firstseg->waypoints);
    Waypoint *end;
    TrackSegment *prevseg = firstseg;
    TrackSegment *trkseg;
    while((trkseg = nextElement(&trksegIter)) != NULL) {
        int len = getLength(trkseg->waypoints);
        total_wpts += len;
        if(len != 0) {
            prevseg = trkseg;
        }
    }
    end = getFromBack(prevseg->waypoints);
    if(total_wpts < 4) {
        return false;
    }
    if(lenBetween(start, end) <= delta) {
        return true;
    }
    return false;
}

List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {

    if(doc == NULL) {
        return NULL;
    }

    Waypoint start;
    start.latitude = sourceLat;
    start.longitude = sourceLong;

    Waypoint end;
    end.latitude = destLat;
    end.longitude = destLong;

    int num_routes = 0;

    List *routes = initializeList(&routeToString, &fakeDelete, &compareRoutes);

    ListIterator rteIter = createIterator(doc->routes);
	Route* rte;
	while((rte = nextElement(&rteIter)) != NULL) {
        Waypoint *s = getFromFront(rte->waypoints);
        Waypoint *e = getFromBack(rte->waypoints);
        if(s != NULL && e != NULL) {
            if(lenBetween(&start, s) <= delta && lenBetween(&end, e) <= delta) {
                insertBack(routes, rte);
                num_routes++;
            }
        }
    }
    if(num_routes == 0) {
        freeList(routes);
        return NULL;
    }
    return routes;
}

List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {

    if(doc == NULL) {
        return NULL;
    }

    Waypoint start;
    start.latitude = sourceLat;
    start.longitude = sourceLong;

    Waypoint end;
    end.latitude = destLat;
    end.longitude = destLong;

    int num_tracks = 0;

    List *tracks = initializeList(&trackToString, &fakeDelete, &compareTracks);

    ListIterator trkIter = createIterator(doc->tracks);
	Track* trk;
	while((trk = nextElement(&trkIter)) != NULL) {
        TrackSegment *firstseg = getFromFront(trk->segments);
        TrackSegment *lastseg = getFromBack(trk->segments);
        Waypoint *s = getFromFront(firstseg->waypoints);
        Waypoint *e = getFromBack(lastseg->waypoints);
        if(lenBetween(&start, s) <= delta && lenBetween(&end, e) <= delta) {
            insertBack(tracks, trk);
            num_tracks++;
        }
    }
    if(num_tracks == 0) {
        freeList(tracks);
        return NULL;
    }
    return tracks;
}

// ANCHOR Module 3

/* char* trackToJSON(const Track *tr) {
    // format: {"name":"trackName","len":trackLen,"loop":loopStat}
    char *str = malloc(10000);
    if(str == NULL) {
        return NULL;
    }
    if(tr == NULL) {
        sprintf(str, "{}");
        return str;
    }
    sprintf(str, "{\"name\":\"%s\",\"len\":%.1f,\"loop\":%s}",
        !strcmp(tr->name, "") ? "None" : tr->name,
        round10(getTrackLen(tr)),
        isLoopTrack(tr, 10) ? "true" : "false"
    );
    return str;
} */

char* trackToJSON(const Track *tr) {
    // format: {"name":"trackName","numPoints":numVal,"len":trackLen,"loop":loopStat}
    char *str = malloc(10000);
    if(str == NULL) {
        return NULL;
    }
    if(tr == NULL) {
        sprintf(str, "{}");
        return str;
    }
    sprintf(str, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}",
        !strcmp(tr->name, "") ? "None" : tr->name,
        numTrackPoints((Track *) tr),
        round10(getTrackLen(tr)),
        isLoopTrack(tr, 10) ? "true" : "false"
    );
    return str;
}

char* routeToJSON(const Route *rt) {
    // format: {"name":"routeName","numPoints":numVal,"len":routeLen,"loop":loopStat}
    char *str = malloc(10000);
    if(str == NULL) {
        return NULL;
    }
    if(rt == NULL) {
        sprintf(str, "{}");
        return str;
    }
    sprintf(str, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}",
        !strcmp(rt->name, "") ? "None" : rt->name,
        getLength(rt->waypoints),
        round10(getRouteLen(rt)),
        isLoopRoute(rt, 10) ? "true" : "false"
    );
    return str;
}

char* routeListToJSON(const List *list) {
    // format: [RouteString1,RouteString2,...,RouteStringN]
    char *str = malloc(10000);
    if(str == NULL) {
        return NULL;
    }
    sprintf(str, "[");
    if(list != NULL) {
        bool init = true;
        ListIterator rteIter = createIterator((List *) list);
        Route* rte;
        while((rte = nextElement(&rteIter)) != NULL) {
            if(init == true) {
                char *rtestr = routeToJSON(rte);
                strcat(str, rtestr);
                free(rtestr);
                init = false;
            } else {
                char *rtestr = routeToJSON(rte);
                strcat(str, ",");
                strcat(str, rtestr);
                free(rtestr);
            }
        }
    }
    strcat(str, "]");
    return str;
}

char* trackListToJSON(const List *list) {
    // format: [RouteString1,RouteString2,...,RouteStringN]
    char *str = malloc(10000);
    if(str == NULL) {
        return NULL;
    }
    sprintf(str, "[");
    if(list != NULL) {
        bool init = true;
        ListIterator trkIter = createIterator((List *) list);
        Track* trk;
        while((trk = nextElement(&trkIter)) != NULL) {
            if(init == true) {
                char *trkstr = trackToJSON(trk);
                strcat(str, trkstr);
                free(trkstr);
                init = false;
            } else {
                char *trkstr = trackToJSON(trk);
                strcat(str, ",");
                strcat(str, trkstr);
                free(trkstr);
            }
        }
    }
    strcat(str, "]");
    return str;
}

char* GPXtoJSON(const GPXdoc* gpx) {
    // format: {"version":ver,"creator":"crVal","numWaypoints":numW,"numRoutes":numR,"numTracks":numT}
    char *str = malloc(10000);
    if(str == NULL) {
        return NULL;
    }
    if(gpx == NULL) {
        sprintf(str, "{}");
        return str;
    }
    sprintf(str, "{\"version\":%.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}",
        gpx->version,
        gpx->creator,
        getLength(gpx->waypoints),
        getLength(gpx->routes),
        getLength(gpx->tracks)
    );
    return str;
}

// BONUS ///////////////////////////////////////////////////////////////////////////////////////////////////

void addWaypoint(Route *rt, Waypoint *pt) {
    if(rt == NULL || pt == NULL) {
        return;
    }
    insertBack(rt->waypoints, pt);
}

void addRoute(GPXdoc* doc, Route* rt) {
    if(doc == NULL || rt == NULL) {
        return;
    }
    insertBack(doc->routes, rt);
}

GPXdoc* JSONtoGPX(const char* gpxString) {
    // format: {"version":ver,"creator":"creatorValue"}
    if(gpxString == NULL) {
        return NULL;
    }
    GPXdoc *doc = malloc(sizeof(GPXdoc));
    doc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    doc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    doc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    strcpy(doc->namespace, "http://www.topografix.com/GPX/1/1");

    char *str = malloc(strlen(gpxString) + 1);
    strcpy(str, gpxString);
    char d[10] = "{},:\"";
    char *tok = strtok(str, d);
    int i;
    for(i = 0; tok != NULL; i++) {
        if(i == 1) {
            doc->version = atof(tok);
        }
        if(i == 3) {
            doc->creator = malloc(strlen(tok) + 1);
            strcpy(doc->creator, tok);
        }
        tok = strtok(NULL, d);
    }
    free(str);
    return doc;
}

Waypoint* JSONtoWaypoint(const char* gpxString) {
    // format: {"lat":latVal,"lon":lonVal}
    if(gpxString == NULL) {
        return NULL;
    }
    Waypoint *wpt = malloc(sizeof(Waypoint));
    wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    wpt->name = malloc(10);
    strcpy(wpt->name, "");

    char *str = malloc(strlen(gpxString) + 1);
    strcpy(str, gpxString);
    char d[10] = "{},:\"";
    char *tok = strtok(str, d);
    int i;
    for(i = 0; tok != NULL; i++) {
        if(i == 1) {
            wpt->latitude = atof(tok);
        }
        if(i == 3) {
            wpt->longitude = atof(tok);
        }
        tok = strtok(NULL, d);
    }
    free(str);
    return wpt;
}

Route* JSONtoRoute(const char* gpxString) {
    // format: {"name":"nameVal"}
    if(gpxString == NULL) {
        return NULL;
    }
    Route *rte = malloc(sizeof(Route));
    rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    char *str = malloc(strlen(gpxString) + 1);
    strcpy(str, gpxString);
    char d[10] = "{},:\"";
    char *tok = strtok(str, d);
    int i;
    for(i = 0; tok != NULL; i++) {
        if(i == 1) {
            rte->name = malloc(strlen(tok) + 1);
            strcpy(rte->name, tok);
        }
        tok = strtok(NULL, d);
    }
    free(str);
    return rte;
}

// A3 /////////////////////////////////////////////////////////////////////////////////////////////

char* createGPXJSON(char *filename, char *schema) {
    char *path = malloc(200);
    strcpy(path, "./uploads/");
    strcat(path, filename);
    GPXdoc *doc = createValidGPXdoc(path, schema);
    free(path);
    if(doc == NULL) {
        char *str = malloc(10);
        strcpy(str, "{}");
        return str;
    }
    char *json = GPXtoJSON(doc);
    deleteGPXdoc(doc);
    return json;
}

char *createTracklistJSON(char *filename, char *schema) {
    GPXdoc *doc = createValidGPXdoc(filename, schema);
    if(doc == NULL) {
        char *str = malloc(10);
        strcpy(str, "{}");
        return str;
    }
    char *json = trackListToJSON(doc->tracks);
    deleteGPXdoc(doc);
    return json;
}

char *createRoutelistJSON(char *filename, char *schema) {
    GPXdoc *doc = createValidGPXdoc(filename, schema);
    if(doc == NULL) {
        char *str = malloc(10);
        strcpy(str, "{}");
        return str;
    }
    char *json = routeListToJSON(doc->routes);
    deleteGPXdoc(doc);
    return json;
}

int numTrackPoints(Track *trk) {
    int points = 0;
    ListIterator trksegIter = createIterator(trk->segments);
    TrackSegment* trkseg;
    while((trkseg = nextElement(&trksegIter)) != NULL) {
        if(trkseg->waypoints != NULL) {
            points += getLength(trkseg->waypoints);
        }
    }
    return points;
}

char *detailedTrackToJSON(Track *trk) {
    // format: {"name":"trackName","numPoints":numVal,"len":trackLen,"loop":loopStat,"other":{"<name>":"<val>",...}}
    char *str = malloc(10000);
    if(str == NULL) {
        return NULL;
    }
    if(trk == NULL) {
        sprintf(str, "{}");
        return str;
    }
    sprintf(str, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s,\"other\":{",
        !strcmp(trk->name, "") ? "None" : trk->name,
        numTrackPoints((Track *) trk),
        round10(getTrackLen(trk)),
        isLoopTrack(trk, 10) ? "true" : "false"
    );

    int init = 1;

    if(getLength(trk->otherData) > 0) {
        ListIterator otherIter = createIterator(trk->otherData);
        GPXData *other;
        while((other = nextElement(&otherIter)) != NULL) {
            if(init == 1) {
                char *temp = malloc(1000);
                if(temp == NULL) {
                    free(str);
                    return NULL;
                }
                sprintf(temp, "\"%s\":\"%s\"", other->name, other->value);
                strcat(str, temp);
                free(temp);
                init = 0;
            } else {
                char *temp = malloc(1000);
                if(temp == NULL) {
                    free(str);
                    return NULL;
                }
                sprintf(temp, ",\"%s\":\"%s\"", other->name, other->value);
                strcat(str, temp);
                free(temp);
            }
        }
    }
    strcat(str, "}");
    strcat(str, "}");
    int i;
    for(i = 0; str[i] != '\0'; i++) {
        if(str[i] == '\n') {
            str[i] = str[i+1];
        }
    }
    return str;
}

char *getDetailedTrackInfo(char *filename, char *schema, char *name) {
    GPXdoc *doc = createValidGPXdoc(filename, schema);
    if(doc == NULL) {
        char *str = malloc(10);
        strcpy(str, "{}");
        return str;
    }
    Track *trk = getTrack(doc, name);
    char *json = detailedTrackToJSON(trk);
    deleteGPXdoc(doc);
    return json;
}

char *detailedRouteToJSON(Route *rt) {
    // format: {"name":"routeName","numPoints":numVal,"len":routeLen,"loop":loopStat,"other":{"<name>":"<val>",...}}
    char *str = malloc(10000);
    if(str == NULL) {
        return NULL;
    }
    if(rt == NULL) {
        sprintf(str, "{}");
        return str;
    }
    sprintf(str, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s,\"other\":{",
        !strcmp(rt->name, "") ? "None" : rt->name,
        getLength(rt->waypoints),
        round10(getRouteLen(rt)),
        isLoopRoute(rt, 10) ? "true" : "false"
    );

    int init = 1;

    if(getLength(rt->otherData) > 0) {
        ListIterator otherIter = createIterator(rt->otherData);
        GPXData *other;
        while((other = nextElement(&otherIter)) != NULL) {
            if(init == 1) {
                char *temp = malloc(1000);
                if(temp == NULL) {
                    free(str);
                    return NULL;
                }
                sprintf(temp, "\"%s\":\"%s\"", other->name, other->value);
                strcat(str, temp);
                free(temp);
                init = 0;
            } else {
                char *temp = malloc(1000);
                if(temp == NULL) {
                    free(str);
                    return NULL;
                }
                sprintf(temp, ",\"%s\":\"%s\"", other->name, other->value);
                strcat(str, temp);
                free(temp);
            }
        }
    }
    strcat(str, "}");
    strcat(str, "}");
    int i;
    for(i = 0; str[i] != '\0'; i++) {
        if(str[i] == '\n') {
            str[i] = str[i+1];
        }
    }
    return str;
}

char *getDetailedRouteInfo(char *filename, char *schema, char *name) {
    GPXdoc *doc = createValidGPXdoc(filename, schema);
    if(doc == NULL) {
        char *str = malloc(10);
        strcpy(str, "{}");
        return str;
    }
    Route *rte = getRoute(doc, name);
    char *json = detailedRouteToJSON(rte);
    deleteGPXdoc(doc);
    return json;
}

bool validateFilename(char *filename, char *schema) {
    GPXdoc *doc = createValidGPXdoc(filename, schema);
    if(doc == NULL) {
        deleteGPXdoc(doc);
        return false;
    }
    deleteGPXdoc(doc);
    return true;
}

bool jsonToWriteGPXdoc(char *gpxJSON, char *filename, char *schema) {
    int len = strlen(filename);
    if(filename[len-4] != '.' || filename[len-3] != 'g' || filename[len-2] != 'p' || filename[len-1] != 'x') {
        return false;
    }
    GPXdoc *doc = JSONtoGPX(gpxJSON);
    if(validateGPXDoc(doc, schema) == false) {
        return false;
    }
    if(doc == NULL) {
        return false;
    }
    writeGPXdoc(doc, filename);
    free(doc);
    return true;
}

bool changeTrackName(char *filename, char *name, int id) {
    GPXdoc *doc = createGPXdoc(filename);
    if(doc == NULL) {
        return false;
    }
    ListIterator Iter = createIterator(doc->tracks);
    Track *trk;
    int i = 0;
    while((trk = nextElement(&Iter)) != NULL) {
        if(i == id-1) {
            trk->name = realloc(trk->name, strlen(name)+10);
            strcpy(trk->name, name);
            writeGPXdoc(doc, filename);
            deleteGPXdoc(doc);
            return true;
        }
        i++;
        if(i == id) {
            deleteGPXdoc(doc);
            return false;
        }
    }
    return false;
}

bool changeRouteName(char *filename, char *name, int id) {
    GPXdoc *doc = createGPXdoc(filename);
    if(doc == NULL) {
        return false;
    }
    ListIterator Iter = createIterator(doc->routes);
    Route *rte;
    int i = 0;
    while((rte = nextElement(&Iter)) != NULL) {
        if(i == id-1) {
            rte->name = realloc(rte->name, strlen(name)+10);
            strcpy(rte->name, name);
            writeGPXdoc(doc, filename);
            deleteGPXdoc(doc);
            return true;
        }
        i++;
        if(i == id) {
            deleteGPXdoc(doc);
            return false;
        }
    }
    return false;
}

bool addRouteToFile(char *filename, char *routeJSON) {
    GPXdoc *doc = createGPXdoc(filename);
    if(doc == NULL) {
        return false;
    }
    Route *rte = JSONtoRoute(routeJSON);
    if(rte == NULL) {
        deleteGPXdoc(doc);
        return false;
    }
    addRoute(doc, rte);
    writeGPXdoc(doc, filename);
    deleteGPXdoc(doc);
    return true;
}

int filenameToNumRoutes(char *filename) {
    GPXdoc *doc = createGPXdoc(filename);
    if(doc == NULL) {
        return 0;
    }
    int num = getNumRoutes(doc);
    deleteGPXdoc(doc);
    return num;
}

bool addWaypointToRoute(char *filename, int id, char *wptJSON) {
    // format: {"lat":latVal,"lon":lonVal}
    GPXdoc *doc = createGPXdoc(filename);
    if(doc == NULL) {
        return false;
    }
    Waypoint *wpt = JSONtoWaypoint(wptJSON);
    if(wpt == NULL) {
        deleteGPXdoc(doc);
        return false;
    }
    ListIterator Iter = createIterator(doc->routes);
    Route *rte;
    int i = 0;
    while((rte = nextElement(&Iter)) != NULL) {
        if(i == id-1) {
            addWaypoint(rte, wpt);
            writeGPXdoc(doc, filename);
            deleteGPXdoc(doc);
            return true;
        }
        i++;
        if(i == id) {
            deleteGPXdoc(doc);
            return false;
        }
    }
    return false;
}

char *fileToJSONTracksBetween(char *filename, float lat1, float lon1, float lat2, float lon2, float d) {

    GPXdoc *doc = createGPXdoc(filename);
    if(doc == NULL) {
        char *str = malloc(5);
        strcpy(str, "[]");
        return str;
    }
    List *tracks = getTracksBetween(doc, lat1, lon1, lat2, lon2, d);
    if(tracks == NULL) {
        char *str = malloc(5);
        strcpy(str, "[]");
        return str;
    }
    char *json = trackListToJSON(tracks);
    deleteGPXdoc(doc);
    return json;
}

char *fileToJSONRoutesBetween(char *filename, float lat1, float lon1, float lat2, float lon2, float d) {
    GPXdoc *doc = createGPXdoc(filename);
    if(doc == NULL) {
        char *str = malloc(5);
        strcpy(str, "[]");
        return str;
    }
    List *routes = getRoutesBetween(doc, lat1, lon1, lat2, lon2, d);
    if(routes == NULL) {
        char *str = malloc(5);
        strcpy(str, "[]");
        return str;
    }
    char *json = routeListToJSON(routes);
    deleteGPXdoc(doc);
    return json;
}

int fileToGetRoutesOfLength(char *filename, float length) {
    GPXdoc *doc = createGPXdoc(filename);
    if(doc == NULL) {
        return -1;
    }
    float d = 10;
    int rv = numRoutesWithLength(doc, length, d);
    deleteGPXdoc(doc);
    printf("%d\n", rv);
    return rv;
}

int fileToGetTracksOfLength(char *filename, float length) {
    GPXdoc *doc = createGPXdoc(filename);
    if(doc == NULL) {
        return -1;
    }
    float d = 10;
    int rv = numTracksWithLength(doc, length, d);
    deleteGPXdoc(doc);
    printf("%d\n", rv);
    return rv;
}

// A4 Functions ///////////////////////////////////////////////////////////////////////////////////////////

char *WaypointToJSON(const Waypoint* wpt) {
    // format: {"lat":val,"lon":val,"name":"name"}
    char *str = malloc(10000);
    if(str == NULL) {
        return NULL;
    }
    if(wpt == NULL) {
        sprintf(str, "{}");
        return str;
    }
    sprintf(str, "{\"lat\":%.7f,\"lon\":%.7f,\"name\":\"%s\"}",
        wpt->latitude,
        wpt->longitude,
        !strcmp(wpt->name, "") ? "None" : wpt->name
    );
    return str;
}

char *WaypointListToJSON(const List *list) {
    // format: [wptString1,wptString2,...,wptStringN]
    char *str = malloc(10000);
    if(str == NULL) {
        return NULL;
    }
    sprintf(str, "[");
    if(list != NULL) {
        bool init = true;
        ListIterator Iter = createIterator((List *) list);
        Waypoint *wpt;
        while((wpt = nextElement(&Iter)) != NULL) {
            if(init == true) {
                init = false;
                char *s = WaypointToJSON(wpt);
                strcat(str, s);
                free(s);
            } else {
                char *s = WaypointToJSON(wpt);
                strcat(str, ",");
                strcat(str, s);
                free(s);
            }
        }
    }
    strcat(str, "]");
    return str;
}

char *RouteToWptList(char *filename, int routeNum) {
    GPXdoc *doc = createGPXdoc(filename);
    if(doc == NULL) {
        return NULL;
    }
    if(getLength(doc->routes) < routeNum) {
        return NULL;
    }
    ListIterator Iter = createIterator(doc->routes);
    Route *rte;
    int i;
    for(i = 0; i < routeNum; i++) {
        rte = nextElement(&Iter);
    }
    char *JSON = WaypointListToJSON(rte->waypoints);
    return JSON;
}
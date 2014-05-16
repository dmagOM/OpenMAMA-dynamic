#include "example.h"

#include <mama/log.h>

#include <string.h> /* strcmp */

mama_status
examplePlugin_init (mamaPlugin plugin)
{
    printf ( "examplePlugin_init (): "
              "Initialising example plugin.\n" );

    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "examplePlugin_init (): "
              "Initialising example plugin.");
    return MAMA_STATUS_OK;
}

mama_status
examplePlugin_queryClosureData (mamaPlugin plugin,
                                const char *query,
                                void ** closure)
{
    const char* closureData = NULL;

    if (0 == strcmp(query, "pluginOnOpen")) {
        closureData = malloc (22);
        closureData = "Example Plugin - Open";
    } else if (0 == strcmp (query, "pluginOnClose")) {
        closureData = malloc (23);
        closureData = "Example Plugin - Close";
    } else if (0 == strcmp (query, "pluginOnStart")) {
        closureData = malloc (23);
        closureData = "Example Plugin - Start";
    } else if (0 == strcmp (query, "pluginOnStop")) {
        closureData = malloc (22);
        closureData = "Example Plugin - Stop";
    }

    *closure = (void*)closureData;

    return MAMA_STATUS_OK;
}

mama_status
examplePlugin_onOpen (mamaPlugin plugin, void *closure)
{
    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "examplePlugin_onOpen (): "
              "Triggering example plugin.");

     mama_log (MAMA_LOG_LEVEL_NORMAL,
              "examplePlugin_onOpen (): "
              "Closure Data: [%s]", (const char *)closure);

     return MAMA_STATUS_OK;
}

mama_status
examplePlugin_onClose (mamaPlugin plugin, void *closure)
{
    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "examplePlugin_onClose (): "
              "Triggering example plugin.");

    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "examplePlugin_onClose (): "
              "Closure Data: [%s]", (const char *)closure);

    return MAMA_STATUS_OK;
}

mama_status
examplePlugin_onStart (mamaBridge bridge, mamaPlugin plugin, void *closure)
{
    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "examplePlugin_onStart (): "
              "Triggering example plugin.");

    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "examplePlugin_onStart (): "
              "Closure Data: [%s]", (const char *)closure);

   return MAMA_STATUS_OK;
}

mama_status
examplePlugin_onStop (mamaBridge bridge, mamaPlugin plugin, void *closure)
{
    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "examplePlugin_onStop (): "
              "Triggering example plugin.");

    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "examplePlugin_onStop (): "
              "Closure Data: [%s]", (const char *)closure);

   return MAMA_STATUS_OK;
}


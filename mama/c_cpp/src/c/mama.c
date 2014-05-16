/* $Id$
 *
 * OpenMAMA: The open middleware agnostic messaging API
 * Copyright (C) 2011 NYSE Technologies, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "wombat/port.h"
#include "wombat/environment.h"
#include "wombat/strutils.h"
#include <wombat/wtable.h>

#include <mama/mama.h>
#include <mama/error.h>
#include <mamainternal.h>
#include <mama/librarymanager.h>
#include <mama/version.h>
#include <bridge.h>
#include <payloadbridge.h>
#include <property.h>
#include <platform.h>
#include "fileutils.h"
#include "reservedfieldsimpl.h"
#include <mama/statslogger.h>
#include <mama/stat.h>
#include <mama/statfields.h>
#include <statsgeneratorinternal.h>
#include <statsgeneratorinternal.h>
#include <mama/statscollector.h>
#include "transportimpl.h"

#include <baseBridge.h>

#define PROPERTY_FILE "mama.properties"
#define WOMBAT_PATH_ENV "WOMBAT_PATH"
#define MAMA_PROPERTY_BRIDGE "mama.bridge.provider"
#define DEFAULT_STATS_INTERVAL 60

#ifdef WITH_ENTITLEMENTS
#include <OeaClient.h>
#include <OeaStatus.h>
#define SERVERS_PROPERTY "entitlement.servers"
#define MAX_ENTITLEMENT_SERVERS 32
#define MAX_USER_NAME_STR_LEN 64
#define MAX_HOST_NAME_STR_LEN 64

extern void initReservedFields (void);


#if (OEA_MAJVERSION == 2 && OEA_MINVERSION >= 11) || OEA_MAJVERSION > 2

void MAMACALLTYPE entitlementDisconnectCallback (oeaClient*,
                                    const OEA_DISCONNECT_REASON,
                                    const char * const,
                                    const char * const,
                                    const char * const);
void MAMACALLTYPE entitlementUpdatedCallback (oeaClient*,
                                 int openSubscriptionForbidden);
void MAMACALLTYPE entitlementCheckingSwitchCallback (oeaClient*,
                                        int isEntitlementsCheckingDisabled);
#else

void entitlementDisconnectCallback (oeaClient*,
                                    const OEA_DISCONNECT_REASON,
                                    const char * const,
                                    const char * const,
                                    const char * const);
void entitlementUpdatedCallback (oeaClient*,
                                 int openSubscriptionForbidden);
void entitlementCheckingSwitchCallback (oeaClient*,
                                        int isEntitlementsCheckingDisabled);
#endif
oeaClient *               gEntitlementClient = 0;
oeaStatus                 gEntitlementStatus;
mamaEntitlementCallbacks  gEntitlementCallbacks;
static const char*        gServerProperty     = NULL;
static const char*        gServers[MAX_ENTITLEMENT_SERVERS];
static mama_status enableEntitlements (const char **servers);
static const char*        gEntitled = "entitled";
#else
static const char*        gEntitled = "not entitled";
#endif /*WITH_ENTITLEMENTS */

/* Sats Configuration*/
int gLogQueueStats          = 1;
int gLogTransportStats      = 1;
int gLogGlobalStats         = 1;
int gLogLbmStats            = 1;
int gLogUserStats           = 1;

int gGenerateQueueStats     = 0;
int gGenerateTransportStats = 0;
int gGenerateGlobalStats    = 0;
int gGenerateLbmStats       = 0;
int gGenerateUserStats       = 0;

int gPublishQueueStats      = 0;
int gPublishTransportStats  = 0;
int gPublishGlobalStats     = 0;
int gPublishLbmStats        = 0;
int gPublishUserStats       = 0;

static mamaStatsLogger  gStatsPublisher  = NULL;

mamaStatsGenerator      gStatsGenerator         = NULL;
mamaStatsCollector      gGlobalStatsCollector   = NULL;

/* Global Stats */
mamaStat                gInitialStat;
mamaStat                gRecapStat;
mamaStat                gUnknownMsgStat;
mamaStat                gMessageStat;
mamaStat                gFtTakeoverStat;
mamaStat                gSubscriptionStat;
mamaStat                gTimeoutStat;
mamaStat                gWombatMsgsStat;
mamaStat                gFastMsgsStat;
mamaStat                gRvMsgsStat;
mamaStat                gPublisherSend;
mamaStat                gPublisherInboxSend;
mamaStat                gPublisherReplySend;

mama_bool_t gAllowMsgModify = 0;

mama_status mama_statsInit (void);
mama_status mama_setupStatsGenerator (void);

int gCatchCallbackExceptions = 0;

wproperty_t             gProperties      = 0;

static mamaPayloadBridge    gDefaultPayload = NULL;

static wthread_key_t last_err_key;

/**
 * struct mamaApplicationGroup
 * Contains the name of the application and its class name.
 */
typedef struct mamaAppContext_
{
    const char* myApplicationName;
    const char* myApplicationClass;
} mamaApplicationContext;

static mamaImpl gImpl = {
                    NULL,                           /* mBridges          */
                    NULL,                           /* mPayloads         */
                    NULL,                           /* mPayloadIdMap     */
                    NULL,                           /* mPlugins          */
                    NULL,                           /* mFunctionTable    */
                    0,                              /* mInit             */
                    0,                              /* myRefCount        */
                    WSTATIC_MUTEX_INITIALIZER       /* myLock            */
                 };

static mamaApplicationContext  appContext;
static char mama_ver_string[256];


/*  Description :   This function will free any memory associated with a
 *                  mamaApplicationContext object but will not free the
 *                  object itself.
 *  Arguments   :   context [I] The context object to free.
 */
static void
mama_freeAppContext(mamaApplicationContext *context)
{
    /* Only continue if the object is valid. */
    if(context != NULL)
    {
        /* Free all memory */
        if(context->myApplicationName != NULL)
        {
            free((void *)context->myApplicationName);
            context->myApplicationName = NULL;
        }

        if(context->myApplicationClass != NULL)
        {
            free((void *)context->myApplicationClass);
            context->myApplicationClass = NULL;
        }
    }
}

mama_status
mama_setApplicationName (const char* applicationName)
{
    if (appContext.myApplicationName)
    {
        free ((void*)appContext.myApplicationName);
        appContext.myApplicationName = NULL;
    }

    if (applicationName)
    {
        appContext.myApplicationName = strdup (applicationName);
    }
    return MAMA_STATUS_OK;
}

mama_status
mama_setApplicationClassName (const char* className)
{
    if (appContext.myApplicationClass)
    {
        free ((void*)appContext.myApplicationClass);
        appContext.myApplicationClass = NULL;
    }

    if (className)
    {
        appContext.myApplicationClass =  strdup (className);
    }
    return MAMA_STATUS_OK;
}

mama_status
mama_getApplicationName (const char**  applicationName)
{
    if (applicationName == NULL) return MAMA_STATUS_NULL_ARG;
    *applicationName = appContext.myApplicationName;
    return MAMA_STATUS_OK;
}

mama_status
mama_getApplicationClassName (const char** className)
{
    if (className == NULL) return MAMA_STATUS_NULL_ARG;
    *className = appContext.myApplicationClass;
    return MAMA_STATUS_OK;
}

static void
mamaInternal_loadProperties (const char *path,
                             const char *filename)
{
    wproperty_t fileProperties;

    if( gProperties == 0 )
    {
        gProperties = properties_Create ();
    }

    if( !path )
    {
        path = getenv (WOMBAT_PATH_ENV);
        mama_log (MAMA_LOG_LEVEL_NORMAL, "Using path specified in %s",
            WOMBAT_PATH_ENV);
    }

    if( !filename )
    {
       filename = PROPERTY_FILE;
       mama_log (MAMA_LOG_LEVEL_NORMAL,
                 "Using default properties file %s",
                 PROPERTY_FILE);
    }

    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "Attempting to load MAMA properties from %s", path ? path : "");

    fileProperties = properties_Load (path, filename);

    if( fileProperties == 0 )
    {
			mama_log (MAMA_LOG_LEVEL_ERROR, "Failed to open properties file.\n");
        return;
    }

    /* We've got file properties, so we need to merge 'em into
     * anything we've already gotten */
    properties_Merge( fileProperties, gProperties );

    /* Free the file properties, note that FreeEx2 is called to ensure that the data
     * isn't freed as the pointers have been copied over to gProperties.
     */
    properties_FreeEx2(gProperties);
    
   gProperties =  fileProperties;
}

static int mamaInternal_statsPublishingEnabled (void)
{
    return (gPublishGlobalStats
         || gPublishTransportStats
         || gPublishQueueStats
         || gPublishLbmStats
         || gPublishUserStats);
}

static mama_status
mamaInternal_createStatsPublisher (void)
{
    mama_status     result                  = MAMA_STATUS_OK;
    mamaBridge      bridge                  = NULL;
    mamaQueue       queue                   = NULL;
    mamaTransport   statsLogTport           = NULL;
    const char*     userName                = NULL;
    const char*     statsLogTportName       = NULL;
    const char*     statsLogMiddlewareName  = NULL;

    mama_log (MAMA_LOG_LEVEL_NORMAL, "Stats publishing enabled");

    statsLogTportName       = properties_Get (gProperties,
                                              "mama.statslogging.transport");
    statsLogMiddlewareName  = properties_Get (gProperties,
                                              "mama.statslogging.middleware");

    if (!statsLogMiddlewareName)
    {
        statsLogMiddlewareName = "wmw";
    }

    bridge = ((mamaBridgeLib*)
                (wtable_lookup (gImpl.mBridges, 
                                statsLogMiddlewareName)
                ))->bridge;

    if (MAMA_STATUS_OK != (result = mamaBridgeImpl_getInternalEventQueue (bridge,
                                                                          &queue)))
    {
        return result;
    }

    result = mamaStatsLogger_allocate (&gStatsPublisher);

    if( result != MAMA_STATUS_OK )
        return result;

    mama_getUserName (&userName);
    mamaStatsLogger_setReportSize       (gStatsPublisher, 100);
    mamaStatsLogger_setUserName         (gStatsPublisher, userName);
    mamaStatsLogger_setIpAddress        (gStatsPublisher, getIpAddress());
    mamaStatsLogger_setHostName         (gStatsPublisher, getHostName());
    mamaStatsLogger_setApplicationName  (gStatsPublisher,
                                         appContext.myApplicationName);
    mamaStatsLogger_setApplicationClass (gStatsPublisher,
                                         appContext.myApplicationClass);

    mamaStatsLogger_setLogMsgStats (gStatsPublisher, 0);

    if (!statsLogTportName)
    {
        statsLogTportName = "statslogger";
    }

    result = mamaTransportImpl_allocateInternalTransport (&statsLogTport);
    if( result != MAMA_STATUS_OK )
        return result;

    result = mamaTransport_create (statsLogTport,
                                   statsLogTportName,
                                   bridge);
    if (result != MAMA_STATUS_OK)
        return result;

    result = mamaStatsLogger_createForStats (gStatsPublisher,
                                             queue,
                                             statsLogTport,
                                             STATS_TOPIC);
    if (result != MAMA_STATUS_OK)
        return result;

    mama_log (MAMA_LOG_LEVEL_NORMAL, "Stats logging middleware [%s]",
                                    statsLogMiddlewareName ? statsLogMiddlewareName : "");
    mama_log (MAMA_LOG_LEVEL_NORMAL, "Stats logging transport [%s]",
                                    statsLogTportName ? statsLogTportName : "");

    return result;
}

static mama_status
mamaInternal_enableStatsLogging (void)
{
    mama_status     result                  = MAMA_STATUS_OK;
    const char*     statsLogIntervalStr     = NULL;

    mama_log (MAMA_LOG_LEVEL_NORMAL, "Stats logging enabled");

    if (mamaInternal_statsPublishingEnabled())
    {
        if (MAMA_STATUS_OK != (result = mamaInternal_createStatsPublisher ()))
        {
            mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mamaInternal_enableStatsLogging(): "
                      "Could not create stats publisher");
            return result;
        }
    }

    mama_log (MAMA_LOG_LEVEL_NORMAL, "Stats logging interval [%s]",
                                    statsLogIntervalStr ? statsLogIntervalStr : "");
    mama_log (MAMA_LOG_LEVEL_NORMAL, "Stats logging: global stats logging %s",
                                     gGenerateGlobalStats ? "enabled" : "disabled");
    mama_log (MAMA_LOG_LEVEL_NORMAL, "Stats logging: transport stats logging %s",
                                     gGenerateTransportStats ? "enabled" : "disabled");
    mama_log (MAMA_LOG_LEVEL_NORMAL, "Stats logging: queue stats logging %s",
                                     gGenerateQueueStats ? "enabled" : "disabled");


    if (gGenerateGlobalStats)
    {
        const char* appName;
        mama_getApplicationName (&appName);

        if (MAMA_STATUS_OK != (result =
            mamaStatsCollector_create (&gGlobalStatsCollector,
                                       MAMA_STATS_COLLECTOR_TYPE_GLOBAL,
                                       appName,
                                       "-----")))
        {
            return result;
        }

        if (!gLogGlobalStats)
        {
            if (MAMA_STATUS_OK != (result =
                mamaStatsCollector_setLog (gGlobalStatsCollector, 0)))
            {
                return MAMA_STATUS_OK;
            }
        }

        if (gPublishGlobalStats)
        {
            if (MAMA_STATUS_OK != (result =
                mamaStatsCollector_setPublish (gGlobalStatsCollector, 1)))
            {
                return MAMA_STATUS_OK;
            }

            mama_log (MAMA_LOG_LEVEL_NORMAL, "Stats publishing enabled for global stats");
        }

        result = mamaStat_create (&gInitialStat,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatInitials.mName,
                                  MamaStatInitials.mFid);
        if (result != MAMA_STATUS_OK) return result;

        result = mamaStat_create (&gRecapStat,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatRecaps.mName,
                                  MamaStatRecaps.mFid);
        if (result != MAMA_STATUS_OK) return result;

        result = mamaStat_create (&gUnknownMsgStat,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatUnknownMsgs.mName,
                                  MamaStatUnknownMsgs.mFid);
        if (result != MAMA_STATUS_OK) return result;

        result = mamaStat_create (&gMessageStat,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatNumMessages.mName,
                                  MamaStatNumMessages.mFid);
        if (result != MAMA_STATUS_OK) return result;

        result = mamaStat_create (&gFtTakeoverStat,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatFtTakeovers.mName,
                                  MamaStatFtTakeovers.mFid);
        if (result != MAMA_STATUS_OK) return result;

        result = mamaStat_create (&gSubscriptionStat,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatNumSubscriptions.mName,
                                  MamaStatNumSubscriptions.mFid);
        if (result != MAMA_STATUS_OK) return result;

        result = mamaStat_create (&gTimeoutStat,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatTimeouts.mName,
                                  MamaStatTimeouts.mFid);
        if (result != MAMA_STATUS_OK) return result;

        result = mamaStat_create (&gWombatMsgsStat,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatWombatMsgs.mName,
                                  MamaStatWombatMsgs.mFid);
        if (result != MAMA_STATUS_OK) return result;

        result = mamaStat_create (&gFastMsgsStat,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatFastMsgs.mName,
                                  MamaStatFastMsgs.mFid);
        if (result != MAMA_STATUS_OK) return result;

        result = mamaStat_create (&gRvMsgsStat,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatRvMsgs.mName,
                                  MamaStatRvMsgs.mFid);
        if (result != MAMA_STATUS_OK) return result;

        result = mamaStat_create (&gPublisherSend,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatPublisherSend.mName,
                                  MamaStatPublisherSend.mFid);
        if (result != MAMA_STATUS_OK) return result;

        result = mamaStat_create (&gPublisherInboxSend,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatPublisherInboxSend.mName,
                                  MamaStatPublisherInboxSend.mFid);
        if (result != MAMA_STATUS_OK) return result;

        result = mamaStat_create (&gPublisherReplySend,
                                  gGlobalStatsCollector,
                                  MAMA_STAT_LOCKABLE,
                                  MamaStatPublisherReplySend.mName,
                                  MamaStatPublisherReplySend.mFid);
        if (result != MAMA_STATUS_OK) return result;
        mamaStatsGenerator_addStatsCollector (gStatsGenerator, gGlobalStatsCollector);
    }

    if (gLogQueueStats || gLogTransportStats || gLogGlobalStats || gLogLbmStats || gLogUserStats)
    {
        mamaStatsGenerator_setLogStats (gStatsGenerator, 1);
    }
    else
    {
        mamaStatsGenerator_setLogStats (gStatsGenerator, 0);
    }

    if (gStatsPublisher != NULL)
    {
        if (MAMA_STATUS_OK != (result =
                        mamaStatsGenerator_setStatsLogger (gStatsGenerator, &gStatsPublisher)))
        {
            return result;
        }
    }

    return result;
}

mamaStatsGenerator
mamaInternal_getStatsGenerator()
{
    return gStatsGenerator;
}

mamaStatsCollector
mamaInternal_getGlobalStatsCollector()
{
    return gGlobalStatsCollector;
}

/**
 * Expose the property object
 */
wproperty_t
mamaInternal_getProperties()
{
  return gProperties;
}


/****************************************************************
 *  Callbacks used by wtable_for_each in mamaInternal_findBridge
 ****************************************************************/
/* Find the first mamaBridge stored in the gImpl.mBridges variable, and
 * return it.
 */
static void
mamaInternal_getFirstBridgeCb (wtable_t     table,
                               void*        data,
                               const char*  key,
                               void*        closure)
{
    if (*(mamaBridge*)closure == NULL
            && ((mamaBridgeLib*)data)->bridge != NULL)
        *(mamaBridge*)closure = ((mamaBridgeLib*)data)->bridge;
}

/**
 * Iterate through the bridge array and return the first non-NULL value.
 * Note: We actually iterate the full hash table to find the first value. This
 * isn't pretty, but wtable doesn't have a mechanism for dropping out of an
 * iteration at present, and the idea of wtable_getFirst doesn't really make
 * sense anyway.
 */
mamaBridge
mamaInternal_findBridge ()
{
    int middleware = 0;
    mamaBridge bridge = NULL;

    wtable_for_each (gImpl.mBridges, 
                     mamaInternal_getFirstBridgeCb, 
                     (void*)&bridge);

    return bridge;
}

mamaPayloadBridge
mamaInternal_findPayload (char id)
{
    char   idString[2];
    char*  payloadName;
    mamaPayloadLib* payloadLib;

    if ('\0' == id)
        return NULL;

    idString[0] = id;
    idString[1] = '\0';

    payloadName = (char*)wtable_lookup (gImpl.mPayloadIdMap, idString);

    if (NULL == payloadName) {
        mama_log (MAMA_LOG_LEVEL_WARN,
                  "mamaInternal_findPayload (): "
                  "No payload string found for character identifier [%c]",
                  id);
        return NULL;
    }

    payloadLib  = (mamaPayloadLib*)wtable_lookup (gImpl.mPayloads, payloadName);

    if (payloadLib && payloadLib->payload) {
        return payloadLib->payload;
    } else {
        mama_log (MAMA_LOG_LEVEL_FINE,
                  "mamaInternal_findPayload (): "
                  "No payload found for payload character identifier [%c]",
                  id);

        return NULL;
    }
}

mamaPayloadBridge
mamaInternal_getDefaultPayload (void)
{
    return gDefaultPayload;
}

void
mamaInternal_setDefaultPayload (mamaPayloadBridge payloadBridge)
{
    gDefaultPayload = payloadBridge;
}

mama_bool_t
mamaInternal_getAllowMsgModify (void)
{
    return gAllowMsgModify;
}

/****************************************************************
 *  Callbacks used by wtable_for_each in mama_openWithPropertiesCount
 ****************************************************************/
/* Iterates each middleware bridge currently loaded, and logs it's version.
 * Note: This presently does nothing but log the version number, to be
 * compatible with the old code. However, we should probably do something.
 */
static void
mamaInternal_logBridgeVersionsCb (wtable_t      table,
                                  void*         data,
                                  const char*   key,
                                  void*         closure)
{
    mama_log (MAMA_LOG_LEVEL_FINE, 
              ((mamaBridgeLib*)data)->bridge->bridgeGetVersion());
    ++(*(int*)closure);
}

static void
mamaInternal_openDefaultPayloadForLoadedBridgeCb (wtable_t      table,
                                                  void*         data,
                                                  const char*   key,
                                                  void*         closure)
{
    /* TODO: For the love of god man, add some comments...*/
    char** payloadName = NULL;
    char*  payloadId   = NULL;
    int i = 0;

    mamaBridgeLib* bridgeLib = (mamaBridgeLib*)data;
    if (bridgeLib->bridge)
    {
        if (bridgeLib->bridge->bridgeGetDefaultPayloadId (&payloadName,
                                                          &payloadId)
                == MAMA_STATUS_OK)
        {
            while (payloadId[i] != '\0') {
                mamaPayloadBridge payloadImpl;
                char searchId[2];

                searchId[0] = payloadId[i];
                searchId[1] = '\0';

                mama_log (MAMA_LOG_LEVEL_NORMAL, "Loading char: %c", payloadId[i]);

                char* libraryName = (char*)wtable_lookup (gImpl.mPayloadIdMap, searchId);

                if (NULL != libraryName)
                    mama_loadPayloadBridge (&payloadImpl, libraryName);

                i++;
            }
        }
    }
}

static mama_status
mama_openWithPropertiesCount (const char* path,
                              const char* filename,
                              unsigned int* count)
{
    mama_status     result			        = MAMA_STATUS_OK;
    mama_size_t     numBridges              = 0;
    mamaMiddleware  middleware              = 0;
    const char*     appString               = NULL;
    const char*		prop                     = NULL;
    char**			payloadName;
    char*			payloadId;
    int             i                       = 0;

    wthread_static_mutex_lock (&gImpl.myLock);

    if (wthread_key_create(&last_err_key, NULL) != 0)
    {
        mama_log (MAMA_LOG_LEVEL_NORMAL, "WARNING!!! - CANNOT ALLOCATE KEY FOR ERRORS");
    }

    /* If appName is not specified, set a default */
    appString = NULL;
    mama_getApplicationName(&appString);
    if (NULL==appString)
      mama_setApplicationName("MamaApplication");

    /* If appClass is not specified, set a default */
    appString = NULL;
    mama_getApplicationClassName(&appString);
    if (NULL==appString)
      mama_setApplicationClassName("MamaApplications");

#ifdef WITH_CALLBACK_RETURN
    mama_log (MAMA_LOG_LEVEL_WARN,
            "********************************************************");
    mama_log (MAMA_LOG_LEVEL_WARN, "WARNING!!! - In callback return mode."
                                      " Do not release!!!");
    mama_log (MAMA_LOG_LEVEL_WARN,
            "********************************************************");
#endif

#ifdef WITH_UNITTESTS
    mama_log (MAMA_LOG_LEVEL_WARN,
            "********************************************************");
    mama_log (MAMA_LOG_LEVEL_WARN, "WARNING!!! - Built for unittesting."
                                      " Do not release!!!");
    mama_log (MAMA_LOG_LEVEL_WARN,
            "********************************************************");
#endif

    if (0 != gImpl.myRefCount)
    {
        if (MAMA_STATUS_OK == result)
            gImpl.myRefCount++;
        
        if (count)
            *count = gImpl.myRefCount;

        wthread_static_mutex_unlock (&gImpl.myLock);
        return result;
    }
    /* Code after this point is one-time initialization */

#ifdef DEV_RELEASE
    mama_log (MAMA_LOG_LEVEL_WARN,
                "\n********************************************************************************\n"
                "Warning: This is a developer release and has only undergone basic sanity checks.\n"
                "It is for testing only and should not be used in a production environment\n"
                "**********************************************************************************");
#endif /* BAR_RELEASE */

    if (!gProperties)
        mamaInternal_loadProperties (path, filename);

    initReservedFields();
    mama_loginit();

    /* Iterate the loaded middleware bridges, check for their default payloads,
     * and make sure they have been loaded.
     */
    wtable_for_each (gImpl.mBridges,
                     mamaInternal_openDefaultPayloadForLoadedBridgeCb,
                     NULL);

    prop = properties_Get (gProperties, "mama.catchcallbackexceptions.enable");
    if (prop != NULL && strtobool(prop))
    {
        gCatchCallbackExceptions = 1;
    }

    prop = properties_Get (gProperties, "mama.message.allowmodify");
    if (prop && strtobool (prop))
    {
        gAllowMsgModify = 1;
        mama_log (MAMA_LOG_LEVEL_FINE, "mama.message.allowmodify: true");
    }

    mama_log (MAMA_LOG_LEVEL_FINE, "%s (%s)",mama_version, gEntitled);

    wtable_for_each (gImpl.mBridges,
                     mamaInternal_logBridgeVersionsCb,
                     (void*)&numBridges);

    if (0 == numBridges)
    {
        mama_log (MAMA_LOG_LEVEL_SEVERE,
                  "mama_openWithProperties(): "
                  "At least one bridge must be specified");
        if (count)
            *count = gImpl.myRefCount;
        
        wthread_static_mutex_unlock (&gImpl.myLock);
        return MAMA_STATUS_NO_BRIDGE_IMPL;
    }

    if (!gDefaultPayload)
    {
        mama_log (MAMA_LOG_LEVEL_SEVERE,
                  "mama_openWithProperties(): "
                  "At least one payload must be specified");
        if (count)
            *count = gImpl.myRefCount;
        
        wthread_static_mutex_unlock (&gImpl.myLock);
        return MAMA_STATUS_NO_BRIDGE_IMPL;
    }

#ifndef WITH_ENTITLEMENTS
    mama_log (MAMA_LOG_LEVEL_WARN,
                "\n********************************************************************************\n"
                "Note: This build of the MAMA API is not enforcing entitlement checks.\n"
                "Please see the Licensing file for details\n"
                "**********************************************************************************");
#else
    result = enableEntitlements (NULL);
    if (result != MAMA_STATUS_OK)
    {
        mama_log (MAMA_LOG_LEVEL_SEVERE,
                  "mama_openWithProperties(): "
                  "Error connecting to Entitlements Server");
        wthread_static_mutex_unlock (&gImpl.myLock);
        mama_close();
        
        if (count)
            *count = gImpl.myRefCount;

        return result;
    }
#endif /* WITH_ENTITLEMENTS */

    mama_statsInit();


    gImpl.myRefCount++;
    if (count)
        *count = gImpl.myRefCount;

    /* TODO: Need to determine if this is the correct location for firing this
     * event. Also, need to implement the rest of the data. And tidy up the name
     * of the struct, which is long, and error prone.
     * Triggering the mamaPlugin_onOpen plugin functions. */
    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {

        struct mamaPluginStruct_onOpen *plugin
            = &gImpl.mFunctionTable->openCloseFunctions.onMamaOpen[i];

        if (plugin->pluginOnOpen)
            plugin->pluginOnOpen (*plugin->plugin, plugin->closure);

    }

    wthread_static_mutex_unlock (&gImpl.myLock);
    return result;
}

/****************************************************************
 *  Callbacks used by wtable_for_each in mama_openWithPropertiesCount
 ****************************************************************/
/* Callback for mama_statsInit iteration of bridge list.
 * For each bridge found, calls the enableStats method on the 
 * default event queue.
 */
static void
mamaInternal_enableStatsLoggingPerBridgeCb (wtable_t        table,
                                            void*           data,
                                            const char*     key,
                                            void*           closure)
{
    mamaBridgeLib* bridgeLib = (mamaBridgeLib*)data;
    if (bridgeLib->bridge)
    {
        mamaQueue_enableStats (bridgeLib->bridge->mDefaultEventQueue);
    }
}

mama_status
mama_statsInit (void)
    {
    mamaMiddleware middleware = 0;
    char* statsLogging = (char*) properties_Get (gProperties, "mama.statslogging.enable");

    if ( (statsLogging != NULL) && strtobool (statsLogging))
    {
        const char*     propVal           = NULL;

        /* If logging has been explicitly set false, and publishing is also set false, then don't
           generate stats (and neither log nor publish). */

        propVal           = properties_Get (gProperties, "mama.statslogging.global.logging");
        if ( propVal != NULL)
            gLogGlobalStats = strtobool(propVal);

        propVal           = properties_Get (gProperties, "mama.statslogging.global.publishing");
        if ( propVal != NULL)
            gPublishGlobalStats = strtobool(propVal);

        propVal           = properties_Get (gProperties, "mama.statslogging.transport.logging");
        if ( propVal != NULL)
            gLogTransportStats = strtobool(propVal);

        propVal           = properties_Get (gProperties, "mama.statslogging.transport.publishing");
        if ( propVal != NULL)
            gPublishTransportStats = strtobool(propVal);

        propVal           = properties_Get (gProperties, "mama.statslogging.queue.logging");
        if ( propVal != NULL)
            gLogQueueStats = strtobool(propVal);

        propVal           = properties_Get (gProperties, "mama.statslogging.queue.publishing");
        if ( propVal != NULL)
            gPublishQueueStats = strtobool(propVal);

        propVal           = properties_Get (gProperties, "mama.statslogging.lbm.logging");
        if ( propVal != NULL)
            gLogLbmStats = strtobool(propVal);

        propVal           = properties_Get (gProperties, "mama.statslogging.lbm.publishing");
        if ( propVal != NULL)
            gPublishLbmStats = strtobool(propVal);

        propVal           = properties_Get (gProperties, "mama.statslogging.user.logging");
        if ( propVal != NULL)
            gLogUserStats = strtobool(propVal);

        propVal           = properties_Get (gProperties, "mama.statslogging.user.publishing");
        if ( propVal != NULL)
            gPublishUserStats = strtobool(propVal);
        
        if (gLogGlobalStats || gPublishGlobalStats) gGenerateGlobalStats=1;
        if (gLogTransportStats || gPublishTransportStats) gGenerateTransportStats=1;
        if (gLogQueueStats || gPublishQueueStats) gGenerateQueueStats=1;
        if (gLogLbmStats || gPublishLbmStats) gGenerateLbmStats=1;
        if (gLogUserStats || gPublishUserStats) gGenerateUserStats=1;
          
        mama_setupStatsGenerator();

        mamaInternal_enableStatsLogging();

        /* Iterate each bridge, and call the enableStats on the default bridge
         * for each.
         */
        wtable_for_each (gImpl.mBridges,
                         mamaInternal_enableStatsLoggingPerBridgeCb,
                         NULL);
    }

    return MAMA_STATUS_OK;
}

mama_status
mama_setupStatsGenerator (void)
{
	mama_status result = MAMA_STATUS_OK;

	unsigned int* 	count				= NULL;
	const char* 	statsIntervalStr 	= NULL;
	mamaQueue       statsGenQueue       = NULL;

	statsIntervalStr = properties_Get (gProperties, "mama.statslogging.interval");

	if (MAMA_STATUS_OK != (result = mamaStatsGenerator_create(
	                                &gStatsGenerator,
	                                statsIntervalStr ? atof (statsIntervalStr) : DEFAULT_STATS_INTERVAL)))
	{
		mama_log (MAMA_LOG_LEVEL_ERROR,
				  "mama_openWithProperties(): "
				  "Could not create stats generator.");
		if (count)
			*count = gImpl.myRefCount;

		wthread_static_mutex_unlock (&gImpl.myLock);
		return result;
	}
        /* No publishing, therefore no middleware needs to be specified
           in mama.properties.  Instead, check through loaded bridges */
        if (!mamaInternal_statsPublishingEnabled())
        {
            mamaBridgeImpl* impl = (mamaBridgeImpl*) mamaInternal_findBridge ();

            if (impl != NULL)
            {
                statsGenQueue = impl->mDefaultEventQueue;
            }
        }
        else
        {
            /* Stats publishing enabled, therefore use the mama.statslogging.middleware
               property */
            mamaBridge bridge;

            const char* statsMiddleware = NULL;
            statsMiddleware = properties_Get (gProperties, "mama.statslogging.middleware");

            if (!statsMiddleware)
            {
                statsMiddleware = "wmw";
            }

		    mama_loadBridge(&bridge, statsMiddleware);

            if (MAMA_STATUS_OK != (result = mamaBridgeImpl_getInternalEventQueue (bridge,
                                                               &statsGenQueue)))
            {
                if (count)
                    *count = gImpl.myRefCount;
                wthread_static_mutex_unlock (&gImpl.myLock);
                return result;
            }
        }

        if (MAMA_STATUS_OK != (result = mamaStatsGenerator_setQueue (gStatsGenerator, statsGenQueue)))
        {
            mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mama_openWithProperties(): "
                      "Could not set queue for stats generator.");
            if (count)
                *count = gImpl.myRefCount;
            wthread_static_mutex_unlock (&gImpl.myLock);
            return result;
        }

	mamaStatsGenerator_setLogStats (gStatsGenerator, 1);

    return result;
}

mama_status
mama_open ()
{
    /*Passing NULL as path and filename will result in the
     default behaviour - mama.properties on $WOMBAT_PATH*/
    return mama_openWithPropertiesCount (NULL, NULL, NULL);
}

mama_status
mama_openWithProperties (const char* path,
                         const char* filename)
{
    return mama_openWithPropertiesCount (path, filename, NULL);
}

mama_status
mama_setProperty (const char* name,
                  const char* value)
{
    if (!gProperties)
    {
       mamaInternal_loadProperties( NULL, NULL );
    }

    if (!name||!value)
        return MAMA_STATUS_NULL_ARG;

    mama_log (MAMA_LOG_LEVEL_NORMAL,"Setting property: %s with value: %s",
                                   name, value);

    if( 0 == properties_setProperty (gProperties,
                                     name,
                                     value))
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
            "mama_setProperty(): Failed to set property");
        return MAMA_STATUS_PLATFORM;
    }
    return MAMA_STATUS_OK;
}

mama_status
mama_setPropertiesFromFile (const char *path,
                            const char *filename)
{
    wproperty_t fileProperties;

    if (!gProperties)
    {
       mamaInternal_loadProperties( NULL, NULL );
    }

    if( !path )
    {
        path = environment_getVariable(WOMBAT_PATH_ENV);
        mama_log (MAMA_LOG_LEVEL_NORMAL, "Using path specified in %s",
            WOMBAT_PATH_ENV);
    }

    if( !filename )
        return MAMA_STATUS_NULL_ARG;

    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "Attempting to load additional MAMA properties from %s", path ? path : "");

    fileProperties = properties_Load (path, filename);

    if( fileProperties == 0 )
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "Failed to open additional properties file.\n");
        return MAMA_STATUS_IO_ERROR;
    }

    /* We've got file properties, so we need to merge 'em into
     * anything we've already gotten */
    properties_Merge( fileProperties, gProperties );

    /* Free the file properties, note that FreeEx2 is called to ensure that the data
     * isn't freed as the pointers have been copied over to gProperties.
     */
    properties_FreeEx2(gProperties);
    
    gProperties = fileProperties;

    return MAMA_STATUS_OK;
}

const char *
mama_getProperty (const char *name)
{
    if( !gProperties )
    {
        mamaInternal_loadProperties( NULL, NULL );
    }

    if( !name )
        return NULL;

    return properties_Get( gProperties, name );
}

const char*
mama_getVersion (mamaBridge bridgeImpl)
{
    mamaBridgeImpl* impl =  (mamaBridgeImpl*)bridgeImpl;

    if (!impl)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mama_getVersion(): No bridge implementation specified");
        return NULL;
    }

    /*Delegate the call to the bridge specific implementation*/
    snprintf(mama_ver_string,sizeof(mama_ver_string),"%s (%s) (%s)",
             mama_version, impl->bridgeGetVersion (), gEntitled);

    return mama_ver_string;
}

/****************************************************************
 *  Callbacks used by wtable_for_each in mama_closeCount
 ****************************************************************/
/*
 * Stops the internal event queue of each bridge.
 */
static void
mamaInternal_stopEachBridgeEventQueueCB (wtable_t    table,
                                         void*       data,
                                         const char* key,
                                         void*       closure)
{
    mamaBridgeLib* bridgeLib = (mamaBridgeLib*)data;

    if (bridgeLib->bridge)
        mamaBridgeImpl_stopInternalEventQueue (bridgeLib->bridge);
}

/*
 * For each payload bridge in the table, first set the payload pointer to NULL,
 * then close the shared library, and set it to NULL as well.
 */
static void
mamaInternal_closePayloadAndLibraryCb (wtable_t         table,
                                       void*            data,
                                       const char*      key,
                                       void*            closure)
{
    mamaPayloadLib* payloadLib = (mamaPayloadLib *)data;
    int i = 0;

    /* TODO: Need to determine when to free the memory allocated for the
     * mamaPayloadBridge struct. If we take ownership of allocation, it
     * should probably be freed here.
     */
    if (payloadLib->payload) {
        free (payloadLib->payload);
        payloadLib->payload = NULL;
    }

    if(payloadLib->library) {
#ifndef MAMA_DEBUG
        closeSharedLib (payloadLib->library);
#endif
        payloadLib->library = NULL;
    }

    free (payloadLib);
    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i)
    {
        if (gImpl.mFunctionTable->libraryLoadFunctions.onPayloadUnload[i])
            gImpl.mFunctionTable->libraryLoadFunctions.onPayloadUnload[i](key);
    }
}

/*
 * For each plugin in the table, first set the plugin pointer to NULL,
 * then close the shared library, and set it to NULL as well.
 */
static void
mamaInternal_closePluginAndLibraryCb (wtable_t         table,
                                       void*            data,
                                       const char*      key,
                                       void*            closure)
{
    mamaPluginLib* pluginLib = (mamaPluginLib *)data;
    int i = 0;

    /* TODO: Need to determine when to free the memory allocated for the
     * mamaPlugin struct. Since we allocate it, it should probably be here.
     */
    if (pluginLib->plugin) {
        free (pluginLib->plugin);
        pluginLib->plugin = NULL;
    }

    if(pluginLib->library) {
#ifndef MAMA_DEBUG
        closeSharedLib (pluginLib->library);
#endif
        pluginLib->library = NULL;
    }

    free (pluginLib);
    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i)
    {
        if (gImpl.mFunctionTable->libraryLoadFunctions.onPluginUnload[i])
            gImpl.mFunctionTable->libraryLoadFunctions.onPluginUnload[i](key);
    }
}


/*
 * For each middleware bridge in the table, first run bridgeClose, set the
 * bridge pointer to NULL, then close the shared library, and set it to
 * NULL as well.
 */
static void
mamaInternal_closeBridgeAndLibraryCb (wtable_t      table,
                                      void*         data,
                                      const char*   key,
                                      void*         closure)
{
    mama_status status = MAMA_STATUS_OK;
    mamaBridgeLib* bridgeLib = (mamaBridgeLib*)data;
    int i = 0;

    if (bridgeLib->bridge)
    {
        status = bridgeLib->bridge->bridgeClose (bridgeLib->bridge);
        if (MAMA_STATUS_OK != status)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mama_close(): Error closing %s bridge.",
                      key);
        }
        /* TODO: The check here needs to be made much more complete. */
        if (bridgeLib->bridge->bridgeProperties) {
            /* TODO: We need to decide when to free the underlying bridge
             * impl. Legacy bridges expect to own it, but in the new system we
             * will.
             */
            wlock_destroy (bridgeLib->bridge->mLock);

           /* If the properties object exists, this is a new bridge, and
             * we can free this...
             */
            free (bridgeLib->bridge);
        }
        bridgeLib->bridge = NULL;
    }

    if (bridgeLib->library)
    {
#ifndef MAMA_DEBUG
        closeSharedLib (bridgeLib->library);
#endif
        bridgeLib->library = NULL;
    }

    free (bridgeLib);
    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i)
    {
        if (gImpl.mFunctionTable->libraryLoadFunctions.onBridgeUnload[i])
            gImpl.mFunctionTable->libraryLoadFunctions.onBridgeUnload[i](key);
    }
}

/* Free the data associated with the payloadIdMap table. */
static void
mamaInternal_freePayloadIdMapCb (wtable_t       table,
                                void*           data,
                                const char*     key,
                                void*           closure)
{
    if (data)
        free (data);
}

static mama_status
mama_closeCount (unsigned int* count)
{
    mama_status    result     = MAMA_STATUS_OK;
    mamaMiddleware middleware = 0;
    int payload = 0;
    int i       = 0;

    wthread_static_mutex_lock (&gImpl.myLock);
    if (gImpl.myRefCount == 0)
    {
        if (count)
            *count = gImpl.myRefCount;
        wthread_static_mutex_unlock (&gImpl.myLock);
        return MAMA_STATUS_OK;
    }

    /* TODO: Need to determine if this is the correct location for firing this
     * event. Also, need to implement the rest of the data. And tidy up the name
     * of the struct, which is long, and error prone.
     * Triggering the mamaPlugin_onClose plugin functions. */
    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {
        struct mamaPluginStruct_onClose *plugin
            = &gImpl.mFunctionTable->openCloseFunctions.onMamaClose[i];

        if (plugin->pluginOnClose)
            plugin->pluginOnClose (*plugin->plugin, plugin->closure);
    }

    if (!--gImpl.myRefCount)
    {
#ifdef WITH_ENTITLEMENTS
        if( gEntitlementClient != 0 )
        {
            oeaClient_destroy( gEntitlementClient );
            gEntitlementClient = 0;
        }
#endif /* WITH_ENTITLEMENTS */

        wthread_key_delete(last_err_key);

        if (gStatsGenerator)
        {
            mamaStatsGenerator_stopReportTimer(gStatsGenerator);
        }

        wtable_for_each (gImpl.mBridges,
                         mamaInternal_stopEachBridgeEventQueueCB,
                         NULL);

        if (gInitialStat)
        {
            mamaStat_destroy (gInitialStat);
            gInitialStat = NULL;
        }

        if (gRecapStat)
        {
            mamaStat_destroy (gRecapStat);
            gRecapStat = NULL;
        }

        if (gUnknownMsgStat)
        {
            mamaStat_destroy (gUnknownMsgStat);
            gUnknownMsgStat = NULL;
        }

        if (gMessageStat)
        {
            mamaStat_destroy (gMessageStat);
            gMessageStat = NULL;
        }

        if (gFtTakeoverStat)
        {
            mamaStat_destroy (gFtTakeoverStat);
            gFtTakeoverStat = NULL;
        }

        if (gSubscriptionStat)
        {
            mamaStat_destroy (gSubscriptionStat);
            gSubscriptionStat = NULL;
        }

        if (gTimeoutStat)
        {
            mamaStat_destroy (gTimeoutStat);
            gTimeoutStat = NULL;
        }

        if (gWombatMsgsStat)
        {
            mamaStat_destroy (gWombatMsgsStat);
            gWombatMsgsStat = NULL;
        }

        if (gFastMsgsStat)
        {
            mamaStat_destroy (gFastMsgsStat);
            gFastMsgsStat = NULL;
        }

        if (gRvMsgsStat)
        {
            mamaStat_destroy (gRvMsgsStat);
            gRvMsgsStat = NULL;
        }

        if (gPublisherSend)
        {
            mamaStat_destroy (gPublisherSend);
            gPublisherSend = NULL;
        }

        if (gPublisherInboxSend)
        {
            mamaStat_destroy (gPublisherInboxSend);
            gPublisherInboxSend = NULL;
        }

        if (gPublisherReplySend)
        {
            mamaStat_destroy (gPublisherReplySend);
            gPublisherReplySend = NULL;
        }
        if (gGlobalStatsCollector)
        {
            if (gStatsGenerator)
            {
                mamaStatsGenerator_removeStatsCollector (gStatsGenerator, gGlobalStatsCollector);
            }
            mamaStatsCollector_destroy (gGlobalStatsCollector);
            gGlobalStatsCollector = NULL;
        }

        if (gStatsPublisher)
        {
            mamaStatsLogger_destroy (gStatsPublisher);
            gStatsPublisher = NULL;
        }

                /* Destroy the stats generator after the bridge is closed so we will
           have removed the default queue stats collector */
        if (gStatsGenerator)
        {
            mamaStatsGenerator_destroy (gStatsGenerator);
            gStatsGenerator = NULL;
        }

        cleanupReservedFields();

        /* Iterate over the payloads which have been loaded, and unload each
         * library in turn.
         */
        wtable_clear_for_each (gImpl.mPayloads,
                                mamaInternal_closePayloadAndLibraryCb,
                                NULL);
        gDefaultPayload = NULL;

        /* Iterate over each of the loaded middleware bridges, and unload each
         * library in turn.
         */
        wtable_clear_for_each (gImpl.mBridges, 
                               mamaInternal_closeBridgeAndLibraryCb,
                               NULL);

        /* Iterate over each of the plugin which have been loaded, and unload
         * each library in turn.
         */
        wtable_clear_for_each (gImpl.mPlugins,
                               mamaInternal_closePluginAndLibraryCb,
                               NULL);

        /* Ok, so there is a small issue here. If we free this now, we have to
         * repopulate when open is called. Which is silly. Need a better way, but
         * also not sure there's much point at this stage. Therefore, removing
         * and introducing a small leak...
         * TODO: Fix this...
         */
        /*
        wtable_clear_for_each (gImpl.mPayloadIdMap,
                               mamaInternal_freePayloadIdMapCb,
                               NULL);
        */

        /* We also need to free the mFunctionTable structure, which should only
         * be done once all the othe bridges/payloads etc have been unloaded.
         */
        free(gImpl.mFunctionTable);
        gImpl.mFunctionTable = NULL;

        gImpl.mInit = 0;

        /* The properties must not be closed down until after the bridges have been destroyed. */
        if (gProperties != 0)
        {
            properties_Free (gProperties);
            gProperties = 0;
        }

        /* Destroy logging */
        mama_logDestroy();

        /* Free application context details. */
        mama_freeAppContext(&appContext);

    }
    if (count)
        *count = gImpl.myRefCount;

    wthread_static_mutex_unlock (&gImpl.myLock);
    return result;
}

mama_status
mama_close (void)
{
    return mama_closeCount (NULL);
}

/**
 * Start processing messages.
 */
mama_status
mama_start (mamaBridge bridgeImpl)
{
    mamaBridgeImpl* impl =  (mamaBridgeImpl*)bridgeImpl;
    mama_status rval = MAMA_STATUS_OK;
    unsigned int prevRefCnt = 0;
    int i = 0;

    if (!impl)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mama_start(): No bridge implementation specified");
        return MAMA_STATUS_NO_BRIDGE_IMPL;
    }

    if (!impl->mDefaultEventQueue)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "mama_start(): NULL default queue. "
                  "Has mama_open() been called?");
        return MAMA_STATUS_INVALID_QUEUE;
    }

    /* TODO: Need to determine if this is the correct location for firing this
     * event. Also, need to implement the rest of the data. And tidy up the name
     * of the struct, which is long, and error prone.
     * Triggering the mamaPlugin_onStart plugin functions. */
    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {

        struct mamaPluginStruct_onStart *plugin
            = &gImpl.mFunctionTable->openCloseFunctions.onMamaStart[i];

        if (plugin->pluginOnStart)
            plugin->pluginOnStart (bridgeImpl, *plugin->plugin, plugin->closure);

    }

    wthread_static_mutex_lock(&gImpl.myLock);
    prevRefCnt = impl->mRefCount++;
    wthread_static_mutex_unlock(&gImpl.myLock);

    if (prevRefCnt > 0)
        return MAMA_STATUS_OK;

    /* Delegate to the bridge specific implementation */
    /* Can't hold lock because bridgeStart blocks */
    rval =  impl->bridgeStart (impl->mDefaultEventQueue);

    if (rval != MAMA_STATUS_OK)
    {
        wthread_static_mutex_lock(&gImpl.myLock);
        impl->mRefCount--;
        wthread_static_mutex_unlock(&gImpl.myLock);
    }

    return rval;
}

struct startBackgroundClosure
{
    mamaStopCB   mStopCallback;
    mamaStopCBEx mStopCallbackEx;
    mamaBridge   mBridgeImpl;
    void*        mClosure;
};

static void* mamaStartThread (void* closure)
{
    struct startBackgroundClosure* cb =
                (struct startBackgroundClosure*)closure;
    mama_status rval = MAMA_STATUS_OK;

    if (!cb) return NULL;

    rval = mama_start (cb->mBridgeImpl);

    if (cb->mStopCallback)
        cb->mStopCallback (rval);

    if (cb->mStopCallbackEx)
        cb->mStopCallbackEx (rval, cb->mBridgeImpl, cb->mClosure);

    /* Free the closure object */
    free(cb);

    return NULL;
}

static mama_status
mama_startBackgroundHelper (mamaBridge   bridgeImpl,
                            mamaStopCB   callback,
                            mamaStopCBEx exCallback,
                            void*        closure)
{
    struct startBackgroundClosure*  closureData;
    wthread_t       t = 0;

    if (!bridgeImpl)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "mama_startBackgroundHelper(): NULL bridge "
                  " impl.");
        return MAMA_STATUS_NO_BRIDGE_IMPL;
    }

    if (!callback && !exCallback)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "mama_startBackgroundHelper(): No "
                  "stop callback or extended stop callback specified.");
        return MAMA_STATUS_INVALID_ARG;
    }

    if (callback && exCallback)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "mama_startBackgroundHelper(): Both "
                "stop callback and extended stop callback specified.");
        return MAMA_STATUS_INVALID_ARG;
    }

    closureData = calloc (1, (sizeof (struct startBackgroundClosure)));

    if (!closureData)
        return MAMA_STATUS_NOMEM;

    closureData->mStopCallback   = callback;
    closureData->mStopCallbackEx = exCallback;
    closureData->mBridgeImpl    = bridgeImpl;
    closureData->mClosure        = closure;

    if (0 != wthread_create(&t, NULL, mamaStartThread, (void*) closureData))
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "Could not start background MAMA "
                  "thread.");
        return MAMA_STATUS_SYSTEM_ERROR;
    }

    return MAMA_STATUS_OK;
}

mama_status
mama_startBackground (mamaBridge bridgeImpl, mamaStartCB callback)
{
    /* Passing these NULLs tells mama_startBackgroundHelper to use old functionality */
    return mama_startBackgroundHelper (bridgeImpl, (mamaStopCB)callback, NULL, NULL);
}

mama_status
mama_startBackgroundEx (mamaBridge bridgeImpl, mamaStopCBEx exCallback, void* closure)
{
    /* Passing this NULL tells mama_StartBackgroundHelper to use new functionality */
    return mama_startBackgroundHelper (bridgeImpl, NULL, exCallback, closure);
}
/**
 * Stop processing messages
 */
mama_status
mama_stop (mamaBridge bridgeImpl)
{
    mamaBridgeImpl* impl =  (mamaBridgeImpl*)bridgeImpl;
    mama_status rval = MAMA_STATUS_OK;
    int         i    = 0;

    if (!impl)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mama_stop(): No bridge implementation specified");
        return MAMA_STATUS_NO_BRIDGE_IMPL;
    }

    if (!impl->mDefaultEventQueue)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mama_stop(): NULL default queue. Has mama_open() been "
                  "called?");
        return MAMA_STATUS_INVALID_QUEUE;
    }

    /*Delegate to the bridge specific implementation*/
    wthread_static_mutex_lock(&gImpl.myLock);
    if (impl->mRefCount > 0)
    {
        impl->mRefCount--;
        if(impl->mRefCount == 0)
        {
            rval = impl->bridgeStop (impl->mDefaultEventQueue);
            if (MAMA_STATUS_OK != rval)
                impl->mRefCount++;
        }
    }
    wthread_static_mutex_unlock(&gImpl.myLock);

    /* TODO: Need to determine if this is the correct location for firing this
     * event. Also, need to implement the rest of the data. And tidy up the name
     * of the struct, which is long, and error prone.
     * Triggering the mamaPlugin_onStart plugin functions. */
    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {

        struct mamaPluginStruct_onStop *plugin
            = &gImpl.mFunctionTable->openCloseFunctions.onMamaStop[i];

        if (plugin->pluginOnStop)
            plugin->pluginOnStop (bridgeImpl, *plugin->plugin, plugin->closure);

    }

    return rval;
}

/****************************************************************
 *  Callback used by wtable_for_each in mama_stopAll
 ****************************************************************/
static void
mamaInternal_stopAllBridgeCb (wtable_t      table,
                              void*         data,
                              const char*   key,
                              void*         closure)
{
    mama_status status = MAMA_STATUS_OK;
    mamaBridgeLib* bridgeLib = (mamaBridgeLib*)data;

    if (bridgeLib->bridge)
    {
        status = mama_stop (bridgeLib->bridge);
        if (MAMA_STATUS_OK != status)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mama_stopAll(): Error stopping %s bridge.",
                      key);

            /* We're doing this to maintain consistency with the previous
             * implementation, though there are issues - multiple failures
             * will still only pass back the error status of the last one.
             */
            *(mama_status*)closure = status;
        }
    }
}

/**
 * Stops all the bridges.
 */
mama_status
mama_stopAll (void)
{
    mama_status status = MAMA_STATUS_OK;

    wtable_for_each (gImpl.mBridges,
                     mamaInternal_stopAllBridgeCb,
                     (void*)&status);

    return status;
}

mama_status
mama_getDefaultEventQueue (mamaBridge bridgeImpl,
                           mamaQueue* defaultQueue)
{
    mamaBridgeImpl* impl =  (mamaBridgeImpl*)bridgeImpl;

    if (!impl)
    {
        mama_log (MAMA_LOG_LEVEL_WARN, "mama_getDefaultEventQueue(): "
                  "No bridge implementation specified");
        return MAMA_STATUS_NO_BRIDGE_IMPL;
    }

    if (!impl->mDefaultEventQueue)
    {
        mama_log (MAMA_LOG_LEVEL_WARN, "mama_getDefaultEventQueue (): "
                  "NULL default queue for bridge impl. Has mama_open() been "
                  "called?");
        return MAMA_STATUS_INVALID_QUEUE;
    }

    *defaultQueue = impl->mDefaultEventQueue;
    return MAMA_STATUS_OK;
}

mama_status
mama_getUserName (const char** userName)
{
    if (userName == NULL)
        return MAMA_STATUS_NULL_ARG;

    *userName = getlogin();
    return MAMA_STATUS_OK;
}

mama_status
mama_getHostName (const char** hostName)
{
    if (hostName == NULL) return MAMA_STATUS_NULL_ARG;
    *hostName = getHostName();
    return MAMA_STATUS_OK;
}

mama_status
mama_getIpAddress (const char** ipAddress)
{
    if (ipAddress == NULL) return MAMA_STATUS_NULL_ARG;
    *ipAddress = getIpAddress();
    return MAMA_STATUS_OK;
}

#ifdef WITH_ENTITLEMENTS

mama_status
mama_registerEntitlementCallbacks (const mamaEntitlementCallbacks* entitlementCallbacks)
{
    if (entitlementCallbacks == NULL) return MAMA_STATUS_NULL_ARG;
    gEntitlementCallbacks = *entitlementCallbacks;
    return MAMA_STATUS_OK;
}

#if (OEA_MAJVERSION == 2 && OEA_MINVERSION >= 11) || OEA_MAJVERSION > 2
void MAMACALLTYPE entitlementDisconnectCallback (oeaClient*                  client,
                                    const OEA_DISCONNECT_REASON reason,
                                    const char * const          userId,
                                    const char * const          host,
                                    const char * const          appName)
{
    if (gEntitlementCallbacks.onSessionDisconnect != NULL)
    {
        gEntitlementCallbacks.onSessionDisconnect (reason, userId, host, appName);
    }
}

void MAMACALLTYPE entitlementUpdatedCallback (oeaClient* client,
                                 int openSubscriptionForbidden)
{
    if (gEntitlementCallbacks.onEntitlementUpdate != NULL)
    {
        gEntitlementCallbacks.onEntitlementUpdate();
    }
}

void MAMACALLTYPE entitlementCheckingSwitchCallback (oeaClient* client,
                                        int isEntitlementsCheckingDisabled)
{
    if (gEntitlementCallbacks.onEntitlementCheckingSwitch != NULL)
    {
        gEntitlementCallbacks.onEntitlementCheckingSwitch(isEntitlementsCheckingDisabled);
    }
}

#else

void entitlementDisconnectCallback (oeaClient*                  client,
                                    const OEA_DISCONNECT_REASON reason,
                                    const char * const          userId,
                                    const char * const          host,
                                    const char * const          appName)
{
    if (gEntitlementCallbacks.onSessionDisconnect != NULL)
    {
        gEntitlementCallbacks.onSessionDisconnect (reason, userId, host, appName);
    }
}

void entitlementUpdatedCallback (oeaClient* client,
                                 int openSubscriptionForbidden)
{
    if (gEntitlementCallbacks.onEntitlementUpdate != NULL)
    {
        gEntitlementCallbacks.onEntitlementUpdate();
    }
}

void entitlementCheckingSwitchCallback (oeaClient* client,
                                        int isEntitlementsCheckingDisabled)
{
    if (gEntitlementCallbacks.onEntitlementCheckingSwitch != NULL)
    {
        gEntitlementCallbacks.onEntitlementCheckingSwitch(isEntitlementsCheckingDisabled);
    }
}

#endif


const char **
mdrvImpl_ParseServersProperty()
{
    char *ptr;
    int idx = 0;

    if (gServerProperty == NULL)
    {
        memset (gServers, 0, sizeof(gServers));

        if( properties_Get (gProperties, SERVERS_PROPERTY) == NULL)
        {
            if (gMamaLogLevel)
            {
                mama_log( MAMA_LOG_LEVEL_WARN,
                          "Failed to open properties file "
                          "or no entitlement.servers property." );
            }
            return NULL;
        }

        gServerProperty = strdup (properties_Get (gProperties,
                                                  SERVERS_PROPERTY));

        if (gMamaLogLevel)
        {
            mama_log (MAMA_LOG_LEVEL_NORMAL,
                      "entitlement.servers=%s",
                      gServerProperty == NULL ? "NULL" : gServerProperty);
        }

        while( idx < MAX_ENTITLEMENT_SERVERS - 1 )
        {
            gServers[idx] = strtok_r (idx == 0 ? (char *)gServerProperty : NULL
                                      , ",",
                                      &ptr);


            if (gServers[idx++] == NULL) /* last server parsed */
            {
                break;
            }

            if (gMamaLogLevel)
            {
                mama_log (MAMA_LOG_LEVEL_NORMAL,
                          "Parsed entitlement server: %s",
                          gServers[idx-1]);
            }
        }
    }
    return gServers;
}

static mama_status
enableEntitlements (const char **servers)
{
    int size = 0;
    const char* portLowStr = NULL;
    const char* portHighStr = NULL;
    int portLow = 8000;
    int portHigh = 8001;
    oeaCallbacks entitlementCallbacks;
    const char* altUserId;
    const char* altIp;
    const char* site;


    if (gEntitlementClient != 0)
    {
        oeaClient_destroy (gEntitlementClient);
        gEntitlementClient = 0;
    }

    if (servers == NULL)
    {
        if (NULL == (servers = mdrvImpl_ParseServersProperty()))
        {
            return MAMA_ENTITLE_NO_SERVERS_SPECIFIED;
        }
    }

    while (servers[size] != NULL)
    {
        size = size + 1;
    }

    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "Attempting to connect to entitlement server");

    portLowStr  = properties_Get (gProperties, "mama.entitlement.portlow");
    portHighStr = properties_Get (gProperties, "mama.entitlement.porthigh");

    /*properties_Get returns NULL if property does not exist, in which case
      we just use defaults*/
    if (portLowStr != NULL)
    {
        portLow  = (int)atof(portLowStr);
    }

    if (portHighStr != NULL)
    {
        portHigh = (int)atof(portHighStr);
    }

    altUserId   = properties_Get (gProperties, "mama.entitlement.altuserid");
    site = properties_Get (gProperties, "mama.entitlement.site");
    altIp = properties_Get (gProperties, "mama.entitlement.effective_ip_address");
    entitlementCallbacks.onDisconnect = entitlementDisconnectCallback;
    entitlementCallbacks.onEntitlementsUpdated = entitlementUpdatedCallback;
    entitlementCallbacks.onSwitchEntitlementsChecking = entitlementCheckingSwitchCallback;

    gEntitlementClient = oeaClient_create(&gEntitlementStatus,
                                site,
                                portLow,
                                portHigh,
                                servers,
                                size);

    if (gEntitlementStatus != OEA_STATUS_OK)
    {
        return gEntitlementStatus + MAMA_STATUS_BASE;
    }

    if (gEntitlementClient != 0)
    {
        if (OEA_STATUS_OK != (gEntitlementStatus = oeaClient_setCallbacks (gEntitlementClient, &entitlementCallbacks)))
        {
            return gEntitlementStatus + MAMA_STATUS_BASE;
        }

        if (OEA_STATUS_OK != (gEntitlementStatus = oeaClient_setAlternativeUserId (gEntitlementClient, altUserId)))
        {
            return gEntitlementStatus + MAMA_STATUS_BASE;
        }

        if (OEA_STATUS_OK != (gEntitlementStatus = oeaClient_setEffectiveIpAddress (gEntitlementClient, altIp)))
        {
            return gEntitlementStatus + MAMA_STATUS_BASE;
        }

        if (OEA_STATUS_OK != (gEntitlementStatus = oeaClient_setApplicationId (gEntitlementClient, appContext.myApplicationName)))
        {
            return gEntitlementStatus + MAMA_STATUS_BASE;
        }

        if (OEA_STATUS_OK != (gEntitlementStatus = oeaClient_downloadEntitlements ((oeaClient*const)gEntitlementClient)))
        {
            return gEntitlementStatus + MAMA_STATUS_BASE;
        }
    }

    return MAMA_STATUS_OK;
}

#endif

void
mamaInternal_registerBridge (mamaBridge     bridge,
                             const char*    middlewareName)
{
    /* NOTE: With the wtable implementation, we have no need to register the
     * bridge at this point. Therefore, this function has become a no-op.
     */
}



mama_status
mama_setDefaultPayload (char id)
{
    char payloadId[2];

    if ('\0' == id )
        return MAMA_STATUS_NULL_ARG;

    payloadId[0] = id;
    payloadId[1] = '\0';

    mamaPayloadLib* payloadLib = wtable_lookup (gImpl.mPayloads, payloadId);

    if (!payloadLib || !payloadLib->payload)
        return MAMA_STATUS_NULL_ARG;

    gDefaultPayload = payloadLib->payload;

    return MAMA_STATUS_OK;
}

int
mamaInternal_generateLbmStats ()
{
	return gGenerateLbmStats;
}

mama_status
mama_loadBridge (mamaBridge* impl,
                 const char* middlewareName)
{
    mamaInternal_init ();
    return mamaLibraryManager_loadLibrary (middlewareName,
                                           NULL,
                                           (void*)impl,
                                           MAMA_MIDDLEWARE_LIBRARY);
}

mama_status
mama_loadBridgeWithPath (mamaBridge* impl,
                         const char* middlewareName,
                         const char* path)
{
    mamaInternal_init ();
    return mamaLibraryManager_loadLibrary (middlewareName,
                                           path,
                                           (void*)impl,
                                           MAMA_MIDDLEWARE_LIBRARY);
}

mama_status
mama_loadPayloadBridge (mamaPayloadBridge* impl,
                         const char*        payloadName)
{
    mamaInternal_init ();
    return mamaLibraryManager_loadLibrary (payloadName,
                                           NULL,
                                           (void*)impl,
                                           MAMA_PAYLOAD_LIBRARY);
}

mama_status
mama_loadPayloadBridgeWithPath (mamaPayloadBridge* impl,
                                const char*        payloadName,
                                const char*        path)
{
    mamaInternal_init ();
    return mamaLibraryManager_loadLibrary (payloadName,
                                           path,
                                           (void*)impl,
                                           MAMA_PAYLOAD_LIBRARY);
}

mama_status
mama_loadPlugin (mamaPlugin* impl,
                 const char* pluginName)
{
    mamaInternal_init ();
    return mamaLibraryManager_loadLibrary (pluginName,
                                           NULL,
                                           (void*)impl,
                                           MAMA_PLUGIN_LIBRARY);
}

mama_status
mama_loadPluginWithPath (mamaPlugin* impl,
                         const char* pluginName,
                         const char* path)
{
    mamaInternal_init ();
    return mamaLibraryManager_loadLibrary (pluginName,
                                           path,
                                           (void*)impl,
                                           MAMA_PLUGIN_LIBRARY);
}


/*
 * Function pointer type for calling getVersion in the wrapper
 */
typedef const char* (MAMACALLTYPE *fpWrapperGetVersion)(void);

static fpWrapperGetVersion wrapperGetVersion = NULL;

MAMAExpDLL
void
mama_setWrapperGetVersion(fpWrapperGetVersion value)
{
	wrapperGetVersion = value;
}

/**
* Exposes the property for whether or not the test classes need to be used for catching callback exceptions
*/
int
mamaInternal_getCatchCallbackExceptions (void)
{
	return gCatchCallbackExceptions;
}


/* Do not expose in the public headers */
const char*
mama_wrapperGetVersion(mamaBridge bridge)
{
  if (wrapperGetVersion)
	  /* use getVersion from wrapper */
	  return wrapperGetVersion();
  else
	  /* Use native getVersion */
	  return mama_getVersion(bridge);
}

void
mama_setLastError (mamaError error)
{
     wthread_setspecific(last_err_key, (void*)error);
}

mamaError
mama_getLastErrorCode (void)
{
    return (mamaError)wthread_getspecific(last_err_key);
}

const char*
mama_getLastErrorText (void)
{
    return mamaError_convertToString((mamaError)wthread_getspecific(last_err_key));
}


mama_status
mama_setBridgeInfoCallback (mamaBridge bridgeImpl, bridgeInfoCallback callback)
{
    mamaBridgeImpl* impl = (mamaBridgeImpl*)bridgeImpl;

    switch (mamaMiddleware_convertFromString (impl->bridgeGetName()))
    {
        case MAMA_MIDDLEWARE_LBM    :   impl->mBridgeInfoCallback = callback;
                                        return MAMA_STATUS_OK;
        case MAMA_MIDDLEWARE_TIBRV  :
        case MAMA_MIDDLEWARE_WMW    :
        default                     :   return MAMA_STATUS_NOT_IMPLEMENTED;
    }

    return MAMA_STATUS_OK;
}

mama_status
mama_addStatsCollector (mamaStatsCollector statsCollector)
{
    mama_status status = MAMA_STATUS_NOT_FOUND;

    if (!gStatsGenerator)
        mama_statsInit();

    if (MAMA_STATUS_OK != (
            status = mamaStatsGenerator_addStatsCollector (
                        gStatsGenerator,
                        statsCollector)))
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "mama_addStatsCollector (): "
                    "Could not add User stats collector.");
        return status;
    }

    return status;
}

mama_status
mama_removeStatsCollector (mamaStatsCollector statsCollector)
{
    mama_status status = MAMA_STATUS_NOT_FOUND;

    if (gStatsGenerator)
    {
        if (MAMA_STATUS_OK != (
                status = mamaStatsGenerator_removeStatsCollector (
                            mamaInternal_getStatsGenerator(),
                            statsCollector)))
        {
            mama_log (MAMA_LOG_LEVEL_ERROR, "mama_removeStatsCollector (): "
                     "Could not remove User stats collector.");
            return status;
        }
    }
    else
    {
        mama_log (MAMA_LOG_LEVEL_ERROR, "mamaInternal_getStatsGenerator (): "
                  "Could not find stats generator.");
    }
    return status;
}

static void
mamaInternal_loadPropertyToPayloadIdMapCb (const char* name,
                                           const char* value,
                                           void* closure)
{
    /* For a first pass, we expect the mapping to be
     * payloadName=payloadIDChar
     * TODO: Need to add lots more checking into this - it shouldn't break
     * if we add something silly, it just won't work right either.
     */
    wtable_t       table       = (wtable_t)closure;
    const char*    payloadName = strdup(name);

    mama_log (MAMA_LOG_LEVEL_FINE, 
              "Adding Payload name to ID mapping [%s]:[%s]",
              payloadName,
              value);

    wtable_insert (table, value, (void*)payloadName);
}

#define PAYLOAD_ID_MAP_SIZE 15
#define MIDDLEWARE_TABLE_SIZE 5
#define PAYLOAD_TABLE_SIZE 15
#define PLUGIN_TABLE_SIZE 25

/* Initialises a number of structures required by OpenMAMA:
 * - Properties
 * - wTable instances
 * - Payload ID Map
 */
mama_status
mamaInternal_init (void)
{
    /* Using the full lock for this initial check is overkill, and the ideal
     * would be to use wInterlocked, but then we have a chicken and egg
     * problem with initialization, so the lock it is.
     */
    wthread_static_mutex_lock (&gImpl.myLock);
    if (!gImpl.mInit)
    {
        /* Begin by loading the payload to ID mappings, so we can query these
         * later.
         * Note: It's safe to use mamaInternal_loadPropteries() in this instance
         * because subsequent calls will simply merge into what we load now.
         */
        if (!gImpl.mPayloadIdMap)
        {
            gImpl.mPayloadIdMap = wtable_create ("PayloadIdMap",
                                                 PAYLOAD_ID_MAP_SIZE);
        }
        mamaInternal_loadProperties (NULL, "mama.payloads");

        properties_ForEach (gProperties,
                            mamaInternal_loadPropertyToPayloadIdMapCb,
                            (void*)gImpl.mPayloadIdMap);

        /* Then load the main mama.properties file. This means we can
         * actually use properties defined within it to find anything else we
         * need
         */
        mamaInternal_loadProperties (NULL, NULL);

        /* Initialize the middlewares wtable 
         * Using 5 for the size is purely speculative, but it's hard to imagine
         * anyone using more.
         */
        if (!gImpl.mBridges)
        {
            gImpl.mBridges = wtable_create ("Middlewares",
                                            MIDDLEWARE_TABLE_SIZE);
        }

        /* Initialize the payloads wtable.
         * Using 15 for the size is purely speculative, but it's hard to imagine
         * anyone using more.
         */
        if (!gImpl.mPayloads)
        {
            gImpl.mPayloads = wtable_create ("Payloads",
                                             PAYLOAD_TABLE_SIZE);
        }

        /* Initialize the plugins wtable 
         * Again, size is speculative, but more than 25 seems unlikely.
         */
        if (!gImpl.mPlugins)
        {
            gImpl.mPlugins = wtable_create ("Plugins", 
                                            PLUGIN_TABLE_SIZE);
        }

        gImpl.mInit = 1;

        /* Allocate the function table structure, which is used to hold the
         * various bridge/payload/plugin loading callbacks, as well as the
         * plugin function pointer structures themselves.
         */
        if (!gImpl.mFunctionTable)
        {
            mamaLibraryManager_allocateFunctionTable ();
        }
    }
    wthread_static_mutex_unlock (&gImpl.myLock);

    return MAMA_STATUS_OK;
}

/**
 * Returns a pointer to the global mamaImpl structure, to allow it's use by
 * functions which live outside the core mama.c code file.
 */
mamaImpl*
mamaInternal_getGlobalImpl (void)
{
    return &gImpl;
}

/**
 * Comparison function for MAMA versions.
 *
 * TODO: This needs to be implemented fully to, you know, do something...
 */
int
mamaInternal_compareMamaVersion (const char *version)
{
    if (NULL == version)
        return -1;

    if ( 0 == strcmp (version, "2.4.0") )
        return 0;

    return -1;
}

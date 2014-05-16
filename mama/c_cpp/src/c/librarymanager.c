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

#include "librarymanagerinternal.h"
#include "mama/librarymanager.h"

#include <mama/mama.h>

#include "mamainternal.h" /* gImpl access */

/**
 * Contains function pointer definitions for middleware, payload and plugins, as
 * we as default implementations for each.
 */
#include "baseBridge.h"
#include "bridge.h"
#include "payloadbridge.h"
#include "plugin.h"

#include <wombat/port.h>
#include <wombat/wtable.h>
#include <platform.h>

#define WTABLE_SUCCESS 1

/*********************************************************
 * Static Implementaion Functions
 *********************************************************/
static void
mamaLibraryManager_getWtableSize (wtable_t   table,
                                  void       *data,
                                  const char *key,
                                  void       *closure)
{
    (*(mama_size_t*)closure)++;
}

static void
mamaLibraryManager_getMiddlewareNamesCb (wtable_t   table,
                                     void       *data,
                                     const char *key,
                                     void       *closure)
{
    const char** array = *(const char***)closure;
    int i = 0;

    while(NULL != array[i])
        i++;

    array[i] = strdup(((mamaBridgeLib*)data)->name);
}

static void
mamaLibraryManager_getPayloadNamesCb (wtable_t   table,
                                     void       *data,
                                     const char *key,
                                     void       *closure)
{
    const char** array = *(const char***)closure;
    int i = 0;

    while(NULL != array[i])
        i++;

    array[i] = strdup(((mamaPayloadLib*)data)->name);
}

static void
mamaLibraryManager_getPluginNamesCb (wtable_t   table,
                                     void       *data,
                                     const char *key,
                                     void       *closure)
{
    const char** array = *(const char***)closure;
    int i = 0;

    while(NULL != array[i])
        i++;

    array[i] = strdup(((mamaPluginLib*)data)->name);
}


/*********************************************************
 * Public Functions
 *********************************************************/
/**
 * @brief Method to return an array of the names of the loaded middlewares.
 *
 * Functions queries the OpenMAMA library manager for the names of the currently
 * loaded middleware libraries, populating an array with the names and returning
 * them to the caller, along with the size of the array.
 *
 * Note: The string representations of the names are owned by the caller, and
 * thus must be freed after use.
 *
 * @param[out] middlewares A pointer to an array of nul-terminated strings of
 *             middleware names.
 * @param[out] size The size of the returned array.
 *
 * @return A mama_status denoting the success or failure of the call.
 *         MAMA_STATUS_OK indicates the call was successful.
 *         MAMA_STATUS_NOMEM indicates an issue allocating memeory for the array.
 */
mama_status
mamaLibraryManager_listLoadedMiddlewares (const char  ***middlewares,
                                          mama_size_t *size)
{
    mama_status status      = MAMA_STATUS_OK;
    mamaImpl    *gImpl      = mamaInternal_getGlobalImpl ();

    if (NULL == middlewares || NULL == size) {
        status = MAMA_STATUS_NULL_ARG;
        goto exit;
    }

    wtable_for_each (gImpl->mBridges,
                     mamaLibraryManager_getWtableSize,
                     (void*)size);

    *middlewares = calloc (*size, sizeof(char*));

    if (NULL == *middlewares) {
        status = MAMA_STATUS_NOMEM;
        goto exit;
    }

    wtable_for_each (gImpl->mBridges,
                     mamaLibraryManager_getMiddlewareNamesCb,
                     (void*)middlewares);

exit:
    return status;
}

/**
 * @brief Method to return an array of the names of the loaded payloads.
 *
 * Functions queries the OpenMAMA library manager for the names of the currently
 * loaded payload libraries, populating an array with the names and returning
 * them to the caller, along with the size of the array.
 *
 * Note: The string representations of the names are owned by the caller, and
 * thus must be freed after use.
 *
 * @param[out] payloads A pointer to an array of null-terminated strings of
 *             payload names.
 * @param[out] size The size of the returned array.
 *
 * @return A mama_status denoting the success or failure of the call.
 *         MAMA_STATUS_OK indicates the call was successful.
 *         MAMA_STATUS_NOMEM indicates an issue allocating memeory for the array.
 */
mama_status
mamaLibraryManager_listLoadedPayloads (const char  ***payloads,
                                       mama_size_t *size)
{
    mama_status status = MAMA_STATUS_OK;
    mamaImpl    *gImpl = mamaInternal_getGlobalImpl ();

    if (NULL == payloads || NULL == size) {
        status = MAMA_STATUS_NULL_ARG;
        goto exit;
    }

    wtable_for_each (gImpl->mPayloads,
                     mamaLibraryManager_getWtableSize,
                     (void*)size);

    *payloads = calloc (*size, sizeof(char*));

    if (NULL == *payloads) {
        status = MAMA_STATUS_NOMEM;
        goto exit;
    }

    wtable_for_each (gImpl->mPayloads,
                     mamaLibraryManager_getPayloadNamesCb,
                     (void*)payloads);

exit:
    return status;
}

/**
 * @brief Method to return an array of the names of the loaded plugins.
 *
 * Functions queries the OpenMAMA library manager for the names of the currently
 * loaded plugin libraries, populating an array with the names and returning
 * them to the caller, along with the size of the array.
 *
 * Note: The string representations of the names are owned by the caller, and
 * thus must be freed after use.
 *
 * @param[out] plugins A pointer to an array of null-terminated strings of
 *             plugin names.
 * @param[out] size The size of the returned array.
 *
 * @return A mama_status denoting the success or failure of the call.
 *         MAMA_STATUS_OK indicates the call was successful.
 *         MAMA_STATUS_NOMEM indicates an issue allocating memeory for the array.
 */
mama_status
mamaLibraryManager_listLoadedPlugins (const char  ***plugins,
                                      mama_size_t *size)
{
    mama_status status = MAMA_STATUS_OK;
    mamaImpl    *gImpl      = mamaInternal_getGlobalImpl ();

    if (NULL == plugins || NULL == size) {
        status = MAMA_STATUS_NULL_ARG;
        goto exit;
    }

    wtable_for_each (gImpl->mPlugins,
                     mamaLibraryManager_getWtableSize,
                     (void*)size);

    *plugins = calloc (*size, sizeof(char*));

    if (NULL == *plugins) {
        status = MAMA_STATUS_NOMEM;
        goto exit;
    }

    wtable_for_each (gImpl->mPlugins,
                     mamaLibraryManager_getPluginNamesCb,
                     (void*)plugins);

exit:
    return status;
}

/**
 * @brief Method to query the manager for a specific middleware.
 *
 * Function queries the OpenMAMA library manager for a middleware which matches
 * the passed name, and returns it in the given bridge parameter.
 *
 * @param[in]  middlewareName The name of the middleware being requested.
 * @param[out] bridge         Pointer to the middleware bridge struct to be
 *             returned.
 *
 * @result A mama_status denoting the success or failure of the call.
 *         MAMA_STATUS_OK indicates success.
 *         MAMA_STATUS_NULL_ARG indicates a NULL argument was passed to the method
 *         MAMA_STATUS_NO_BRIDGE_IMPL indicates no loaded bridge with that name
 *         was found.
 */
mama_status
mamaLibraryManager_getMiddleware (const char *middlewareName,
                                  mamaBridge *bridge)
{
    mama_status   status     = MAMA_STATUS_OK;
    mamaImpl      *gImpl     = mamaInternal_getGlobalImpl ();
    mamaBridgeLib *bridgeLib = NULL;

    if (NULL == middlewareName || NULL == bridge) {
        status = MAMA_STATUS_NULL_ARG;
        goto exit;
    }

    bridgeLib = (mamaBridgeLib*)wtable_lookup (gImpl->mBridges, middlewareName);

    if (NULL == bridgeLib) {
        status = MAMA_STATUS_NO_BRIDGE_IMPL;
        goto exit;
    }

    *bridge = bridgeLib->bridge;

exit:
    return status;
}

/**
 * @brief Method to query the manager for a specific payload.
 *
 * Function queries the OpenMAMA library manager for a payload which matches
 * the passed name, and returns it in the given payload parameter.
 *
 * @param[in]  payloadName The name of the middleware being requested.
 * @param[out] payload     Pointer to the payload bridge struct to be returned.
 *
 * @result A mama_status denoting the success or failure of the call.
 *         MAMA_STATUS_OK indicates success.
 *         MAMA_STATUS_NULL_ARG indicates a NULL argument was passed to the method
 *         MAMA_STATUS_NO_BRIDGE_IMPL indicates no loaded bridge with that name
 *         was found.
 */
mama_status
mamaLibraryManager_getPayload (const char        *payloadName,
                               mamaPayloadBridge *payload)
{
    mama_status    status      = MAMA_STATUS_OK;
    mamaImpl       *gImpl      = mamaInternal_getGlobalImpl ();
    mamaPayloadLib *payloadLib = NULL;

    if (NULL == payloadName || NULL == payload) {
        status = MAMA_STATUS_NULL_ARG;
        goto exit;
    }

    payloadLib = (mamaPayloadLib*)wtable_lookup (gImpl->mPayloads, payloadName);

    if (NULL == payloadLib) {
        status = MAMA_STATUS_NO_BRIDGE_IMPL;
        goto exit;
    }

    *payload = payloadLib->payload;

exit:
    return status;
}

/**
 * @brief Method to query the manager for a specific middleware.
 *
 * Function queries the OpenMAMA library manager for a middleware which matches
 * the passed name, and returns it in the given bridge parameter.
 *
 * @param[in]  pluginName The name of the middleware being requested.
 * @param[out] plugin     Pointer to the plugin bridge struct to be returned.
 *
 * @result A mama_status denoting the success or failure of the call.
 *         MAMA_STATUS_OK indicates success.
 *         MAMA_STATUS_NULL_ARG indicates a NULL argument was passed to the method
 *         MAMA_STATUS_NO_BRIDGE_IMPL indicates no loaded bridge with that name
 *         was found.
 */
mama_status
mamaLibraryManager_getPlugin (const char *pluginName,
                              mamaPlugin *plugin)
{
    mama_status   status     = MAMA_STATUS_OK;
    mamaImpl      *gImpl     = mamaInternal_getGlobalImpl ();
    mamaPluginLib *pluginLib = NULL;

    if (NULL == pluginName || NULL == plugin) {
        status = MAMA_STATUS_NULL_ARG;
        goto exit;
    }

    pluginLib = (mamaPluginLib*)wtable_lookup (gImpl->mPlugins, pluginName);

    if (NULL == pluginLib) {
        status = MAMA_STATUS_NO_BRIDGE_IMPL;
        goto exit;
    }

    *plugin = pluginLib->plugin;

exit:
    return status;
}


/********************************************
 * Internal Functions: Registration
 ********************************************/

/**
 * @brief Mechanism for registering required bridge functions.
 *
 * Taking a function string name search the shared library handle, using the
 * loadLibFunc portability method, for the function. If it is found, set the
 * appropriate function pointer in the bridge struct to the result. If not
 * set the pointer to 0xDEADBEEF (so it shows up really clearly in stacktraces.
 *
 * @param FUNCSTRINGNAME The string function name.
 * @param FUNCIMPLNAME The name of the function pointer in the bridge struct
 * @param FUNCIMPLTYPE The type of the function pointer expected.
 *
 * TODO: If the function is not found, an error should be registered, and bridge
 * loading should be stopped.
 */
#define REGISTER_BRIDGE_FUNCTION(FUNCSTRINGNAME, FUNCIMPLNAME, FUNCIMPLTYPE)    \
do {                                                                            \
    snprintf (functionName, 256, "%s"#FUNCSTRINGNAME, name);                    \
    result = loadLibFunc (bridgeLib, functionName);                             \
                                                                                \
    if (NULL != result) {                                                       \
        (*bridge)->FUNCIMPLNAME = (FUNCIMPLTYPE)result;                      \
        result = NULL;                                                          \
    } else {                                                                    \
        (*bridge)->FUNCIMPLNAME = (FUNCIMPLTYPE)0xDEADBEEF;                     \
    }                                                                           \
} while (0)

/**
 * @brief Mechanism for registering optional bridge functions, and setting a
 * default.
 *
 * Taking a function string name search the shared library handle, using the
 * loadLibFunc portability method, for the function. If it is found, set the
 * appropriate function pointer in the bridge struct to the result. If not
 * set to the default implementation from the base bridge.
 *
 * @param FUNCSTRINGNAME The string function name.
 * @param FUNCIMPLNAME The name of the function pointer in the bridge struct
 * @param FUNCIMPLTYPE The type of the function pointer expected.
 */
#define REGISTER_BRIDGE_FUNCTION_WITH_DEFAULT(FUNCSTRINGNAME,                   \
                                              FUNCIMPLNAME,                     \
                                              FUNCIMPLTYPE)                     \
do {                                                                            \
    snprintf (functionName, 256, "%s"#FUNCSTRINGNAME, name);                    \
    result = loadLibFunc (bridgeLib, functionName);                             \
                                                                                \
    if (NULL != result) {                                                       \
        (*bridge)->FUNCIMPLNAME = (FUNCIMPLTYPE)result;                      \
        result = NULL;                                                          \
    } else {                                                                    \
        (*bridge)->FUNCIMPLNAME = (FUNCIMPLTYPE) mamaBase ## FUNCSTRINGNAME;    \
    }                                                                           \
} while (0)

/**
 * Register function pointers associated with a specific middleware bridge.
 */
mama_status
mamaLibraryManager_registerBridgeFunctions (LIB_HANDLE  bridgeLib,
                                            mamaBridge* bridge,
                                            const char* name)
{
    mama_status  status        = MAMA_STATUS_OK;
    const char * version       = NULL;
    void*        funcPointer   = NULL;
    void*        result        = NULL;
    char         functionName[256];
    char         propertyName[256];
    int          versionCompare = 0;

    wproperty_t  bridgeProperties = NULL;

    bridge_createImpl initFunc = NULL;
    bridge_getBridgeProperties propertyFunc = NULL;

    /* Begin by querying the bridge for it's property information, which we will
     * then attach to the struct (if required). This also allows us to
     * dynamically determine the registration approach to take when loading the
     * bridge.
     */
    snprintf (functionName, 256, "%sBridge_getBridgeProperties", name);
    funcPointer = loadLibFunc(bridgeLib, functionName);
    propertyFunc    = (bridge_getBridgeProperties)funcPointer;

    if (propertyFunc) {
        mama_log (MAMA_LOG_LEVEL_FINE,
                  "mamaLibraryManager_registerBridgeFunctions (): "
                  "Loading bridge properties for bridge [%s]",
                  name);

        status = propertyFunc (&bridgeProperties);

        if (MAMA_STATUS_OK != status) {
            mama_log (MAMA_LOG_LEVEL_ERROR,
                      "mamaLibraryManager_registerBridgeFunctions (): "
                      "Error loading bridge properties for bridge [%s]",
                      name);
            goto err1;
        }
    }

    /* Determine how the current bridge version compares with the MAMA version. */
    /* TODO: Implement a proper version comparision function. */
    snprintf (propertyName, 256, "mama.%s.supported_mama_version", name);
    versionCompare
        = mamaInternal_compareMamaVersion (
                                             properties_Get (bridgeProperties,
                                                             propertyName)
                                          );

    if (1 == versionCompare) {
        status = MAMA_STATUS_SYSTEM_ERROR;
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaLibraryManager_registerBridgeFunctions (): "
                  "Supported MAMA Version of [%s] bridge is greater than the current MAMA version. "
                  "Not loading.", name);
        goto err1;
    }

    /* Initialise the bridge by calling the init function. */
    snprintf (functionName, 256, "%sBridge_createImpl", name);
    funcPointer = loadLibFunc(bridgeLib, functionName);
    initFunc    = (bridge_createImpl)funcPointer;

    if (!initFunc)
    {
        status = MAMA_STATUS_NO_BRIDGE_IMPL;
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaInternal_registerBridgeFunctions(): "
                  "Could not find function [%s] in library [%s]",
                  functionName,
                  bridgeLib);
        goto err1;
    }

    /* Allocate the bridge impl before pasing to the bridge itself, which allows
     * us to ensure we correctly NULL everything before it hits the bridge.
     */
    *bridge = calloc (1, sizeof(mamaBridgeImpl));


    if (NULL == bridge)
    {
        status = MAMA_STATUS_NOMEM;
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaInternal_registerBridgeFunctions(): "
                  "Failed to allocate memory for middleware bridge impl.");
        goto err1;
    }

    /* TODO: This is a temporary mechanism. We need some sort of string conversion
     * function for checking against MAMA versions.
     */
    if (0 == versionCompare) {
        initFunc (bridge);
    } else {
        mamaBridge tmpBridge = NULL;
        initFunc (&tmpBridge);
        free(tmpBridge);
    }

    if (NULL == bridge)
    {
        status = MAMA_STATUS_NO_BRIDGE_IMPL;
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaInternal_registerBridgeFunctions(): "
                  "Error in bridge initialization.");
         goto err2;
    }

    (*bridge)->bridgeProperties = bridgeProperties;

    /* Once the bridge has been successfully loaded, and the initialization
     * function called, we register each of the required bridge functions.
     */
    REGISTER_BRIDGE_FUNCTION_WITH_DEFAULT (Bridge_additionalMethodTest,
                                            bridgeAdditionalMethodTest,
                                            bridge_additionalMethodTest);
    REGISTER_BRIDGE_FUNCTION (Bridge_start, bridgeStart, bridge_start);
    REGISTER_BRIDGE_FUNCTION (Bridge_stop,  bridgeStop,  bridge_stop);
    REGISTER_BRIDGE_FUNCTION (Bridge_open,  bridgeOpen,  bridge_open);
    REGISTER_BRIDGE_FUNCTION (Bridge_close, bridgeClose, bridge_close);

    /* General purpose functions */
    REGISTER_BRIDGE_FUNCTION (Bridge_getVersion, bridgeGetVersion, bridge_getVersion);

    /* Queue Functions */
    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_create, bridgeMamaQueueCreate, bridgeMamaQueue_create);
    REGISTER_BRIDGE_FUNCTION (BridgeMamaQueue_destroy, bridgeMamaQueueDestroy, bridgeMamaQueue_destroy);

    /* Payload Functions */
    REGISTER_BRIDGE_FUNCTION (Bridge_getDefaultPayloadId, bridgeGetDefaultPayloadId, bridge_getDefaultPayloadId);

    /* Once the functions have been registered, the middleware bridge needs to
     * be opened.
     */
    status = (*bridge)->bridgeOpen (*bridge);
    if (MAMA_STATUS_OK != status)
    {
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaInternal_loadLibrary(): "
                  "Error in bridge initialization.");
        goto err2;
    }

    /* Return success */
    return status;

err2:
    free(*bridge);
err1:
    return status;
}

/**
 * Register function pointers associated with a specific payload.
 */
mama_status
mamaLibraryManager_registerPayloadFunctions (LIB_HANDLE         bridgeLib,
                                             mamaPayloadBridge* bridge,
                                             const char*        name)
{
    mama_status status  = MAMA_STATUS_OK;
    void* funcPointer   = NULL;
    void* result        = NULL;
    char  functionName[256];
    char  payloadChar   = '\0';

    msgPayload_createImpl initFunc = NULL;

    /* Initialise the bridge by calling the init function. */
    snprintf (functionName, 256, "%sPayload_createImpl", name);
    funcPointer = loadLibFunc(bridgeLib, functionName);
    initFunc    = (msgPayload_createImpl)funcPointer;

    if (!initFunc)
    {
        status = MAMA_STATUS_NO_BRIDGE_IMPL;
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaInternal_registerPayloadFunctions(): "
                  "Could not find function [%s] in library [%s]",
                  functionName,
                  bridgeLib);
        goto err1;
    }

    /* Allocate the payload impl before passing to the bridge itself, which
     * allows us to ensure we correctly NULL everything before it hits the
     * bridge.
     */
    *bridge = calloc (1, sizeof(mamaPayloadBridgeImpl));
    if (NULL == bridge)
    {
        status = MAMA_STATUS_NOMEM;
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaInternal_registerPayloadFunctions(): "
                  "Failed to allocate memory for payload bridge impl.");
        goto err1;
    }

    initFunc (bridge, &payloadChar);
    if (NULL == bridge)
    {
        status = MAMA_STATUS_NO_BRIDGE_IMPL;
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaInternal_loadLibrary(): "
                  "Error in payload bridge initialization.");
        goto err2;
    }

    /* Once the payload has been successfully loaded, and the initialization
     * function run, we register the rest of the payload functions.
     * TODO: Finish the remainder of the payload bridge registration funcitons.
     */
    REGISTER_BRIDGE_FUNCTION (Payload_create, msgPayloadCreate, msgPayload_create);
    REGISTER_BRIDGE_FUNCTION (Payload_addI8, msgPayloadAddI8, msgPayload_addI8);

    return status;

err2:
    free (*bridge);
err1:
    return status;
}

/**
 * @brief Mechanism for registering optional plugin functions.
 *
 * Taking a function string name search the shared library handle, using the
 * loadLibFunc portability method, for the function. If it is found, set the
 * appropriate function pointer in function table. If no function is found,
 * ignore.
 *
 * @param FUNCSTRINGNAME The string function name.
 * @param FUNCIMPLSTRUCT The structure to which the function data needs to be
 *                       added.
 * @param FUNCIMPLNAME The name of the function pointer in the function struct.
 * @param FUNCIMPLTYPE The type of the function pointer expected.
 */
#define REGISTER_PLUGIN_FUNCTION(FUNCSTRINGNAME,                                \
                                 FUNCIMPLSTRUCT,                                \
                                 FUNCIMPLNAME,                                  \
                                 FUNCIMPLTYPE)                                  \
do {                                                                            \
    snprintf (functionName, 256, "%s"#FUNCSTRINGNAME, name);                    \
    result = loadLibFunc (bridgeLib, functionName);                             \
    int i = 0;                                                                  \
    int registered = 0;                                                         \
                                                                                \
    if (NULL != result) {                                                       \
        for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {                          \
            if (!FUNCIMPLSTRUCT[i].FUNCIMPLNAME) {                              \
                FUNCIMPLSTRUCT[i].FUNCIMPLNAME  = (FUNCIMPLTYPE)result;         \
                FUNCIMPLSTRUCT[i].plugin        = bridge;                       \
                closureFunc (*bridge,                                           \
                             #FUNCIMPLNAME,                                     \
                             &FUNCIMPLSTRUCT[i].closure);                       \
                registered = 1;                                                 \
                break;                                                          \
            }                                                                   \
        }                                                                       \
                                                                                \
        if (!registered) {                                                      \
            mama_log (MAMA_LOG_LEVEL_WARN,                                      \
                      "mamaLibraryManager_registerPluginFunctions (): "         \
                      "Maximum number of plugin functions already registered. " \
                      "[%s"#FUNCSTRINGNAME"]", name );                          \
            status = MAMA_STATUS_NOMEM;                                         \
        }                                                                       \
                                                                                \
        result = NULL;                                                          \
    }                                                                           \
} while (0);

/**
 * Register function pointers associated with a specific plugin.
 */
mama_status
mamaLibraryManager_registerPluginFunctions (LIB_HANDLE  bridgeLib,
                                            mamaPlugin* bridge,
                                            const char* name)
{
    mama_status status  = MAMA_STATUS_OK;
    void* funcPointer   = NULL;
    void* result        = NULL;
    char  functionName[256];

    mamaPlugin_init             initFunc    = NULL;
    mamaPlugin_queryClosureData closureFunc = NULL;

    mamaImpl          *gImpl         = mamaInternal_getGlobalImpl ();
    mamaFunctionTable *functionTable = gImpl->mFunctionTable;

    /* Initialise the bridge by calling the init function. */
    snprintf (functionName, 256, "%sPlugin_init", name);
    funcPointer = loadLibFunc(bridgeLib, functionName);
    initFunc    = (mamaPlugin_init)funcPointer;

    if (!initFunc)
    {
        status = MAMA_STATUS_NO_BRIDGE_IMPL;
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaInternal_registerPluginFunctions(): "
                  "Could not find function [%s] in library [%s]",
                  functionName,
                  bridgeLib);
        goto err1;
    }

    /* Setup the closure query function. */
    snprintf (functionName, 256, "%sPlugin_queryClosureData", name);
    funcPointer = loadLibFunc(bridgeLib, functionName);
    closureFunc    = (mamaPlugin_queryClosureData)funcPointer;

    if (!closureFunc)
    {
        status = MAMA_STATUS_NULL_ARG;
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaLibraryManager (): "
                  "Could not find function [%s] in library [%s]",
                  functionName,
                  bridgeLib);
        goto err1;
    }

    /* Allocate the plugin impl before passing to the bridge itself, which
     * allows us to ensure we correctly NULL everything before it hits the
     * bridge.
     */
    *bridge = calloc (1, sizeof(mamaPluginImpl));
    if (NULL == bridge)
    {
        status = MAMA_STATUS_NOMEM;
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaInternal_registerPayloadFunctions(): "
                  "Failed to allocate memory for payload bridge impl.");
        goto err1;
    }

    initFunc (*bridge, ((mamaPluginImpl*)bridge)->closure);
    if (NULL == bridge)
    {
        status = MAMA_STATUS_NO_BRIDGE_IMPL;
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mamaInternal_loadLibrary(): "
                  "Error in payload bridge initialization.");
         goto err2;
    }

    REGISTER_PLUGIN_FUNCTION (Plugin_onOpen,
                              functionTable->openCloseFunctions.onMamaOpen,
                              pluginOnOpen,
                              mamaPlugin_onOpen);

    REGISTER_PLUGIN_FUNCTION (Plugin_onClose,
                              functionTable->openCloseFunctions.onMamaClose,
                              pluginOnClose,
                              mamaPlugin_onClose);

    REGISTER_PLUGIN_FUNCTION (Plugin_onStart,
                              functionTable->openCloseFunctions.onMamaStart,
                              pluginOnStart,
                              mamaPlugin_onStart);

    REGISTER_PLUGIN_FUNCTION (Plugin_onStop,
                              functionTable->openCloseFunctions.onMamaStop,
                              pluginOnStop,
                              mamaPlugin_onStop);

    return status;

err2:
    free(*bridge);
err1:
    return status;
}


/*********************************************
 * Internal Functions: Library Loading
 **********************************************/

/**
 * Allocates the mamaFunctionTable structure, and associates it with the global
 * mamaImpl object. Should only be called by mamaInternal_init.
 */
mama_status
mamaLibraryManager_allocateFunctionTable (void)
{
    mama_status status = MAMA_STATUS_OK;
    mamaImpl    *gImpl = mamaInternal_getGlobalImpl();

    mamaFunctionTable* functionTable = calloc (1, sizeof(mamaFunctionTable));

    if (NULL == functionTable)
    {
        status = MAMA_STATUS_NOMEM;
        mama_log (MAMA_LOG_LEVEL_ERROR, "mamaInternal_allocateFunctionTable(): "
                  "Could not allocate memeory for function table.");
        goto exit;
    }

    /* Now that the memory has been successfully allocated, associate with
     * the global mamaImpl struct, and return. It is our responsibility to
     * clean up this memeory later.
     */
    gImpl->mFunctionTable = functionTable;

exit:
    return status;
}

/**
 * Loads a specified library of a given type, with an optional path, and stores
 * the resultant structure in the impl object.
 */
mama_status
mamaLibraryManager_loadLibrary (const char* libraryName,
                                const char* path,
                                void* impl,
                                mamaLibraryType libraryType)
{
    mama_status         status          = MAMA_STATUS_OK;

    char                libraryImplName  [256];
    char                initFuncName     [256];

    LIB_HANDLE          libraryLib      = NULL;
    void*               vp              = NULL;

    mamaMiddleware      middleware      = 0;
    char                payloadChar     = '\0';

    mamaImpl            *gImpl = mamaInternal_getGlobalImpl();

    if (!impl || !libraryName) {
        status = MAMA_STATUS_NULL_ARG;
        goto err0;
    }

    /*  Step 1:
     *      - Check if the bridge has already been loaded.
     *  On Error: 
     *      If already available, return MAMA_STATUS_OK.
     */
    wthread_static_mutex_lock (&gImpl->myLock);

    switch (libraryType) {
    case MAMA_MIDDLEWARE_LIBRARY:
    {
        mamaBridgeLib* bridgeLib =
            (mamaBridgeLib*)wtable_lookup (gImpl->mBridges, libraryName);

        if (bridgeLib)
        {
            status = MAMA_STATUS_OK;
            mama_log (MAMA_LOG_LEVEL_NORMAL,
                      "mamaInternal_loadLibrary(): "
                      "Middleware bridge %s already loaded.",
                      libraryName);

            *(mamaBridge*)impl = bridgeLib->bridge;
            goto exit; 
        }
        break;
    }
    case MAMA_PAYLOAD_LIBRARY:
    {
        mamaPayloadLib* payloadLib =
            (mamaPayloadLib*)wtable_lookup (gImpl->mPayloads, libraryName);

        if (payloadLib)
        {
            status = MAMA_STATUS_OK;
            mama_log (MAMA_LOG_LEVEL_NORMAL,
                      "mamaInternal_loadLibrary(): "
                      "Payload bridge %s already loaded.",
                      libraryName);

            *(mamaPayloadBridge*)impl = payloadLib->payload;
            goto exit;
        }
        break;
    }
    case MAMA_PLUGIN_LIBRARY:
    {
        mamaPluginLib* pluginLib =
            (mamaPluginLib*)wtable_lookup (gImpl->mPlugins, libraryName);

        if (pluginLib)
        {
            status = MAMA_STATUS_OK;
            mama_log (MAMA_LOG_LEVEL_NORMAL,
                      "mamaInternal_loadLibrary(): Plugin %s  already loaded.",
                      libraryName);

            *(mamaPlugin*)impl = pluginLib->plugin;
            goto exit;
        }
        break;
    }
    }

    /*  Step 2:
     *    - Find library name
     *      - For Middlewares:  mama<middleware>impl
     *      - For payload:      mama<payload>impl
     *      - For plugin:       mamaPlugin<plugin>impl
     *    - Open the shared library
     *      - Using standard call to openSharedLib
     *
     *  On Error:
     *      If the load fails, log and return MAMA_STATUS_NO_BRIDGE_IMPL (?)
     */
    switch (libraryType) {
    case MAMA_MIDDLEWARE_LIBRARY:
    {
        snprintf (libraryImplName, 256, "mama%simpl", libraryName);
        break;
    }
    case MAMA_PAYLOAD_LIBRARY:
    {
        snprintf (libraryImplName, 256, "mama%simpl", libraryName);
        break;
    }
    case MAMA_PLUGIN_LIBRARY:
    {
        snprintf (libraryImplName, 256, "mamaplugin%s", libraryName);
        break;
    }
    }

    libraryLib = openSharedLib (libraryImplName, path);

    if (!libraryLib)
    {
        if (path)
        {
            mama_log (MAMA_LOG_LEVEL_ERROR,
                        "mamaInternal_loadLibrary(): "
                        "Could not open %s [%s] [%s] [%s]",
                        mamaLibraryManager_libraryTypeToString (libraryType),
                        path,
                        libraryImplName,
                        getLibError());
        }
        else
        {
            mama_log (MAMA_LOG_LEVEL_ERROR,
                        "mamaInternal_loadLibrary(): "
                        "Could not open %s [%s] [%s]",
                        mamaLibraryManager_libraryTypeToString (libraryType),
                        libraryImplName,
                        getLibError());
        }
        status = MAMA_STATUS_NO_BRIDGE_IMPL;
        goto err1;
    }

    /* Step 3:
    *    - Register library functions.
     *      - For middleware:   mamaLibraryManager_registerBridgeFunctions
     *      - For payload:      mamaLibraryManager_registerPayloadFunctions
     *      - For plugin:       mamaLibraryManager_registerPluginFunctions
     *
     *  On Error:
     *      If the function doesn't exist,log and return MAMA_STATUS_NO_BRIDGE_IMPL
     */
    switch (libraryType) {
    case MAMA_MIDDLEWARE_LIBRARY:
    {
        status = mamaLibraryManager_registerBridgeFunctions (libraryLib,
                                                             (mamaBridge*)impl,
                                                             libraryName);

        if (MAMA_STATUS_OK != status)
            goto err2;

        break;
    }
    case MAMA_PAYLOAD_LIBRARY:
    {
        status = mamaLibraryManager_registerPayloadFunctions (libraryLib,
                                                              (mamaPayloadBridge*)impl,
                                                              libraryName);

        if (MAMA_STATUS_OK != status)
            goto err2;

        break;
    }
    case MAMA_PLUGIN_LIBRARY:
    {
        status = mamaLibraryManager_registerPluginFunctions (libraryLib,
                                                             (mamaPlugin*)impl,
                                                             libraryName);

        if (MAMA_STATUS_OK != status)
            goto err2;
        
        break;
    }
    }

    if (!impl)
    {
        status = MAMA_STATUS_NO_BRIDGE_IMPL;
        mama_log (MAMA_LOG_LEVEL_ERROR,
                  "mama_loadBridge(): Error in [%s] ", initFuncName);

        goto err2;
    }
    
    mama_log (MAMA_LOG_LEVEL_NORMAL,
              "mamaInternal_loadLibrary(): "
              "Sucessfully loaded %s %s from library [%s]",
              libraryName,
              mamaLibraryManager_libraryTypeToString (libraryType),
              libraryImplName);

    switch (libraryType) {
    case MAMA_MIDDLEWARE_LIBRARY:
    {
        mamaBridgeLib* bridgeLib = malloc (sizeof(mamaBridgeLib));
        int wtableStatus = WTABLE_SUCCESS;
        int i = 0;

        bridgeLib->name    = strdup (libraryName);
        bridgeLib->bridge  = *(mamaBridge*)impl;
        bridgeLib->library = libraryLib;

        wtableStatus = wtable_insert (gImpl->mBridges,
                                      libraryName,
                                      (void*)bridgeLib);

        if (WTABLE_SUCCESS != wtableStatus)
        {
            status = MAMA_STATUS_SYSTEM_ERROR;
            mama_log (MAMA_LOG_LEVEL_ERROR, "mamaInternal_loadLibrary(): "
                                            "Failed to add middleware [%s] to map",
                                            libraryName);

            goto err2;
        }

        for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {
            if (gImpl->mFunctionTable->libraryLoadFunctions.onBridgeLoad[i])
                gImpl->mFunctionTable->libraryLoadFunctions.
                                            onBridgeLoad[i] (bridgeLib->bridge,
                                                             libraryName);
        }

        break;
    }
    case MAMA_PAYLOAD_LIBRARY:
    {
        mamaPayloadLib* payloadLib = malloc (sizeof(mamaPayloadLib));
        int wtableStatus = WTABLE_SUCCESS;
        int i = 0;

        payloadLib->name    = strdup (libraryName);
        payloadLib->payload = *(mamaPayloadBridge*)impl;
        payloadLib->library = libraryLib;

        wtableStatus = wtable_insert (gImpl->mPayloads,
                                      libraryName,
                                      (void*)payloadLib);

        if (WTABLE_SUCCESS != wtableStatus)
        {
            status = MAMA_STATUS_SYSTEM_ERROR;
            mama_log (MAMA_LOG_LEVEL_ERROR, "mamaInternal_loadLibrary(): "
                                            "Failed to add payload [%s] to map",
                                            libraryName);
            goto err2;
        }

        if (!mamaInternal_getDefaultPayload ())
        {
            mamaInternal_setDefaultPayload (*(mamaPayloadBridge*)impl);
        }

        for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {
            if (gImpl->mFunctionTable->libraryLoadFunctions.onPayloadLoad[i])
                gImpl->mFunctionTable->libraryLoadFunctions.
                                            onPayloadLoad[i] (payloadLib->payload,
                                                              libraryName);
        }

        break;
    }
    case MAMA_PLUGIN_LIBRARY:
    {
        mamaPluginLib* pluginLib = malloc (sizeof(mamaPluginLib));
        int wtableStatus = WTABLE_SUCCESS;
        int i = 0;

        pluginLib->name    = strdup (libraryName);
        pluginLib->plugin  = *(mamaPlugin*)impl;
        pluginLib->library = libraryLib;

        wtableStatus = wtable_insert (gImpl->mPlugins,
                                      libraryName,
                                      (void*)pluginLib);

        if (WTABLE_SUCCESS != wtableStatus)
        {
            status = MAMA_STATUS_SYSTEM_ERROR;
            mama_log (MAMA_LOG_LEVEL_ERROR, "mamaInternal_loadLibrary(): "
                                            "Failed to add plugin [%s] to map",
                                            libraryName);

            goto err2;
        }

        for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {
            if (gImpl->mFunctionTable->libraryLoadFunctions.onPluginLoad[i])
                gImpl->mFunctionTable->libraryLoadFunctions.
                                            onPluginLoad[i] (pluginLib->plugin,
                                                             libraryName);
        }

        break;
    }
    }

exit: /* Exit success */
    wthread_static_mutex_unlock (&gImpl->myLock);
    return status;

err2: /* Exit after library open */
    closeSharedLib (libraryLib);

err1: /* Exit after lock */
    wthread_static_mutex_unlock (&gImpl->myLock);

err0: /* Exit before lock */
    return status;
}

/*********************************************************
 * General Utility Functions
 *********************************************************/

/**
 * Utility function to print the library type as a string
 */
const char*
mamaLibraryManager_libraryTypeToString (mamaLibraryType type) {
    switch (type) {
    case MAMA_MIDDLEWARE_LIBRARY:
        return "middleware library";
    case MAMA_PAYLOAD_LIBRARY:
        return "payload_library";
    case MAMA_PLUGIN_LIBRARY:
        return "plugin library";
    }
}


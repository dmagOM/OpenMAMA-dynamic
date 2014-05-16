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
 * 021MAX_FUNCTION_CALLBACKS-1301 USA
 */

#include <mama/loadcallbacks.h>
#include <mamainternal.h>
#include <mama/log.h>

/* TODO: Really should check the return codes of some of the functions we call...*/

/*
 * @brief Register a callback triggered on bridge load.
 *
 * Public function registration of a callback which is triggered when a new
 * middleware bridge is loaded by OpenMAMA. Will only be triggered once
 * a bridge has been successfully loaded.
 *
 * @param bridgeLoadCb A function pointer to the callback to be triggered.
 * @return A mama_status, with MAMA_STATUS_OK indicating a successful
 *         registration, MAMA_STATUS_NOMEM indicates that the maximum number of
 *         callbacks have already been registered.
 */
mama_status
mama_registerBridgeLoadCallback (mamaBridge_onLoad bridgeLoadCb)
{
    mama_status status = MAMA_STATUS_OK;
    int i;
    int registered = 0;

    mamaImpl          *globalImpl = mamaInternal_getGlobalImpl();
    mamaFunctionTable *functionTable;

    mamaInternal_init ();

    functionTable = globalImpl->mFunctionTable;

    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {
        if (!functionTable->libraryLoadFunctions.onBridgeLoad[i])
        {
            functionTable->libraryLoadFunctions.onBridgeLoad[i] = bridgeLoadCb;
            registered = 1;
            break;
        }
    }

    if (!registered) 
    {
        mama_log (MAMA_LOG_LEVEL_WARN, "mama_registerBridgeLoadCallback(): "
                  "Maximum number of callbacks already registered. Ignoring.");
        status = MAMA_STATUS_NOMEM;
    }

    return status;
}

/**
 * @brief Register a callback triggered on bridge unload.
 *
 * Public function for registration of a callback which is triggered when a
 * middleware bridge is unloaded by OpenMAMA. Will only be triggered once
 * a bridge has been successfully unloaded.
 *
 * @param bridgeUnloadCb A function pointer to the callback to be triggered.
 * @return A mama_status, with MAMA_STATUS_OK indicating a successful
 *         registration, MAMA_STATUS_NOMEM indicates that the maximum number of
 *         callbacks have already been registered.
 */
mama_status
mama_registerBridgeUnloadCallback (mamaBridge_onUnload bridgeUnloadCb)
{
    mama_status status = MAMA_STATUS_OK;
    int i;
    int registered = 0;

    mamaImpl          *globalImpl = mamaInternal_getGlobalImpl();
    mamaFunctionTable *functionTable;

    mamaInternal_init ();

    functionTable = globalImpl->mFunctionTable;

    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {
        if (!functionTable->libraryLoadFunctions.onBridgeUnload[i])
        {
            functionTable->libraryLoadFunctions.onBridgeUnload[i] = bridgeUnloadCb;
            registered = 1;
            break;
        }
    }

    if (!registered) 
    {
        mama_log (MAMA_LOG_LEVEL_WARN, "mama_registerBridgeUnloadCallback(): "
                  "Maximum number of callbacks already registered. Ignoring.");
        status = MAMA_STATUS_NOMEM;
    }

    return status;
}

/**
 * @brief Register a callback to be triggered on payload load.
 *
 * Public function for registration of a callback which is triggered each time
 * a new payload bridge is loaded by OpenMAMA. Will only be triggered once
 * a payload has been successfully loaded.
 *
 * @param payloadLoadCb A function pointer to the callback to be triggered.
 * @return A mama_status, with MAMA_STATUS_OK indicating a successful
 *         registration, MAMA_STATUS_NOMEM indicates that the maximum number of
 *         callbacks have already been registered. 
 */
mama_status
mama_registerPayloadLoadCallback (mamaPayload_onLoad payloadLoadCb)
{
    mama_status status = MAMA_STATUS_OK;
    int i;
    int registered = 0;

    mamaImpl          *globalImpl = mamaInternal_getGlobalImpl();
    mamaFunctionTable *functionTable;

    mamaInternal_init ();

    functionTable = globalImpl->mFunctionTable;

    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {
        if (!functionTable->libraryLoadFunctions.onPayloadLoad[i])
        {
            functionTable->libraryLoadFunctions.onPayloadLoad[i] = payloadLoadCb;
            registered = 1;
            break;
        }
    }

    if (!registered) 
    {
        mama_log (MAMA_LOG_LEVEL_WARN, "mama_registerPayloadLoadCallback(): "
                  "Maximum number of callbacks already registered. Ignoring.");
        status = MAMA_STATUS_NOMEM;
    }

    return status;
}

/**
 * @brief Register a callback to be triggered on payload unload.
 *
 * Public function for registration of a callback which is triggered each time
 * a payload bridge is unloaded by OpenMAMA. Will only be triggered once
 * a payload has been successfully unloaded.
 *
 * @param payloadLoadCb A function pointer to the callback to be triggered.
 * @return A mama_status, with MAMA_STATUS_OK indicating a successful
 *         registration, MAMA_STATUS_NOMEM indicates that the maximum number of
 *         callbacks have already been registered. 
 */
mama_status
mama_registerPayloadUnloadCallback (mamaPayload_onUnload payloadUnloadCb)
{
    mama_status status = MAMA_STATUS_OK;
    int i;
    int registered = 0;

    mamaImpl          *globalImpl = mamaInternal_getGlobalImpl();
    mamaFunctionTable *functionTable;

    mamaInternal_init ();

    functionTable = globalImpl->mFunctionTable;

    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {
        if (!functionTable->libraryLoadFunctions.onPayloadUnload[i])
        {
            functionTable->libraryLoadFunctions.onPayloadUnload[i] = payloadUnloadCb;
            registered = 1;
            break;
        }
    }

    if (!registered) 
    {
        mama_log (MAMA_LOG_LEVEL_WARN, "mama_registerPayloadUnloadCallback(): "
                  "Maximum number of callbacks already registered. Ignoring.");
        status = MAMA_STATUS_NOMEM;
    }

    return status;
}

/**
 * @brief Register a callback to be triggered on plugin load.
 *
 * Public function for registration of a callback which is triggered each time
 * a new plugin is loaded by OpenMAMA. Will only be triggered once plugin has
 * been successfully loaded.
 *
 * @param payloadLoadCb A function pointer to the callback to be triggered.
 * @return A mama_status, with MAMA_STATUS_OK indicating a successful
 *         registration, MAMA_STATUS_NOMEM indicates that the maximum number of
 *         callbacks have already been registered. 
 */
mama_status
mama_registerPluginLoadCallback (mamaPlugin_onLoad pluginLoadCb)
{
    mama_status status = MAMA_STATUS_OK;
    int i;
    int registered = 0;

    mamaImpl          *globalImpl = mamaInternal_getGlobalImpl();
    mamaFunctionTable *functionTable;

    mamaInternal_init ();

    functionTable = globalImpl->mFunctionTable;

    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {
        if (!functionTable->libraryLoadFunctions.onPluginLoad[i])
        {
            functionTable->libraryLoadFunctions.onPluginLoad[i] = pluginLoadCb;
            registered = 1;
            break;
        }
    }

    if (!registered) 
    {
        mama_log (MAMA_LOG_LEVEL_WARN, "mama_registerPluginLoadCallback(): "
                  "Maximum number of callbacks already registered. Ignoring.");
        status = MAMA_STATUS_NOMEM;
    }

    return status;
}

/**
 * @brief Register a callback to be triggered on plugin unload.
 *
 * Public function for registration of a callback which is triggered each time
 * a plugin is unloaded by OpenMAMA. Will only be triggered once a plugin has
 * been successfully unloaded.
 *
 * @param payloadLoadCb A function pointer to the callback to be triggered.
 * @return A mama_status, with MAMA_STATUS_OK indicating a successful
 *         registration, MAMA_STATUS_NOMEM indicates that the maximum number of
 *         callbacks have already been registered. 
 */
mama_status
mama_registerPluginUnloadCallback (mamaPlugin_onUnload pluginUnloadCb)
{
    mama_status status = MAMA_STATUS_OK;
    int i;
    int registered = 0;

    mamaImpl          *globalImpl = mamaInternal_getGlobalImpl();
    mamaFunctionTable *functionTable;

    mamaInternal_init ();

    functionTable = globalImpl->mFunctionTable;

    for (i = 0; i < MAX_FUNCTION_CALLBACKS; ++i) {
        if (!functionTable->libraryLoadFunctions.onPluginUnload[i])
        {
            functionTable->libraryLoadFunctions.onPluginUnload[i] = pluginUnloadCb;
            registered = 1;
            break;
        }
    }

    if (!registered) 
    {
        mama_log (MAMA_LOG_LEVEL_WARN, "mama_registerPluginUnloadCallback(): "
                  "Maximum number of callbacks already registered. Ignoring.");
        status = MAMA_STATUS_NOMEM;
    }

    return status;
}


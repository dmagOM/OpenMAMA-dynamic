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

#ifndef MamaLoadCallbacksH__
#define MamaLoadCallbacksH__

#include <mama/types.h>
#include <mama/status.h> /* mama_status */

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/**
 * @brief Function pointer prototype for the onBridgeLoad callback.
 *
 * The callback is triggered each time a middleware bridge is loaded by 
 * OpenMAMA.
 *
 * @param bridge The bridge which has been loaded
 * @param name A null terminated string name of the bridge.
 * @return Status indicating the success of the command, though OpenMAMA is
 *         presently free to ignore the response.
 */
typedef 
mama_status (*mamaBridge_onLoad) (mamaBridge bridge, const char* name);

/**
 * @brief Function pointer prototype for the onBridgeUnload callback.
 *
 * Triggered when a middleware bridge is unloaded by OpenMAMA.
 *
 * @param name A null terminated string with the name of the bridge.
 * @return Status indicating the success of the function, though OpenMAMA is 
 *         presently free to ignore the response.
 */
typedef
mama_status (*mamaBridge_onUnload) (const char* name);

/**
 * @brief Function pointer prototype for the onPayloadLoad callback.
 *
 * Triggered when a payload bridge is loaded by OpenMAMA.
 *
 * @param payload The payload which has just been loaded.
 * @param name A null terminated string with the name of the payload.
 * @return A mama_status indicating success of the function, though OpenMAMA is
 *         presently free to ignore the response.
 */
typedef
mama_status (*mamaPayload_onLoad) (mamaPayloadBridge payload,
                                   const char* name);

/**
 * @brief Function pointer prototype for the onPayloadUnload callback.
 *
 * Triggered when a payload bridge has been unloaded by OpenMAMA.
 *
 * @param name A null terminated string with the name of the payload.
 * @return A mama_status indicating the success of the function, though OpenMAMA
 *         is presently free to ignore the response.
 */
typedef
mama_status (*mamaPayload_onUnload) (const char* name);

/**
 * @brief Function pointer prototype for the onPluginLoad callback.
 *
 * Triggered when a plugin has been loaded by OpenMAMA.
 *
 * @param plugin The plugin which has been loaded.
 * @param name A null terminated string with the name of the plugin.
 * @return A mama_status indicating the success of the function, though OpenMAMA
 *         is presently free to ignore the response.
 */
typedef
mama_status (*mamaPlugin_onLoad) (mamaPlugin plugin, const char* name);

/**
 * @brief Function pointer prototype for the onPluginUnload callback.
 *
 * Triggered when a plugin is unloaded by OpenMAMA.
 *
 * @param name A null terminated string with the name of the plugin.
 * @return A mama_status indicating the success of the function, though OpenMAMA
 *         is presently free to ignore the response.
 */
typedef
mama_status (*mamaPlugin_onUnload) (const char* name);


/**
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
extern mama_status
mama_registerBridgeLoadCallback (mamaBridge_onLoad bridgeLoadCb);

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
extern mama_status
mama_registerBridgeUnloadCallback (mamaBridge_onUnload bridgeUnloadCb);

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
extern mama_status
mama_registerPayloadLoadCallback (mamaPayload_onLoad payloadLoadCb);

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
extern mama_status
mama_registerPayloadUnloadCallback (mamaPayload_onUnload payloadUnloadCb);

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
extern mama_status
mama_registerPluginLoadCallback (mamaPlugin_onLoad pluginLoadCb);

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
extern mama_status
mama_registerPluginUnloadCallback (mamaPlugin_onUnload pluginUnloadCb);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaLoadCallbacksH__ */

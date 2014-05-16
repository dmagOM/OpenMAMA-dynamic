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

#ifndef MamaLibraryManagerH__
#define MamaLibraryManagerH__

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

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
                                          mama_size_t *size);

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
                                       mama_size_t *size);

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
                                      mama_size_t *size);

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
 *         MAMA_STATUS_NO_BRIDGE_IMPL indicates no loaded bridge with that name
 *         was found.
 */
mama_status
mamaLibraryManager_getMiddleware (const char *middlewareName,
                                  mamaBridge *bridge);

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
 *         MAMA_STATUS_NO_BRIDGE_IMPL indicates no loaded bridge with that name
 *         was found.
 */
mama_status
mamaLibraryManager_getPayload (const char        *payloadName,
                               mamaPayloadBridge *payload);

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
 *         MAMA_STATUS_NO_BRIDGE_IMPL indicates no loaded bridge with that name
 *         was found.
 */
mama_status
mamaLibraryManager_getPlugin (const char *pluginName,
                              mamaPlugin *plugin);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /*MamaLibraryManagerH__ */

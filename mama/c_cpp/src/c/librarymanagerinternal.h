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

#ifndef MamaLibraryManagerInternalH__
#define MamaLibraryManagerInternalH__

#include <mama/types.h>
#include <mama/status.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/*********************************************************
 * Library Manager Data Structures.
 *********************************************************/

/**
 * Enum representing the different types of libraries which may be loaded by
 * OpenMAMA.
 */
typedef enum mamaLibraryType_ {
    MAMA_MIDDLEWARE_LIBRARY,            /**< Middleware bridge library */
    MAMA_PAYLOAD_LIBRARY,               /**< Payload bridge library */
    MAMA_PLUGIN_LIBRARY                 /**< Plugin library */
} mamaLibraryType;

/**
 * Structure for storing combined mamaBridge and LIB_HANDLE data
 */
typedef struct mamaBridgeLib_ {
    const char  *name;                  /**< Middleware name */
    mamaBridge  bridge;                 /**< Middleware bridge structure */
    LIB_HANDLE  library;                /**< Middleware bridge shared library handle */
} mamaBridgeLib;

/**
 * Structure for storing combined mamaPayload and LIB_HANDLE data
 */
typedef struct mamaPayloadLib_ {
    const char         *name;           /**< Payload name */
    mamaPayloadBridge  payload;         /**< Payload bridge structure */
    LIB_HANDLE         library;         /**< Payload bridge shared library handle */
} mamaPayloadLib;

/**
 * Structure for storing combined mamaPlugin and LIB_HANDLE data
 */
typedef struct mamaPluginLib_ {
    const char  *name;                  /**< Plugin name */
    mamaPlugin  plugin;                 /**< Plugin structure */
    LIB_HANDLE  library;                /**< Plugin shared library handle */
} mamaPluginLib;

/**
 * @brief Method for managing the registration of middleware bridge functions.
 *
 * Internal method used by OpenMAMA to perform a number of bridge setup and
 * registration steps. This includes the allocation of the appropriate structure
 * for the bridge, the calling of an initialisation function, and the
 * registration of any available plugin functions.
 *
 * @param[in]  bridgeLib A library object which is used by the function to
 *             search for the functions to be registered.
 * @param[out] bridge A pointer to the mamaBridge structure for which the 
 *             functions are being registered.
 * @param[in]  middlewareName The name of the middleware for which the functions
 *             are being registered. Must match the library name so the function
 *             search will work correctly.
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
mama_status
mamaLibraryManager_registerBridgeFunctions (LIB_HANDLE  bridgeLib,
                                            mamaBridge* bridge,
                                            const char* middlewareName);
/**
 * @brief Method for managing the registration of payload bridge functions.
 *
 * Internal method used by OpenMAMA to perform a number of payload setup and
 * registration steps. This includes the allocation of the appropriate structure
 * for the payload, the calling of an initialisation function, and the
 * registration of any available payload functions.
 *
 * @param[in]  payloadLib A library object which is used by the function to
 *             search for the functions to be registered.
 * @param[out] payload A pointer to the mamaBridge structure for which the
 *             functions are being registered.
 * @param[in]  payloadName The name of the middleware for which the functions
 *             are being registered. Must match the library name so the function
 *             search will work correctly.
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
mama_status
mamaLibraryManager_registerPayloadFunctions (LIB_HANDLE         payloadLib,
                                             mamaPayloadBridge* payloadBridge,
                                             const char*        payloadName);
/**
 * @brief Method for managing the registration of plugin functions.
 *
 * Internal method used by OpenMAMA to perform a number of plugin setup and
 * registration steps. This includes the allocation of the appropriate structure
 * for the plugin, the calling of an initialisation function for that plugin,
 * and the registration of any available plugin functions.
 *
 * @param[in]  pluginLib A library object which is used by the function to
 *             search for the functions to be registered.
 * @param[out] plugin A pointer to the mamaPlugin structure for which the
 *             functions are being registered.
 * @param[in]  name The name of the plugin for which the functions are being
 *             registered. Must match the library name so the function search
 *             will work correctly.
 *
 * @return A mama_status indicating the success or failure of the registration.
 */
mama_status
mamaLibraryManager_registerPluginFunctions (LIB_HANDLE  pluginLib,
                                      mamaPlugin* plugin,
                                      const char* name);

/**
 * @brief Load and initialise libraries of specific types.
 *
 * Takes a libraryName, an optional path, a void* structure, and a library
 * type, and loads the appropriate shared library. It then searches the library
 * for a function which is used to initialise the library, before finally
 * calling the bridge registration methods.
 *
 * @param libraryName The string name for a library e.g. wmw, qpid.
 * @param path The path on which the library may be found.
 * @param impl A void pointer to type specific data structures
 *              - mamaBridge for middleware bridges
 *              - mamaPayloadBridge for payload bridges
 *              - mamaPlugin for plugin libraries
 * @param libraryType enum mamaLibraryType value denoting the type of library 
 *                    to be loaded.
 *
 * @return A mama_status denoting the success or failure of the bridge loading.
 *         MAMA_STATUS_OK denotes a successful load.
 *         MAMA_STATUS_NULL_ARG indicates a NULL argument was passed to the function.
 *         MAMA_STATUS_NO_BRIDGE_IMPL indicates a failure to load the shared library.
 *         MAMA_STATUS_NOMEM indicates a failure to allocate memeory.
 *         MAMA_STATUS_SYSTEM_ERROR indicates a failure in another part of the
 *           system (wTable failures in particular).
 *
 */
mama_status
mamaLibraryManager_loadLibrary (const char* libraryName,
                                const char* path,
                                void* impl,
                                mamaLibraryType libraryType);

/**
 * @brief Function for allocating the global function table
 *
 * Allocates the mamaFunctionTable structure, and associates it with the global
 * mamaImpl object. Should only be called by mamaInternal_init.
 *
 * @return A mama_status indicating the success of failure of the allocation.
 *         MAMA_STATUS_OK indicates success, MAMA_STATUS_NOMEM indicates that
 *         there has been a failure to allocate memory.
 */
mama_status
mamaLibraryManager_allocateFunctionTable (void);


/*********************************************************
 * General Utility Functions
 *********************************************************/

/**
 * @brief Converstion function to convert a library enum value to a string
 * representation.
 *
 * Takes an enum value (in the form of a mamaLibraryType), and returns a null
 * terminated string representation of the type.
 *
 * @param type The enum to be converted.
 * @return A null terminated string representtion of the library type.
 */
const char*
mamaLibraryManager_libraryTypeToString (mamaLibraryType type);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaLibraryManagerInternalH__ */

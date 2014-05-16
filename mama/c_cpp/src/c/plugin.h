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

#ifndef MamaPluginH__
#define MamaPluginH__

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* Structure for storing the plugin info. */
typedef struct mamaPluginImpl_ {
    void* closure;
} mamaPluginImpl;

/**
 * @brief initialisation function for OpenMAMA plugins. 
 *
 * Function is called during plugin loading, and allows the Plugin the
 * opportunity to perform any required initialisation.
 *
 * @param[in]  plugin  The mamaPlugin object associated with this Plugin.
 * @param[out] closure void* value which may optionally be set by plugins.
 */
typedef mama_status
(*mamaPlugin_init) (mamaPlugin plugin, void **closure);

/**
 * @brief Function to return closure information per integration point.
 *
 * Function allows OpenMAMA to query plugin bridges for specific closure
 * information to be included along with the plugin integration functions.
 * MAMA will pass a string representation of the required info, and the response
 * is expected to be set in the closure parameter.
 *
 * Current requested responses:
 *  - "onOpen"
 *  - "onClose"
 *  - "onStart"
 *  - "onStop"
 *
 * @param[in] plugin MamaPlugin object for which the closure is being queried
 * @param[in] query  Null terminated string indicating the closure being queried
 * @param[out] closure void* in which the closure should be returned.
 *
 * @return mama_status indicating the success or failure of the query. MAMA may
 *         ignore the response, as a failure will also be indicated by returning
 *         a NULL pointer.
 */
typedef mama_status
(*mamaPlugin_queryClosureData) (mamaPlugin plugin,
                                const char *query,
                                void **closure);

/**
 * @brief Integration function called when mama_open is triggered.
 *
 * @param plugin The mamaPlugin for which the call is triggered.
 * @param closure The closure data associated with the call.
 *
 * @return A mama_status indicating the success or failure of the call. May be
 *         ignored by OpenMAMA at present.
 */
typedef mama_status
(*mamaPlugin_onOpen) (mamaPlugin plugin, void *closure);

/**
 * Structure storing the details of the onOpen plugin integration function, and
 * supporting closure data.
 */
struct mamaPluginStruct_onOpen {
    mamaPlugin_onOpen pluginOnOpen;     /**< Integration function pointer. */
    mamaPlugin        *plugin;          /**< Plugin associated with the call */
    void              *closure;         /**< Closure data associated with the call */
};

/**
 * @brief Integration function called when mama_close is triggered.
 *
 * @param plugin The mamaPlugin for which the call is triggered.
 * @param closure The closure data associated with the call.
 *
 * @return A mama_status indicating the success or failure of the call. May be
 *         ignored by OpenMAMA at present.
 */
typedef mama_status
(*mamaPlugin_onClose) (mamaPlugin plugin, void *closure);

/**
 * Structure storing the details of the onClose plugin integration function, and
 * supporting closure data.
 */
struct mamaPluginStruct_onClose {
    mamaPlugin_onClose  pluginOnClose;  /**< Integration function pointer. */
    mamaPlugin          *plugin;        /**< Plugin associated with the call */
    void                *closure;       /**< Closure data associated with the call */
};

/**
 * @brief Integration function called when mama_start is triggered.
 *
 * @param bridge The mamaBridge for which start has been called.
 * @param plugin The mamaPlugin for which the call is triggered.
 * @param closure The closure data associated with the call.
 *
 * @return A mama_status indicating the success or failure of the call. May be
 *         ignored by OpenMAMA at present.
 */
typedef mama_status
(*mamaPlugin_onStart) (mamaBridge bridge, mamaPlugin plugin, void *closure);

/**
 * Structure storing the details of the onStart plugin integration function, and
 * supporting closure data.
 */
struct mamaPluginStruct_onStart {
    mamaPlugin_onStart  pluginOnStart;  /**< Integration function pointer. */
    mamaPlugin          *plugin;        /**< Plugin associated with the call */
    void                *closure;       /**< Closure data associated with the call */
};

/**
 * @brief Integration function called when mama_stop is triggered.
 *
 * @param bridge The mamaBridge for which stop has been called.
 * @param plugin The mamaPlugin for which the call is triggered.
 * @param closure The closure data associated with the call.
 *
 * @return A mama_status indicating the success or failure of the call. May be
 *         ignored by OpenMAMA at present.
 */
typedef mama_status
(*mamaPlugin_onStop) (mamaBridge bridge, mamaPlugin plugin, void *closure);

/**
 * Structure storing the details of the onStop plugin integration function, and
 * supporting closure data.
 */
struct mamaPluginStruct_onStop {
    mamaPlugin_onStop   pluginOnStop;   /**< Integration function pointer. */
    mamaPlugin          *plugin;        /**< Plugin associated with the call */
    void                *closure;       /**< Closure data associated with the call */
};

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /*MamaPluginH__ */

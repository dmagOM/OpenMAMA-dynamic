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

#ifndef MamaFunctionTableH__
#define MamaFunctionTableH__

#include <mama/loadcallbacks.h>
#include <plugin.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* It would be nicer to use a static const here, but that doesn't seem to be
 * allowed by -pedantic...
 */
#define MAX_FUNCTION_CALLBACKS 10

/**
 *  Function pointers for general library loading callbacks.
 */
struct mamaLibraryLoadFunctions {
    /* Array of function pointers to trigger when a bridge is loaded. */
    mamaBridge_onLoad onBridgeLoad[MAX_FUNCTION_CALLBACKS];

    /* Array of function pointers to trigger when a bridge is unloaded */
    mamaBridge_onUnload onBridgeUnload[MAX_FUNCTION_CALLBACKS];

    /* Array of function pointers to trigger when a payload is loaded. */
    mamaPayload_onLoad onPayloadLoad[MAX_FUNCTION_CALLBACKS];

    /* Array of function pointers to trigger when a payload is unloaded. */
    mamaPayload_onUnload onPayloadUnload[MAX_FUNCTION_CALLBACKS];

    /* Array of function pointers to trigger when a plugin is loaded. */
    mamaPlugin_onLoad onPluginLoad[MAX_FUNCTION_CALLBACKS];

    /* Array of function pointers to trigger when a plugin is unloaded. */
    mamaPlugin_onUnload onPluginUnload[MAX_FUNCTION_CALLBACKS];
};

/**
 * Function pointers for open/close plugin integration points.
 */
struct mamaOpenCloseFunctions {

    /* Array of Plugin onOpen function structs */
    struct mamaPluginStruct_onOpen  onMamaOpen[MAX_FUNCTION_CALLBACKS];

    /* Array of plugin onClose function structs */
    struct mamaPluginStruct_onClose onMamaClose[MAX_FUNCTION_CALLBACKS];

    /* Array of plugin onStart function structs */
    struct mamaPluginStruct_onStart onMamaStart[MAX_FUNCTION_CALLBACKS];

    /* Array of plugin onStop function structs */
    struct mamaPluginStruct_onStop  onMamaStop[MAX_FUNCTION_CALLBACKS];
};

/**
 * The mamaFunctionTable_ structure is not a table per-say, but rather a
 * bunch of structures of function pointer arrays. Each struct should be a self
 * contained grouping of function pointers, Each array has a pre-set size, 
 */
typedef struct mamaFunctionTable_ {

    /* Function pointers relating to loading and unloading libraries */
    struct mamaLibraryLoadFunctions libraryLoadFunctions;

    /* Function pointers relating to mama_open, mama_close, mama_start and mama_stop */
    struct mamaOpenCloseFunctions   openCloseFunctions;

} mamaFunctionTable;

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MamaFunctionTableH__ */

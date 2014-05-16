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

#ifndef MamaInternalH__
#define MamaInternalH__

#include <wombat/wtable.h>

#include <property.h>
#include <functiontable.h>
#include <librarymanagerinternal.h>
#include <mama/loadcallbacks.h>
#include "mama/types.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define MAMA_PAYLOAD_MAX	CHAR_MAX

/**
 * This structure contains data needed to control starting and stopping of
 * mama.
 */
typedef struct mamaImpl_
{
    /* wtable of mamaBridgeLib structures. */
    wtable_t               mBridges;

    /* wtable of mamaPayloadLib structures */
    wtable_t               mPayloads;

    /* wtable of payload ID to payload string names */
    wtable_t               mPayloadIdMap;

    /* wtable of mamaPluginLib structures */
    wtable_t               mPlugins;

    /**
     * Pointer to a structure containing the various function tables which may
     * be used by Mama:
     * - Callbacks for bridge loading management.
     * - Plugin integration points.
     */
    mamaFunctionTable      *mFunctionTable;

    /* Flag noting if the mamaImpl has been initialised. 
     * TODO: We currently lock around the initialisation of all of this,
     * but it would probably be cleaner to use a wInterlocked. The problem
     * is where/how do we initialize it.
     */
    unsigned int           mInit;
    unsigned int           myRefCount;
    wthread_static_mutex_t myLock;
} mamaImpl;

/**
* Check whether Callbacks are run in 'debug' catch exceptions mode
*/
MAMAExpDLL
extern int
mamaInternal_getCatchCallbackExceptions (void);

/**
 * Exposes the property object from mama
 */
MAMAExpDLL
extern wproperty_t
mamaInternal_getProperties (void);

/* Used by the bridges to register themselves with MAMA. Should not
   be called from anywhere else */
MAMAExpDLL
extern void
mamaInternal_registerBridge (mamaBridge     bridge,
                             const char*    middleware);

/**
 * getVersion for use when mama is wrapped
 */
const char*
mama_wrapperGetVersion(mamaBridge     bridge);

MAMAExpDLL
extern mamaStatsGenerator
mamaInternal_getStatsGenerator (void);

MAMAExpDLL
extern mamaStatsCollector
mamaInternal_getGlobalStatsCollector (void);

MAMAExpDLL
extern void
cleanupReservedFields (void);

MAMAExpBridgeDLL
extern int
mamaInternal_generateLbmStats (void);

MAMAExpBridgeDLL
mamaBridge
mamaInternal_findBridge (void);

mamaPayloadBridge
mamaInternal_findPayload (char id);

mamaPayloadBridge
mamaInternal_getDefaultPayload (void);

void
mamaInternal_setDefaultPayload (mamaPayloadBridge payloadBridge);

mama_bool_t
mamaInternal_getAllowMsgModify (void);

/**
 * @brief Performs required initialisation for the OpenMAMA global structures.
 *
 * Initialises the various tables and other structures which OpenMAMA uses to
 * maintain it's global state.
 *
 * @return Status indicating the success or failure of the initialisation.
 */
mama_status
mamaInternal_init (void);

/**
 * @brief Function for returning a pointer to the global mamaImpl object.
 *
 * Returns a pointer to the global mamaImpl object, gImpl.
 *
 * @return A mamaImpl pointer to the gImpl object.
 */
mamaImpl *
mamaInternal_getGlobalImpl (void);


/**
 * @brief compares the MAMA version in the supplied string with the current version
 *
 * Perform a comparison between a MAMA version derived from the supplied NULL
 * terminated string and the current version of OpenMAMA. 
 *  - If the supplied version is greater than the current version, return a
 *    value of 1.
 *  - If the two versions are the same, return a value of 0. 
 *  - If the supplied version is less than the current version, return a -1.
 *
 * @param[in] version A NULL terminated string representation of the MAMA version
 *            for which the comparison is to be performed.
 *
 * @return An integer value representing the result of the comparison.
 */
int
mamaInternal_compareMamaVersion (const char *version);

#if defined(__cplusplus)
}
#endif

#endif /*MamaInternalH__ */

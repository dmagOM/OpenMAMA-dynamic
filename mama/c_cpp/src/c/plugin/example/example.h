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

#ifndef MamaPluginExampleH__
#define MamaPluginExampleH__

#include <mama/types.h>
#include <mama/status.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* Structure for storing the plugin info. */
typedef struct mamaExampleImpl_ {
    const char* data;
} mamaExampleImpl;

mama_status
examplePlugin_init (mamaPlugin plugin);

mama_status
examplePlugin_queryClosureData (mamaPlugin plugin,
                                const char *query,
                                void **closure);

mama_status
examplePlugin_onOpen (mamaPlugin plugin, void *closure);

mama_status
examplePlugin_onClose (mamaPlugin plugin, void *closure);

mama_status
examplePlugin_onStart (mamaBridge bridge, mamaPlugin plugin, void *closure);

mama_status
examplePlugin_onStop (mamaBridge bridge, mamaPlugin plugin, void *closure);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /*MamaPluginH__ */

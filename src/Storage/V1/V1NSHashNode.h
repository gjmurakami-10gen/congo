/* V1NSHashNode.h
 *
 * Copyright (C) 2014 MongoDB, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef V1_NS_HASH_NODE_H
#define V1_NS_HASH_NODE_H


#include <stddef.h>

#include <Core/Debug.h>
#include <Core/Macros.h>
#include <Core/Types.h>
#include <Storage/V1/V1NSDetails.h>


BEGIN_DECLS


#pragma pack(push, 1)
typedef struct
{
   int32_t     hash;
   char        key [128];
   V1NSDetails details;
} V1NSHashNode;
#pragma pack(pop)


STATIC_ASSERT (sizeof (V1NSHashNode) == 628);
STATIC_ASSERT (offsetof (V1NSHashNode, details) == 132);


END_DECLS


#endif /* V1_NS_HASH_NODE_H */

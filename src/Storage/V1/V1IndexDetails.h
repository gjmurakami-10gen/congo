/* V1IndexDetails.h
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


#ifndef V1_INDEX_DETAILS_H
#define V1_INDEX_DETAILS_H


#include <Core/Debug.h>
#include <Core/Macros.h>
#include <Core/Types.h>
#include <Storage/V1/V1IndexDetails.h>


BEGIN_DECLS


#pragma pack(push, 1)
typedef struct
{
   V1DiskLoc head; /* Location of BTree head */
   V1DiskLoc info; /* Location of Index info BSON */
} V1IndexDetails;
#pragma pack(pop)


STATIC_ASSERT (sizeof (V1IndexDetails) == 16);


END_DECLS


#endif /* V1_INDEX_DETAILS_H */

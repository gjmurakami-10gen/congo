/* V1DeletedRecordHeader.h
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


#ifndef V1_DELETED_RECORD_HEADER_H
#define V1_DELETED_RECORD_HEADER_H


#include <stddef.h>

#include <Core/Debug.h>
#include <Core/Macros.h>
#include <Core/Types.h>
#include <Storage/V1/V1DiskLoc.h>


BEGIN_DECLS


#pragma pack(push, 1)
typedef struct
{
   int32_t   length;
   int32_t   extent_offset;
   V1DiskLoc next_deleted_loc;
   char      data [4];
} V1DeletedRecordHeader;
#pragma pack(pop)


STATIC_ASSERT (sizeof (V1DeletedRecordHeader) == 20);


END_DECLS


#endif /* V1_DELETED_RECORD_HEADER_H */

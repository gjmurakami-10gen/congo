/* V1NSDetails.h
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


#ifndef V1_NS_DETAILS_H
#define V1_NS_DETAILS_H


#include <stddef.h>

#include <Core/Debug.h>
#include <Core/Macros.h>
#include <Core/Types.h>
#include <Storage/V1/V1DiskLoc.h>
#include <Storage/V1/V1IndexDetails.h>


BEGIN_DECLS


#define V1_NS_DETAILS_NUM_MAX   64
#define V1_NS_DETAILS_NUM_EXTRA 30
#define V1_NS_DETAILS_NUM_BASE  10


#pragma pack(push, 1)
typedef struct
{
   V1DiskLoc      first_extent;
   V1DiskLoc      last_extent;
   V1DiskLoc      free_list_buckets [19];
   struct {
      int64_t     datasize;
      int64_t     nrecords;
   } stats;
   int32_t        last_extent_size;
   int32_t        num_indexes;
   V1IndexDetails indexes [V1_NS_DETAILS_NUM_BASE];
   int32_t        is_capped;
   int32_t        max_docs_in_capped;
   double         padding_factor;
   int32_t        system_flags;
   V1DiskLoc      cap_extent;
   V1DiskLoc      cap_first_new_record;
   uint16_t       data_file_version;
   uint16_t       index_file_version;
   uint64_t       multi_key_index_bits;
   uint64_t       reserved_a;
   int64_t        extra_offset;
   int32_t        index_builds_in_progress;
   int32_t        user_flags;
   char           reserved [72];
} V1NSDetails;
#pragma pack(pop)


STATIC_ASSERT (sizeof (V1NSDetails) == 496);
STATIC_ASSERT (offsetof (V1NSDetails, stats) == 168);
STATIC_ASSERT (offsetof (V1NSDetails, indexes) == 192);
STATIC_ASSERT (offsetof (V1NSDetails, is_capped) == 352);
STATIC_ASSERT (offsetof (V1NSDetails, system_flags) == 368);
STATIC_ASSERT (offsetof (V1NSDetails, reserved_a) == 400);
STATIC_ASSERT (offsetof (V1NSDetails, reserved) == 424);


END_DECLS


#endif /* V1_NS_DETAILS_H */

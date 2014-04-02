/* Array.h
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


#ifndef ARRAY_H
#define ARRAY_H


#include <Sort.h>
#include <Macros.h>
#include <Types.h>


BEGIN_DECLS


#define ARRAY_INITIALIZER(type) { 0, NULL, 0, sizeof(type), false }
#define Array_Append(array, val) Array_AppendRange(array, 1, &(val))
#define Array_Index(array, type, index) (((type *)(array)->data)[index])


typedef struct
{
   uint32_t len;
   void *data;

   /*< private >*/
   uint32_t allocated_len;
   uint32_t element_size;
   bool zeroed;
} Array;


void        Array_AppendRange (Array *array,
                               uint32_t count,
                               const void  *range);
void        Array_Clear       (Array *array);
void        Array_Grow        (Array *array,
                               uint32_t len);
void        Array_Init        (Array *array,
                               uint32_t element_size,
                               bool zeroed);
void        Array_InitSized   (Array *array,
                               uint32_t element_size,
                               bool zeroed,
                               uint32_t length);
void        Array_Destroy     (Array *array);
void        Array_Remove      (Array *array,
                               uint32_t index);
void        Array_RemoveFast  (Array *array,
                               uint32_t index);
const void *Array_Search      (Array *array,
                               const void *target,
                               CompareFunc compare);
void       *Array_StealData   (Array *array,
                               size_t *len);
void        Array_Sort        (Array *array,
                               CompareFunc compare);


END_DECLS


#endif /* ARRAY_H */

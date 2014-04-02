/* Array.c
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


#include <stdlib.h>
#include <string.h>

#include <Array.h>
#include <Debug.h>
#include <Memory.h>


/*
 *--------------------------------------------------------------------------
 *
 * Array_Grow --
 *
 *       Grow the allocated size of the array to the new length, counted
 *       in number of elements.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
Array_Grow (Array *array, /* IN */
            uint32_t len) /* IN */
{
   size_t bytes;

   ASSERT (array);
   ASSERT (len > array->len);

   bytes = ((size_t)len) * ((size_t)array->element_size);

   array->data = realloc(array->data, bytes);
   ASSERT (array->data);

   array->allocated_len = len;

   if (array->zeroed) {
      memset(array->data + (array->len * array->element_size), 0,
             (array->allocated_len - array->len) * array->element_size);
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * Array_Init --
 *
 *       Initializes a new Array. If zereoed is TRUE, then memory will be
 *       zeroed on allocation. Element size should be set to the number of
 *       bytes in each element.
 *
 *       Caller should free with Array_Destroy() when no longer needed.
 *
 * Returns:
 *      None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
Array_Init (Array *array,          /* OUT */
            uint32_t element_size, /* IN */
            bool zeroed)           /* IN */
{
   Array_InitSized (array, element_size, zeroed, 16);
}


/*
 *--------------------------------------------------------------------------
 *
 * Array_InitSized --
 *
 *       Like Array_Init() except that you can specify the amount
 *       of memory to allocate right away. This can save time if you know
 *       the number of items you will need in the array upon creation.
 *
 *       Caller should free with Array_Destroy() when no longer needed.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
Array_InitSized (Array *array,          /* OUT */
                 uint32_t element_size, /* IN */
                 bool zeroed,           /* IN */
                 uint32_t count)        /* IN */
{
   ASSERT (array);

   array->len = 0;
   array->data = NULL;
   array->allocated_len = 0;
   array->element_size = element_size;
   array->zeroed = zeroed;

   if (count) {
      Array_Grow (array, count);
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * Array_Destroy --
 *
 *       Like Array_Destroy() except the structure is not freed. This
 *       is useful for static Array structures initialized with
 *       ARRAY_INITIALIZER().
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       Structures resources are released.
 *
 *--------------------------------------------------------------------------
 */

void
Array_Destroy (Array *array) /* IN */
{
   ASSERT (array);

   Memory_Free (array->data);

   array->data = NULL;
   array->len = 0;
   array->allocated_len = 0;
   array->zeroed = 0;
}


/*
 *--------------------------------------------------------------------------
 *
 * Array_Remove --
 *
 *       Remove an element from the array.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       The item is removed from array.
 *
 *--------------------------------------------------------------------------
 */

void
Array_Remove (Array *array, /* IN */
              uint32_t idx)    /* IN */
{
   ASSERT (array);
   ASSERT (idx < array->len);

   if ((idx + 1) == array->len) {
      array->len--;
      return;
   }

   memmove (array->data + (array->element_size * idx),
            array->data + (array->element_size * (idx + 1)),
            ((array->len - 1 - idx) * array->element_size));

   array->len--;
}


/*
 *--------------------------------------------------------------------------
 *
 * Array_RemoveFast --
 *
 *       Remove an item from the array. The item is replaced with the last
 *       item in the array allowing this to perform in O(1) running time.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       The item is removed from the array.
 *
 *--------------------------------------------------------------------------
 */

void
Array_RemoveFast (Array *array, /* IN */
                  uint32_t idx) /* IN */
{
   ASSERT (array);
   ASSERT (idx < array->len);

   if ((idx + 1) == array->len) {
      array->len--;
      return;
   }

   memcpy (array->data + (array->element_size * idx),
           array->data + (array->element_size * (array->len - 1)),
           array->element_size);

   array->len--;
}


/*
 *--------------------------------------------------------------------------
 *
 * Array_AppendRange --
 *
 *       Append a range of items to the array. range should be a pointer
 *       to the first item and count the number of elements.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       The range of items is added to the array.
 *
 *--------------------------------------------------------------------------
 */

void
Array_AppendRange (Array *array,      /* IN */
                   uint32_t count,    /* IN */
                   const void *range) /* IN */
{
   uint32_t alen;

   ASSERT (array);
   ASSERT (range);

   if ((array->len + count) > array->allocated_len) {
      alen = MAX (16, array->allocated_len);

      while ((array->len + count) > alen) {
         alen <<= 1;
      }

      Array_Grow(array, alen);
   }

   memcpy (array->data + (array->element_size * array->len),
           range, (count * array->element_size));

   array->len += count;
}


/*
 *--------------------------------------------------------------------------
 *
 * Array_StealData --
 *
 *       Steals the data from the array and resizes it to zero length.
 *       The caller becomes the owner of the data. The number of elements
 *       is stored in len.
 *
 * Returns:
 *       The previous array->data pointer.
 *
 * Side effects:
 *       len is set to number of elements.
 *
 *--------------------------------------------------------------------------
 */

void *
Array_StealData (Array *array, /* IN */
                 size_t *len)  /* OUT */
{
   void *ret = NULL;
   size_t retlen = 0;

   ASSERT (array);

   if (array->len) {
      ret = array->data;
      retlen = array->len;

      array->allocated_len = 0;
      array->data = NULL;
      array->len = 0;
   }

   if (len) {
      *len = retlen;
   }

   return ret;
}


/*
 *--------------------------------------------------------------------------
 *
 * Array_Clear --
 *
 *       Remove all items from the array. The allocated size of the array
 *       does not change.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
Array_Clear (Array *array) /* IN */
{
   ASSERT (array);

   array->len = 0;

   if (array->zeroed) {
      memset (array->data, 0, array->allocated_len * array->element_size);
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * Array_Sort --
 *
 *       Sorts the contents of @array using @compare to compare elements.
 *       The sort is performed using qsort().
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
Array_Sort (Array *array,        /* IN */
            CompareFunc compare) /* IN */
{
   ASSERT (array);
   ASSERT (compare);

   qsort (array->data, array->len, array->element_size, compare);
}


/*
 *--------------------------------------------------------------------------
 *
 * Array_Search --
 *
 *       Performs a binary search for @target using bsearch() on the
 *       array elements.
 *
 * Returns:
 *       A pointer to the matching element, or NULL.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

const void *
Array_Search (Array *array,        /* IN */
              const void *target,  /* IN */
              CompareFunc compare) /* IN */
{
   ASSERT (array);
   ASSERT (compare);

   return bsearch (target,
                   array->data,
                   array->len,
                   array->element_size,
                   compare);
}

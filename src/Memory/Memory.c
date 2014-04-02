/* Memory.c
 *
 * Copyright (C) 2013 MongoDB, Inc.
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


#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <Debug.h>
#include <Memory.h>


/*
 *--------------------------------------------------------------------------
 *
 * Memory_Malloc0 --
 *
 *       Returns a malloc()'d buffer that has been zeroed out.
 *
 *       You should use this instead of calloc() so that as memory pools
 *       are added, your allocation will automatically land there.
 *
 * Returns:
 *       A newly allocated buffer that should be freed with Memory_Free().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void *
Memory_Malloc0 (size_t size) /* IN */
{
   return calloc (1, size);
}


/*
 *--------------------------------------------------------------------------
 *
 * Memory_Malloc --
 *
 *       Returns a malloc()'d buffer that has not been initialized.
 *
 *       You should use this instead of malloc() so that as memory pools
 *       are added, your allocation will automatically land there.
 *
 * Returns:
 *       A newly allocated buffer that should be freed with Memory_Free().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void *
Memory_Malloc (size_t size) /* IN */
{
   return malloc (size);
}


/*
 *--------------------------------------------------------------------------
 *
 * Memory_Malloc0N --
 *
 *       Like Memory_malloc0(), but will check for overflow. If the
 *       allocation should overflow size_t, then NULL is returned.
 *
 * Returns:
 *       A newly allocated buffer that should be freed with Memory_Free()
 *       or NULL.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void *
Memory_Malloc0N (size_t elem_size, /* IN */
                 size_t count)     /* IN */
{
   ASSERT (elem_size);

   if (!count || (count > (SIZE_MAX / elem_size))) {
      return NULL;
   }

   return Memory_Malloc0 (elem_size * count);
}


/*
 *--------------------------------------------------------------------------
 *
 * Memory_Free --
 *
 *       Free an allocation allocated with this module.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @mem is freed.
 *
 *--------------------------------------------------------------------------
 */

void
Memory_Free (void *mem) /* IN */
{
   free (mem);
}


/*
 *--------------------------------------------------------------------------
 *
 * Memory_SafeRealloc --
 *
 *       A realloc() equivalent that will abort() if the realloc could not
 *       be performed.
 *
 * Returns:
 *       A realloc()'d buffer of at least @size bytes.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void *
Memory_SafeRealloc (void *mem,   /* IN */
                    size_t size) /* IN */
{
   mem = realloc (mem, size);

   if (!mem && size) {
      fprintf (stderr, "Failed to alloc %llu bytes.\n",
               (unsigned long long)size);
      abort ();
   }

   return mem;
}


/*
 *--------------------------------------------------------------------------
 *
 * Memory_SafeMalloc0 --
 *
 *       Like Memory_Malloc0() except abort() is called if there was a
 *       failure to malloc().
 *
 * Returns:
 *       A newly allocated buffer that should be freed with Memory_Free().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void *
Memory_SafeMalloc0 (size_t size) /* IN */
{
   void *mem;

   mem = calloc (1, size);

   if (!mem && size) {
      fprintf (stderr, "Failed to calloc %llu bytes.\n",
               (unsigned long long)size);
      abort ();
   }

   return mem;
}


/*
 *--------------------------------------------------------------------------
 *
 * Memory_SafeMalloc --
 *
 *       Like Memory_Malloc() except abort() is called if there was a
 *       failure to allocate @size bytes.
 *
 * Returns:
 *       A newly allocated buffer that should be freed with Memory_Free().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void *
Memory_SafeMalloc (size_t size) /* IN */
{
   void *mem;

   mem = malloc (size);

   if (!mem && size) {
      fprintf (stderr, "Failed to malloc %llu bytes.\n",
               (unsigned long long)size);
      abort ();
   }

   return mem;
}


/*
 *--------------------------------------------------------------------------
 *
 * Memory_SafeMallocN --
 *
 *       Allocate @count elements of @elem_size or abort(). If the resulting
 *       size would overflow size_t, abort() will also be called.
 *
 * Returns:
 *       A newly allocated buffer that should be freed with Memory_Free().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void *
Memory_SafeMallocN (size_t elem_size, /* IN */
                    size_t count)     /* IN */
{
   size_t size;
   void *mem;

   ASSERT (elem_size);

   if (!count || (count > (SIZE_MAX / elem_size))) {
      fprintf (stderr, "Malloc overflow detected!");
      abort ();
   }

   size = elem_size * count;
   mem = Memory_Malloc (size);

   if (!mem) {
      fprintf (stderr, "Failed to malloc %llu bytes.\n",
               (unsigned long long)size);
      abort ();
   }

   return mem;
}


void *
Memory_Memalign (size_t size,      /* IN */
                 size_t alignment) /* IN */
{
   void *ptr = NULL;

   if (0 == posix_memalign (&ptr, alignment, size)) {
      return ptr;
   }

   fprintf (stderr, "Failed to posix_memalign() %llu bytes.\n",
            (unsigned long long)size);

   abort ();
}

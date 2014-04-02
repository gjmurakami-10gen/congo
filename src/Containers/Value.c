/* Value.c
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


#include <Debug.h>
#include <Value.h>


STATIC_ASSERT (sizeof (Value) == 16);


/*
 *--------------------------------------------------------------------------
 *
 * Value_Copy --
 *
 *       Copy the contents of @src into @dest, possibly allocating
 *       memory for copied resources.
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
Value_Copy (const Value *src, /* IN */
            Value *dest)      /* OUT */
{
   ASSERT (src);
   ASSERT (dest);

   memcpy (dest, src, sizeof *dest);

   switch (dest->type) {
   case VALUE_TYPE_STRING:
      dest->u.str = CString_Dup (src->u.str);
      break;
   default:
      break;
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * Value_Destroy --
 *
 *       Cleanup after @value, potentially freeing allocated memory.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @value is destroyed.
 *
 *--------------------------------------------------------------------------
 */

void
Value_Destroy (Value *value) /* IN */
{
   if (value) {
      switch (value->type) {
      case VALUE_TYPE_STRING:
         CString_Free (value->u.str);
         value->u.str = NULL;
         break;
      default:
         break;
      }
   }
}

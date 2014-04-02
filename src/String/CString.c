/* CString.c
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
#include <CString.h>


/*
 *--------------------------------------------------------------------------
 *
 * CString_Dup --
 *
 *       Duplicate the contents of @str into a newly allocated string.
 *
 * Returns:
 *       A newly allocated string that should be freed with Memory_Free().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

char *
CString_Dup (const char *str) /* IN */
{
   size_t len;
   char *copy = NULL;

   if (str) {
      len = strlen (str);
      copy = Memory_SafeMalloc (len + 1);
      memcpy (copy, str, len);
      copy [len] = '\0';
   }

   return copy;
}


/*
 *--------------------------------------------------------------------------
 *
 * CString_NDup --
 *
 *       Duplicate the contents of @str into a newly allocated string.
 *
 * Returns:
 *       A newly allocated string that should be freed with Memory_Free().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

char *
CString_NDup (const char *str, /* IN */
              ssize_t len)     /* IN */
{
   char *copy;

   if (!str) {
      return NULL;
   } else if (len < 0) {
      len = strlen (str);
   }

   copy = Memory_SafeMalloc (len + 1);
   memcpy (copy, str, len);
   copy [len] = '\0';

   return copy;
}


/*
 *--------------------------------------------------------------------------
 *
 * CString_Copy --
 *
 *       Copy the contents of @src into @dest. If there is not enough room
 *       in @dest for @src, 0 is returned and no copy is made.
 *
 * Returns:
 *       1 on success, 0 on failure.
 *
 * Side effects:
 *       @dest is initialized.
 *
 *--------------------------------------------------------------------------
 */

int
CString_Copy (const char *src, /* IN */
              char *dest,      /* OUT */
              size_t destlen)  /* IN */
{
   int len;

   ASSERT (src);
   ASSERT (dest);

   len = strlen (src);

   if (len < destlen) {
      strncpy (dest, src, destlen);
      dest [destlen - 1] = '\0';
      return 1;
   }

   if (destlen) {
      dest [0] = '\0';
   }

   return 0;
}


/*
 *--------------------------------------------------------------------------
 *
 * CString_Concat --
 *
 *       Concat @src onto the end of @dest.
 *
 * Returns:
 *       1 if the string was concat'd onto @dest.
 *       0 if there was not enough space to concat @src.
 *
 * Side effects:
 *       @dest will be appended with @src.
 *
 *--------------------------------------------------------------------------
 */

int
CString_Concat (char *dest,       /* INOUT */
                size_t size,      /* IN */
                const char *src)  /* IN */
{
   size_t destlen;
   size_t srclen;

   ASSERT (dest);

   if (src) {
      destlen = strlen (dest);
      srclen = strlen (src);

      if ((destlen + srclen + 1) > size) {
         return 0;
      }

      memcpy (dest + destlen, src, srclen);
      dest [destlen + srclen] = '\0';
   }

   return 1;
}


/*
 *--------------------------------------------------------------------------
 *
 * CString_HasPrefix --
 *
 *       Check if @src begings with @prefix.
 *
 * Returns:
 *       1 if @src beings with @prefix, otherwise 0.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
CString_HasPrefix (const char *src,    /* IN */
                   const char *prefix) /* IN */
{
   size_t srclen = strlen (src);
   size_t prefixlen = strlen (prefix);

   if (prefixlen <= srclen) {
      return (0 == memcmp (src, prefix, prefixlen));
   }

   return 0;
}


/*
 *--------------------------------------------------------------------------
 *
 * CString_HasSuffix --
 *
 *       Checks if @src ends with @suffix.
 *
 * Returns:
 *       1 if @src ends with @suffix, otherwise 0.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
CString_HasSuffix (const char *src,    /* IN */
                   const char *suffix) /* OUT */
{
   size_t srclen = strlen (src);
   size_t suffixlen = strlen (suffix);

   if (suffixlen <= srclen) {
      return (0 == memcmp (src + srclen - suffixlen, suffix, suffixlen));
   }

   return 0;
}


/*
 *--------------------------------------------------------------------------
 *
 * CString_Hash --
 *
 *       A hash-function for a string. This is from Daniel J. Bernstein.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

uint32_t
CString_Hash (const void *data) /* IN */
{
   const char *string = data;
   uint32_t hash = 5381;
   uint32_t i;

   ASSERT(string);

   for (i = 0; string[i]; i++) {
      hash = ((hash << 5) + hash) + string[i];
   }

   return hash;
}


/*
 *--------------------------------------------------------------------------
 *
 * CString_Equal --
 *
 *       Checks to see if two strings are identical. Neither string may
 *       be NULL.
 *
 * Returns:
 *       true if the strings are identical; otherwise false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

bool
CString_Equal (const void *data1, /* IN */
               const void *data2) /* IN */
{
   const char *a = data1;
   const char *b = data2;

   ASSERT (data1);
   ASSERT (data2);

   for (; *a && *b; a++, b++) {
      if (!*(a + 1) && !*(b + 1)) {
         return true;
      }
   }

   return false;
}

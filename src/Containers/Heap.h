/* Heap.h
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


#ifndef HEAP_H
#define HEAP_H


#include <string.h>

#include <Array.h>
#include <Macros.h>


BEGIN_DECLS


#define _HEAP_PARENT(npos)   (((npos)-1)/2)
#define _HEAP_LEFT(npos)     (((npos)*2)+1)
#define _HEAP_RIGHT(npos)    (((npos)*2)+2)
#define _HEAP_ELEMENT(self, ElementType, i) \
   (((ElementType *)((Array *)self)->data)[i])
#define _HEAP_SWAP(self, ElementType, a, b) \
   do { \
      ElementType tmp; \
      memcpy (&tmp, \
              &_HEAP_ELEMENT(self, ElementType, a), \
              sizeof tmp); \
      memcpy (&_HEAP_ELEMENT(self, ElementType, a), \
              &_HEAP_ELEMENT(self, ElementType, b), \
              sizeof tmp); \
      memcpy (&_HEAP_ELEMENT(self, ElementType, b), \
              &tmp, \
              sizeof tmp); \
   } while (0)


#define HEAP_DEFINE(Name, ElementType, Compare) \
typedef Array Name; \
\
static __inline__ unsigned \
Name##_Size (Name *self) \
{ \
   return ((Array *)self)->len; \
} \
\
static void \
Name##_Init (Name *self) \
{ \
   Array_Init ((Array *)self, sizeof(ElementType), false); \
} \
\
static void \
Name##_Destroy (Name *self) \
{ \
   Array_Destroy ((Array *)self); \
} \
\
static void \
Name##_Insert (Name *self, ElementType *val) \
{ \
   int ipos; \
   int ppos; \
\
   ipos = Name##_Size(self); \
   ppos = _HEAP_PARENT (ipos); \
\
   Array_AppendRange ((Array *)self, 1, val); \
\
   while ((ipos > 0) && \
          (Compare (&_HEAP_ELEMENT (self, ElementType, ppos), \
                    &_HEAP_ELEMENT (self, ElementType, ipos)) < 0)) { \
      _HEAP_SWAP (self, ElementType, ppos, ipos); \
\
      ipos = ppos; \
      ppos = _HEAP_PARENT (ipos); \
   } \
} \
\
static bool \
Name##_Extract (Name *self, ElementType *val) \
{ \
   int ipos; \
   int lpos; \
   int rpos; \
   int mpos; \
\
   if (Name##_Size (self)) { \
      memcpy (val, &_HEAP_ELEMENT (self, ElementType, 0), sizeof *val); \
      memmove (&_HEAP_ELEMENT (self, ElementType, 0), \
               &_HEAP_ELEMENT (self, ElementType, Name##_Size(self) - 1), \
               sizeof(ElementType)); \
\
      ((Array *)self)->len--; \
\
      if (!Name##_Size (self)) { \
         return true; \
      } \
\
      ipos = 0; \
\
      for (;;) { \
         lpos = _HEAP_LEFT (ipos); \
         rpos = _HEAP_RIGHT (ipos); \
\
         if ((lpos < Name##_Size (self)) && \
             (Compare (&_HEAP_ELEMENT (self, ElementType, lpos), \
                       &_HEAP_ELEMENT (self, ElementType, ipos)) > 0)) { \
            mpos = lpos; \
         } else { \
            mpos = ipos; \
         } \
\
         if ((rpos < Name##_Size (self)) && \
             (Compare (&_HEAP_ELEMENT(self, ElementType, rpos), \
                       &_HEAP_ELEMENT(self, ElementType, mpos)) > 0)) { \
            mpos = rpos; \
         } \
\
         if (mpos == ipos) { \
            break; \
         } \
\
         _HEAP_SWAP (self, ElementType, mpos, ipos); \
\
         ipos = mpos; \
      } \
\
      return true; \
   } \
\
   return false; \
}


END_DECLS


#endif /* _HEAP_H */

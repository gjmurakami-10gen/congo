/* CString.h
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


#ifndef CSTRING_H
#define CSTRING_H


#include <string.h>

#include <Macros.h>
#include <Memory.h>


BEGIN_DECLS


#define CString_Length(s) ((s) ? strlen(s) : 0)
#define CString_Free(s)   Memory_Free(s)
#define CString_Empty(s)  (!(s) || !((s)[0]))


char     *CString_Dup       (const char *str);
char     *CString_NDup      (const char *str,
                             ssize_t len);
int       CString_Copy      (const char *src,
                             char *dest,
                             size_t destlen);
int       CString_Concat    (char *dest,
                             size_t size,
                             const char *src);
int       CString_HasPrefix (const char *src,
                             const char *prefix);
int       CString_HasSuffix (const char *src,
                             const char *suffix);
uint32_t  CString_Hash      (const void *data);
bool      CString_Equal     (const void *data1,
                             const void *data2);


END_DECLS

#endif /* CSTRING_H */

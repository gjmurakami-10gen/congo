/* Error.c
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


#include <stdio.h>
#include <stdarg.h>

#include <Error.h>


void
Error_Init (Error *error,       /* OUT */
            int domain,         /* IN */
            int code,           /* IN */
            const char *format, /* IN */
            ...)                /* IN */
{
   va_list args;

   if (error) {
      error->domain = domain;
      error->code = code;
      va_start (args, format);
      vsnprintf (error->message, sizeof error->message, format, args);
      error->message [sizeof error->message - 1] = '\0';
      va_end (args);
   }
}

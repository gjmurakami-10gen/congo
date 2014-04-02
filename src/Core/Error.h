/* Error.h
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


#ifndef ERROR_H
#define ERROR_H


#include <Macros.h>


BEGIN_DECLS


typedef struct
{
   int  domain;
   int  code;
   char message [128];
} Error;


void
Error_Init (Error      *error,
            int         domain,
            int         code,
            const char *format,
            ...) GNUC_PRINTF(4, 5);


END_DECLS


#endif /* ERROR_H */

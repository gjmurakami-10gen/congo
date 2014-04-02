/* OptionEntry.h
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


#ifndef OPTION_ENTRY_H
#define OPTION_ENTRY_H


#include <Error.h>
#include <Macros.h>
#include <Types.h>


#define OPTION_ENTRY_ERROR 100
#define OPTION_ENTRY_ERROR_INVALID_STRING 1
#define OPTION_ENTRY_ERROR_INVALID_INT    2


BEGIN_DECLS


typedef enum
{
   OPTION_ARG_NONE = 0,  /* NULL arg_data */
   OPTION_ARG_STRING,    /* char **arg_data */
   OPTION_ARG_FILENAME,  /* char **arg_data */
   OPTION_ARG_INT,       /* int *arg_data */
} OptionArg;


typedef struct
{
   const char  *long_name;
   char         short_name;
   int          flags;
   OptionArg    arg;
   void        *arg_data;
   const char  *description;
} OptionEntry;


bool OptionEntry_Matches (OptionEntry *entry,
                          const char *arg);
bool OptionEntry_Parse   (OptionEntry *entry,
                          char **arg,
                          bool *swallow_next,
                          Error *error);


END_DECLS


#endif /* OPTION_ENTRY_H */

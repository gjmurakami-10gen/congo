/* OptionContext.h
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


#ifndef OPTION_CONTEXT_H
#define OPTION_CONTEXT_H


#include <stdio.h>

#include <Error.h>
#include <Macros.h>
#include <OptionEntry.h>
#include <Types.h>


BEGIN_DECLS


typedef struct
{
   OptionEntry *entries;
   int n_entries;
   const char *prgname;
   const char *description;
} OptionContext;


void OptionContext_Init       (OptionContext *context,
                               const char *prgname,
                               const char *description);
void OptionContext_AddEntries (OptionContext *context,
                               OptionEntry *entries,
                               int n_entries);
bool OptionContext_Parse      (OptionContext *context,
                               int argc,
                               char **argv,
                               Error *error);
void OptionContext_Destroy    (OptionContext *context);
void OptionContext_ShowHelp   (OptionContext *context,
                               FILE *stream);


END_DECLS


#endif /* OPTION_CONTEXT_H */

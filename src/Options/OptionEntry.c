/* OptionEntry.c
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <Debug.h>
#include <OptionEntry.h>


bool
OptionEntry_Matches (OptionEntry *entry, /* IN */
                     const char *arg)    /* IN */
{
   ASSERT (entry);
   ASSERT (arg);

   if (entry->long_name && (0 == strncmp (arg, "--", 2))) {
      if (0 == strcmp (arg + 2, entry->long_name)) {
         return true;
      }
      if ((entry->arg != OPTION_ARG_NONE) &&
          (0 == strncmp (arg + 2, entry->long_name,
                         strlen (entry->long_name))) &&
          arg [2 + strlen (entry->long_name)] == '=') {
         return true;
      }
   } else if ((arg [0] == '-') && (arg [1] == entry->short_name) && !arg [2]) {
      return true;
   }

   return false;
}


static void
OptionEntry_ErrorMissingArg (OptionEntry *entry, /* IN */
                             Error *error)       /* OUT */
{
   if (entry->long_name) {
      Error_Init (error,
                  OPTION_ENTRY_ERROR,
                  OPTION_ENTRY_ERROR_INVALID_STRING,
                  "The option --%s requires an argument.",
                  entry->long_name);
   } else {
      Error_Init (error,
                  OPTION_ENTRY_ERROR,
                  OPTION_ENTRY_ERROR_INVALID_STRING,
                  "The option -%c requires an argument.",
                  entry->short_name);
   }
}


bool
OptionEntry_Parse (OptionEntry *entry, /* IN */
                   char **arg,         /* IN */
                   bool *swallow_next, /* IN */
                   Error *error)       /* OUT */
{
   const char *str;
   int val;

   ASSERT (entry);
   ASSERT (arg);

   switch (entry->arg) {
   case OPTION_ARG_NONE:
      *((bool *)entry->arg_data) = true;
      break;
   case OPTION_ARG_STRING:
      if ((str = strchr (arg[0], '='))) {
         str++;
         *(const char **)entry->arg_data = str;
         *swallow_next = false;
      } else if (arg [1]) {
         *(const char **)entry->arg_data = arg [1];
         *swallow_next = true;
      } else {
         OptionEntry_ErrorMissingArg (entry, error);
         return false;
      }
      break;
   case OPTION_ARG_FILENAME:
      /* TODO */
      break;
   case OPTION_ARG_INT:
      str = NULL;
      if ((str = strchr (arg[0], '='))) {
         str++;
         *swallow_next = false;
      } else if (arg [1]) {
         str = arg [1];
         *swallow_next = true;
      }
      if (str) {
         val = strtol (str, NULL, 10);
         if (((val == LONG_MAX) || (val == LONG_MIN)) && (errno == ERANGE)) {
            Error_Init (error,
                        OPTION_ENTRY_ERROR,
                        OPTION_ENTRY_ERROR_INVALID_INT,
                        "Failed to parse \"%s\" as an integer.",
                        str);
            return false;
         }
         *(int *)entry->arg_data = val;
      } else {
         OptionEntry_ErrorMissingArg (entry, error);
         return false;
      }
      break;
   default:
      ASSERT (false);
      return false;
   }

   return true;
}

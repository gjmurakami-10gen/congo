/* OptionContext.c
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

#include <stdio.h>
#include <stdlib.h>

#include <Debug.h>
#include <Memory.h>
#include <OptionContext.h>


static OptionEntry gHelp = {
   "help", 'h', 0, OPTION_ARG_NONE, NULL,
   "Show this help menu." };
static OptionEntry gVersion = {
   "version", 0, 0, OPTION_ARG_NONE, NULL,
   "Print verison information and exit." };


void
OptionContext_Init (OptionContext *context,  /* OUT */
                    const char *prgname,     /* IN */
                    const char *description) /* IN */
{
   ASSERT (context);
   Memory_Zero (context, sizeof *context);
   context->prgname = prgname;
   context->description = description;
}


void
OptionContext_AddEntries (OptionContext *context, /* IN */
                          OptionEntry *entries,   /* IN */
                          int n_entries)          /* IN */
{
   ASSERT (context);

   context->entries = entries;
   context->n_entries = n_entries;
}


void
OptionContext_Destroy (OptionContext *context) /* IN */
{
   ASSERT (context);
}


static void
OptionContext_ShowHelpEntry (const OptionContext *context, /* IN */
                             FILE *stream,                 /* IN */
                             const OptionEntry *entry)     /* IN */
{
   int spaces;
   int i;

   fprintf (stream, "  ");

   if (entry->short_name) {
      fprintf (stream, "-%c", entry->short_name);
      if (entry->long_name) {
         fprintf (stream, ", ");
      }
   } else {
      fprintf (stream, "    ");
   }

   spaces = 20;

   if (entry->long_name) {
      spaces -= strlen (entry->long_name) + 1;
      fprintf (stream, "--%s", entry->long_name);
      if (entry->arg != OPTION_ARG_NONE) {
         spaces--;
         fprintf (stream, "=");
      }
   } else {
      spaces += 3;
   }

   for (i = 0; i < spaces; i++) {
      fprintf (stream, " ");
   }

   if (entry->description) {
      fprintf (stream, " %s", entry->description);
   }

   fprintf (stream, "\n");
}


void
OptionContext_ShowHelp (OptionContext *context, /* IN */
                        FILE *stream)           /* IN */
{
   int i;

   ASSERT (context);
   ASSERT (stream);

   fprintf (stream, "Usage:\n");
   fprintf (stream, "  %s [OPTION...]\n", context->prgname);
   fprintf (stream, "\n");
   fprintf (stream, "  %s\n", context->description);
   fprintf (stream, "\n");
   fprintf (stream, "Help Options:\n");
   OptionContext_ShowHelpEntry (context, stream, &gHelp);
   OptionContext_ShowHelpEntry (context, stream, &gVersion);
   fprintf (stream, "\n");

   if (context->n_entries) {
      fprintf (stream, "Application Options:\n");
      for (i = 0; i < context->n_entries; i++) {
         OptionContext_ShowHelpEntry (context, stream, &context->entries [i]);
      }
   }

   fprintf (stream, "\n");
}


static void
OptionContext_ShowVersion (OptionContext *context, /* IN */
                           FILE *stream)           /* IN */
{
   fprintf (stream, "%s %s\n", context->prgname, VERSION);
}


bool
OptionContext_Parse (OptionContext *context, /* IN */
                     int argc,               /* IN */
                     char **argv,            /* IN */
                     Error *error)           /* OUT */
{
   bool swallow_next;
   int i;
   int j;

   ASSERT (context);
   ASSERT (argc);
   ASSERT (argv);

   if (!context->prgname) {
      context->prgname = argv [0];
   }

   for (i = 1; i < argc; i++) {
      if (OptionEntry_Matches (&gHelp, argv [i])) {
         OptionContext_ShowHelp (context, stdout);
         exit (0);
      } else if (OptionEntry_Matches (&gVersion, argv [i])) {
         OptionContext_ShowVersion (context, stdout);
         exit (0);
      }

      for (j = 0; (i < argc) && (j < context->n_entries); j++) {
         if (OptionEntry_Matches (&context->entries [j], argv [i])) {
            swallow_next = false;
            if (!OptionEntry_Parse (&context->entries [j], &argv [i], &swallow_next, error)) {
               return false;
            }
            if (swallow_next) {
               i++;
            }
            break;
         }
      }
   }

   return true;
}

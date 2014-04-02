/* congo-ping.c
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

#include <bson.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <Connection.h>
#include <OptionContext.h>
#include <Sched.h>
#include <Task.h>
#include <TimeSpec.h>
#include <WireProtocol.h>


static char *gHost = "127.0.0.1";
static int gPort = 27017;
static OptionEntry entries[] = {
   { "host", 0, 0, OPTION_ARG_STRING, &gHost, "The host to ping." },
   { "port", 'p', 0, OPTION_ARG_INT, &gPort, "The port to ping." },
};


static void
PrintResult (WireProtocolReply *reply, /* IN */
             double msec)              /* IN */
{
   bson_reader_t *reader;
   const bson_t *doc;
   char *str;

   reader = bson_reader_new_from_data (reply->documents, reply->documents_len);
   if ((doc = bson_reader_read (reader, NULL))) {
      str = bson_as_json (doc, NULL);
      fprintf (stdout, "%u bytes from (%s:%u): time=%0.3lf ms: %s\n",
               reply->msg_len, gHost, gPort, msec, str);
      bson_free (str);
   }
   bson_reader_destroy (reader);
}


static void
PingTask (void *data) /* UNUSED */
{
   WireProtocolMessage reply;
   Connection conn;
   uint64_t begin;
   uint64_t end;
   double msec;

   for (;; sleep (1)) {
      begin = TimeSpec_GetMonotonic ();

      if (Connection_InitFromHost (&conn, gHost, gPort)) {
         if (Connection_Ping (&conn, &reply)) {
            end = TimeSpec_GetMonotonic ();
            msec = (end - begin) / 1000.0;
            PrintResult (&reply.reply, msec);
         } else {
            fprintf (stderr, "congo-ping: no response from %s:%u\n",
                     gHost, gPort);
         }

         Connection_Destroy (&conn);
      } else {
         fprintf (stderr, "congo-ping: failed to connect to %s:%d\n",
                  gHost, gPort);
      }
   }
}


int
main (int argc,      /* IN */
      char *argv[])  /* IN */
{
   OptionContext context;
   Error error;
   Task task;

   OptionContext_Init (&context, "congo-ping", "Ping a congo server.");
   OptionContext_AddEntries (&context, entries, N_ELEMENTS (entries));
   if (!OptionContext_Parse (&context, argc, argv, &error)) {
      fprintf (stderr, "%s\n", error.message);
      return EXIT_FAILURE;
   }

   Task_Create (&task, PingTask, NULL);

   Sched_Run ();

   OptionContext_Destroy (&context);

   return EXIT_SUCCESS;
}

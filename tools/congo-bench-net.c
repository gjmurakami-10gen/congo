/* congo-bench-net.c
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
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include <Array.h>
#include <Atomic.h>
#include <Connection.h>
#include <Counter.h>
#include <CString.h>
#include <Debug.h>
#include <File.h>
#include <Macros.h>
#include <OptionContext.h>
#include <OptionEntry.h>
#include <Random.h>
#include <Resolver.h>
#include <Sched.h>
#include <Socket.h>
#include <Task.h>
#include <TimeSpec.h>
#include <Types.h>
#include <WireProtocol.h>


COUNTER (Completed, "Net", "Completed", "Number of requests completed.")
COUNTER (Failed,    "Net", "Failed",    "Number of requests failed.")
COUNTER (Ingress,   "Net", "Ingress",   "Number of bytes ingress.")
COUNTER (Egress,    "Net", "Egress",    "Number of bytes egress.")
COUNTER (MsgSent,   "Net", "MsgSent",   "Number of messages sent.")
COUNTER (MsgRecv,   "Net", "MsgRecv",   "Number of messages received.")


typedef struct
{
   int32_t     type;
   int32_t     flags;
   bson_t     *selector;
   bson_t     *fields;
   bson_t     *update;
   bson_t     *document;
   bson_t     *gle;
   const char *collection;
   const char *cmdname;
} Op;


static Array     gOperations = ARRAY_INITIALIZER (Op);
static int       gOpNext;
static int       gCount;
static char     *gHost = "localhost";
static char     *gQuery;
static char     *gCollection = "test.test";
static char     *gVersion;
static char     *gConfig;
static bson_t    gQueryBson;
static int       gPort = 27017;
static int       gConcurrent = 1;
static int       gRequests = 1;
static int       gActive;
static bool      gIgnoreSockErr = true;
static bool      gSsl;
static bool      gQueryVersion;
static bool      gKeepalive;
static uint64_t  gBeginTime;
static uint64_t  gEndTime;
static uint64_t  gTimeout;


static OptionEntry entries[] = {
   { "config", 0, 0, OPTION_ARG_STRING, &gConfig,
     "An optional configuration file for settings." },
   { "host", 0, 0, OPTION_ARG_STRING, &gHost,
     "The hostname to connect in client mode [localhost]" },
   { "port", 0, 0, OPTION_ARG_INT, &gPort,
     "The port to connect in client mode [27017]" },
   { "concurrency", 'c', 0, OPTION_ARG_INT, &gConcurrent,
     "The number of concurrent requests [1]" },
   { "requests", 'n', 0, OPTION_ARG_INT, &gRequests,
     "The total number of requests to perform [1]" },
   { "ssl", 'z', 0, OPTION_ARG_NONE, &gSsl,
     "Use TLS (commonly referred to as SSL) to communicate" },
   { NULL, 'r', 0, OPTION_ARG_NONE, &gIgnoreSockErr,
     "Don't exit on socket receive errors." },
   { "timeout", 's', 0, OPTION_ARG_INT, &gTimeout,
     "Seconds to wait for reply from server before failure" },
   { NULL, 'k', 0, OPTION_ARG_NONE, &gKeepalive,
     "Reuse sockets between requests" },
   { "query", 'q', 0, OPTION_ARG_STRING, &gQuery,
     "Specify a JSON query to request and exhaust" },
   { "collection", 0, 0, OPTION_ARG_STRING, &gCollection,
     "The collection to query with --query" },
};


static bool
Op_InitInsert (Op *op,            /* OUT */
               bson_iter_t *iter) /* IN */
{
   const uint8_t *data;
   const char *key;
   uint32_t datalen;
   char scratch[128];
   char *copy;
   char *s;

   ASSERT (op);
   ASSERT (iter);

   op->type = WIRE_PROTOCOL_INSERT;

   while (bson_iter_next (iter)) {
      key = bson_iter_key (iter);

      if (0 == strcmp (key, "collection")) {
         op->collection = bson_iter_utf8 (iter, NULL);
         if (!(s = strrchr (op->collection, '.'))) {
            return false;
         }
         copy = CString_NDup (op->collection, s - op->collection);
         snprintf (scratch, sizeof scratch, "%s.$cmd", copy);
         scratch [sizeof scratch - 1] = '\0';
         Memory_Free (copy);
         op->cmdname = CString_Dup (scratch);
      } else if (0 == strcmp (key, "document")) {
         bson_iter_document (iter, &datalen, &data);
         op->document = bson_new_from_data (data, datalen);
         if (!op->document) {
            return false;
         }
      } else if (0 == strcmp (key, "gle")) {
         bson_iter_document (iter, &datalen, &data);
         op->gle = bson_new_from_data (data, datalen);
         if (!op->gle) {
            return false;
         }
      }
   }

   return true;
}


static bool
Op_InitDelete (Op *op,            /* OUT */
               bson_iter_t *iter) /* IN */
{
   const uint8_t *data;
   const char *key;
   uint32_t datalen;
   char scratch[128];
   char *copy;
   char *s;

   ASSERT (op);
   ASSERT (iter);

   op->type = WIRE_PROTOCOL_DELETE;

   while (bson_iter_next (iter)) {
      key = bson_iter_key (iter);

      if (0 == strcmp (key, "collection")) {
         op->collection = bson_iter_utf8 (iter, NULL);
         if (!(s = strrchr (op->collection, '.'))) {
            return false;
         }
         copy = CString_NDup (op->collection, s - op->collection);
         snprintf (scratch, sizeof scratch, "%s.$cmd", copy);
         scratch [sizeof scratch - 1] = '\0';
         Memory_Free (copy);
         op->cmdname = CString_Dup (scratch);
      } else if (0 == strcmp (key, "selector")) {
         bson_iter_document (iter, &datalen, &data);
         op->selector = bson_new_from_data (data, datalen);
         if (!op->selector) {
            return false;
         }
      } else if (0 == strcmp (key, "multi")) {
         if (!bson_iter_as_bool (iter)) {
            op->flags = WIRE_PROTOCOL_DELETE_SINGLE_REMOVE;
         }
      } else if (0 == strcmp (key, "gle")) {
         bson_iter_document (iter, &datalen, &data);
         op->gle = bson_new_from_data (data, datalen);
         if (!op->gle) {
            return false;
         }
      }
   }

   return true;
}


static bool
Op_InitUpdate (Op *op,            /* OUT */
               bson_iter_t *iter) /* IN */
{
   const uint8_t *data;
   const char *key;
   uint32_t datalen;
   char scratch[128];
   char *copy;
   char *s;

   ASSERT (op);
   ASSERT (iter);

   op->type = WIRE_PROTOCOL_UPDATE;

   while (bson_iter_next (iter)) {
      key = bson_iter_key (iter);

      if (0 == strcmp (key, "collection")) {
         op->collection = bson_iter_utf8 (iter, NULL);
         if (!(s = strrchr (op->collection, '.'))) {
            return false;
         }
         copy = CString_NDup (op->collection, s - op->collection);
         snprintf (scratch, sizeof scratch, "%s.$cmd", copy);
         scratch [sizeof scratch - 1] = '\0';
         op->cmdname = CString_Dup (scratch);
         Memory_Free (copy);
      } else if (0 == strcmp (key, "selector")) {
         bson_iter_document (iter, &datalen, &data);
         op->selector = bson_new_from_data (data, datalen);
         if (!op->selector) {
            return false;
         }
      } else if (0 == strcmp (key, "update")) {
         bson_iter_document (iter, &datalen, &data);
         op->update = bson_new_from_data (data, datalen);
         if (!op->update) {
            return false;
         }
      } else if (0 == strcmp (key, "multi")) {
         if (bson_iter_as_bool (iter)) {
            op->flags |= WIRE_PROTOCOL_UPDATE_MULTI_UPDATE;
         }
      } else if (0 == strcmp (key, "upsert")) {
         if (bson_iter_as_bool (iter)) {
            op->flags |= WIRE_PROTOCOL_UPDATE_UPSERT;
         }
      } else if (0 == strcmp (key, "gle")) {
         bson_iter_document (iter, &datalen, &data);
         op->gle = bson_new_from_data (data, datalen);
         if (!op->gle) {
            return false;
         }
      }
   }

   return true;
}


static bool
Op_InitQuery (Op *op,            /* OUT */
              bson_iter_t *iter) /* IN */
{
   const uint8_t *data;
   const char *key;
   uint32_t datalen;

   ASSERT (op);
   ASSERT (iter);

   op->type = WIRE_PROTOCOL_QUERY;

   while (bson_iter_next (iter)) {
      key = bson_iter_key (iter);

      if (0 == strcmp (key, "collection")) {
         op->collection = bson_iter_utf8 (iter, NULL);
      } else if (0 == strcmp (key, "selector")) {
         bson_iter_document (iter, &datalen, &data);
         op->selector = bson_new_from_data (data, datalen);
         if (!op->selector) {
            return false;
         }
      } else if (0 == strcmp (key, "fields")) {
         bson_iter_document (iter, &datalen, &data);
         op->fields = bson_new_from_data (data, datalen);
         if (!op->fields) {
            return false;
         }
      } else if (0 == strcmp (key, "gle")) {
         bson_iter_document (iter, &datalen, &data);
         op->gle = bson_new_from_data (data, datalen);
         if (!op->gle) {
            return false;
         }
      }
   }

   return true;
}


static bool
Op_Init (Op *op,            /* OUT */
         bson_iter_t *iter) /* IN */
{
   bson_iter_t copy;
   const char *str;

   ASSERT (op);
   ASSERT (iter);

   memcpy (&copy, iter, sizeof *iter);

   memset (op, 0, sizeof *op);

   if (bson_iter_find (&copy, "type") &&
       BSON_ITER_HOLDS_UTF8 (&copy)) {
      str = bson_iter_utf8 (&copy, NULL);
      if (0 == strcmp (str, "insert")) {
         return Op_InitInsert (op, iter);
      } else if (0 == strcmp (str, "delete")) {
         return Op_InitDelete (op, iter);
      } else if (0 == strcmp (str, "query")) {
         return Op_InitQuery (op, iter);
      } else if (0 == strcmp (str, "update")) {
         return Op_InitUpdate (op, iter);
      }
   }

   printf ("no type field\n");

   return false;
}


static bool
Op_RunQuery (Op *op,           /* IN */
             Connection *conn) /* IN */
{
   ConnectionCursor cursor;
   const bson_t *doc;
   bool ret;

   Connection_Query (conn,
                     op->collection,
                     op->selector,
                     op->fields,
                     &cursor);

   while (ConnectionCursor_MoveNext (&cursor, &doc)) {
      /* Do Nothing */
   }

   ret = !ConnectionCursor_HasError (&cursor);
   ConnectionCursor_Destroy (&cursor);

   return ret;
}


static bool
Op_RunUpdate (Op *op,           /* IN */
              Connection *conn) /* IN */
{
   Error error;

   if (!Connection_Update (conn, op->collection, op->flags,
                           op->selector, op->update)) {
      return false;
   }

   if (op->gle) {
      if (!Connection_GetLastError (conn, op->cmdname, op->gle, &error)) {
         return false;
      }
   }

   return true;
}


static bool
Op_RunDelete (Op *op,           /* IN */
              Connection *conn) /* IN */
{
   Error error;

   if (!Connection_Delete (conn, op->collection, op->flags, op->selector)) {
      return false;
   }

   if (op->gle) {
      if (!Connection_GetLastError (conn, op->cmdname, op->gle, &error)) {
         return false;
      }
   }

   return true;
}


static bool
Op_RunInsert (Op *op,           /* IN */
              Connection *conn) /* IN */
{
   Error error;

   if (!Connection_Insert (conn, op->collection, op->flags, op->document)) {
      return false;
   }

   if (op->gle) {
      if (!Connection_GetLastError (conn, op->cmdname, op->gle, &error)) {
         return false;
      }
   }

   return true;
}


static bool
Op_Run (Op *op,           /* IN */
        Connection *conn) /* IN */
{
   switch (op->type) {
   case WIRE_PROTOCOL_QUERY:
      return Op_RunQuery (op, conn);
   case WIRE_PROTOCOL_UPDATE:
      return Op_RunUpdate (op, conn);
   case WIRE_PROTOCOL_INSERT:
      return Op_RunInsert (op, conn);
   case WIRE_PROTOCOL_DELETE:
      return Op_RunDelete (op, conn);
   default:
      break;
   }

   return true;
}


static Op *
Op_Random (void)
{
   int i;

   if (gOperations.len) {
      i = ++gOpNext % gOperations.len;
      return &Array_Index (&gOperations, Op, i);
   }

   return NULL;
}


static bool
RunQuery (Connection *conn) /* IN */
{
   ConnectionCursor cursor;
   const bson_t *doc;
   bool ret;

   Connection_Query (conn, gCollection, &gQueryBson, NULL, &cursor);
   while (ConnectionCursor_MoveNext (&cursor, &doc)) {
      /* Do Nothing */
   }
   ret = !ConnectionCursor_HasError (&cursor);
   ConnectionCursor_Destroy (&cursor);

   return ret;
}


static bool
RunOp (Connection *conn) /* IN */
{
   Op *op = Op_Random ();

   ASSERT (conn);

   if (op) {
      return Op_Run (op, conn);
   }

   return true;
}


/*
 *--------------------------------------------------------------------------
 *
 * BenchNet_Client --
 *
 *       This is a coroutine task that will continuely connect to a
 *       mongod instance and benchmark the socket performance.
 *
 *       If -k was specified on the command line, then the socket will
 *       be reused between operations. Otherwise, a new socket will
 *       be used for every request.
 *
 *       Various counters will be incremented during the process of
 *       this coroutine. Ingress and Egress bytes are updated after
 *       every connection and message attempt.
 *
 *       If there was a socket error and we were not instructed to
 *       ignore them, the coroutine will call exit() to end the process.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static void
BenchNet_Client (void *data) /* UNUSED */
{
   WireProtocolMessage reply;
   Connection conn;
   bool socket_valid = false;

   while (AtomicInt_Decrement (&gCount) >= 0) {
      if (socket_valid || Connection_InitFromHost (&conn, gHost, gPort)) {
         Connection_SetTimeout (&conn, gTimeout);
         if (UNLIKELY (!gQueryVersion)) {
            gQueryVersion = true;
            gVersion = Connection_ServerVersionString (&conn);
            if (!gVersion) {
               gQueryVersion = false;
            }
         }
         if (Connection_IsMaster (&conn, &reply) &&
             Connection_Ping (&conn, &reply) &&
             (!gQuery || RunQuery (&conn)) &&
             (!gConfig || RunOp (&conn))) {
            Completed_Increment ();
            socket_valid = true;
         } else {
            Failed_Increment ();
            socket_valid = false;
         }

         Ingress_Add (conn.bytes_recv);
         Egress_Add (conn.bytes_sent);
         MsgRecv_Add (conn.msg_recv);
         MsgSent_Add (conn.msg_sent);

         conn.bytes_sent = 0;
         conn.bytes_recv = 0;
         conn.msg_sent = 0;
         conn.msg_recv = 0;

         if (!gKeepalive) {
            Connection_Destroy (&conn);
            socket_valid = false;
         }
      } else if (!gIgnoreSockErr) {
         fprintf (stderr, "Socket failure, use -r to ignore failures.\n");
         exit (EXIT_FAILURE);
      } else {
         socket_valid = false;
         Failed_Increment ();
      }
   }

   if (AtomicInt_DecrementAndTest (&gActive)) {
      Sched_Quit ();
   }
}


static void
PrintHeader (void)
{
   fprintf (stdout,
            "This is CongoBenchNet, Version "VERSION"\n"
            "Copyright 2014 MongoDB, Inc. https://mongodb.com/\n"
            "Licensed under the AGPL v3 or newer.\n"
            "\n"
            "Benchmarking %s (be patient).....",
            gHost);
   fflush (stdout);
}


/*
 *--------------------------------------------------------------------------
 *
 * FormatBytes --
 *
 *       Format an output string converting bytes into Gigabytes,
 *       Megabytes, or Kilobytes.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @str is initialized.
 *
 *--------------------------------------------------------------------------
 */

static void
FormatBytes (char *str,     /* OUT */
             size_t size,   /* IN */
             int64_t bytes) /* IN */
{
   const char *bstr = "";
   double bytes_dbl;

   if (bytes > (1024 * 1024 * 1024)) {
      bstr = "Gbytes";
      bytes_dbl = bytes / (double)(1024 * 1024 * 1024);
   } else if (bytes > (1024 * 1024)) {
      bstr = "Mbytes";
      bytes_dbl = bytes / (double)(1024 * 1024);
   } else if (bytes > 1024) {
      bstr = "Kbytes";
      bytes_dbl = bytes / 1024.0;
   } else {
      bstr = "Bytes";
      bytes_dbl = bytes;
   }

   snprintf (str, size, "%0.5lf %s", bytes_dbl, bstr);
   str [size - 1] = '\0';
}


/*
 *--------------------------------------------------------------------------
 *
 * PrintReport --
 *
 *       Print the benchmark report using the various counters recorded.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static void
PrintReport (void)
{
   char format [32];
   double elapsed;

   elapsed = (gEndTime - gBeginTime) / (double)USEC_PER_SEC;

   fprintf (stdout, "\n");
   if (gQueryVersion) {
      fprintf (stdout, "%-24s%s\n", "Server Software:", gVersion);
   } else {
      fprintf (stdout, "%-24s%s\n", "Server Software:", "Unknown");
   }
   fprintf (stdout, "%-24s%s\n", "Server Hostname:", gHost);
   fprintf (stdout, "%-24s%d\n", "Server Port:", gPort);
   fprintf (stdout, "\n");
   fprintf (stdout, "%-24s%d\n", "Concurrency Level:", gConcurrent);
   fprintf (stdout, "%-24s%0.3lf seconds\n", "Time taken for tests:", elapsed);
   fprintf (stdout, "%-24s%"PRIu64"\n", "Completed Requests:", Completed_Get ());
   fprintf (stdout, "%-24s%"PRIu64"\n", "Failed Requests:", Failed_Get ());
   FormatBytes (format, sizeof format, Egress_Get ());
   fprintf (stdout, "%-24s%s sent\n", "Transfered:", format);
   FormatBytes (format, sizeof format, Ingress_Get ());
   fprintf (stdout, "%-24s%s received\n", "Transfered:", format);
   fprintf (stdout, "%-24s%0.5lf [sec] (mean)\n", "Time per Request:",
            elapsed / (double)gRequests);
   fprintf (stdout, "%-24s%0.5lf (completed)\n", "Requests per Second:",
            Completed_Get () / elapsed);
   FormatBytes (format, sizeof format, Egress_Get () / elapsed);
   fprintf (stdout, "%-24s%s/sec sent\n", "Transfer rate:", format);
   FormatBytes (format, sizeof format, Ingress_Get () / elapsed);
   fprintf (stdout, "%-24s%s/sec received\n", "Transfer rate:", format);
   fprintf (stdout, "%-24s%"PRIi64"\n", "Messages Sent:", MsgSent_Get ());
   fprintf (stdout, "%-24s%"PRIi64"\n", "Messages Received:", MsgRecv_Get ());
   fprintf (stdout, "\n");
   if (gQuery) {
      fprintf (stdout, "%-24s%s\n", "Query:", gCollection);
      fprintf (stdout, "%-24s%s\n", "Collection:", gQuery);
      fprintf (stdout, "\n");
   }
}


int
main (int argc,     /* IN */
      char *argv[]) /* IN */
{
   OptionContext context;
   bson_error_t berror;
   struct stat st;
   bson_iter_t iter;
   bson_iter_t ar;
   bson_iter_t child;
   Error error;
   bson_t config;
   size_t buflen;
   char *buf;
   bool release_config = false;
   Task task;
   File fd;
   int i;
   Op op;

   Counters_Init ();
   Random_Init ();
   Sched_Create ();

   OptionContext_Init (&context,
                       "congo-bench-net",
                       "A network benchmarking tool.");
   OptionContext_AddEntries (&context, entries, N_ELEMENTS (entries));
   if (!OptionContext_Parse (&context, argc, argv, &error)) {
      fprintf (stderr, "%s\n", error.message);
      return EXIT_FAILURE;
   }

   if (gQuery) {
      if (!bson_init_from_json (&gQueryBson, gQuery, -1, &berror)) {
         fprintf (stderr, "Invalid JSON: %s\n", berror.message);
         return EXIT_FAILURE;
      }
   }

   if (gConfig) {
      fd = open (gConfig, O_RDONLY, 0);
      if (fd == FILE_INVALID) {
         fprintf (stderr, "Failed to open file %s\n", gConfig);
         return EXIT_FAILURE;
      }
      if (0 != fstat (fd, &st)) {
         fprintf (stderr, "Failed to stat %s\n", gConfig);
         return EXIT_FAILURE;
      }
      buflen = st.st_size;
      buf = Memory_SafeMalloc (buflen);
      if (read (fd, buf, buflen) != buflen) {
         fprintf (stderr, "Failed to read %d bytes.\n", (int)buflen);
         return EXIT_FAILURE;
      }
      if (!bson_init_from_json (&config, buf, buflen, &berror)) {
         fprintf (stderr, "Failed to parse JSON config: %s\n",
                  berror.message);
         return EXIT_FAILURE;
      }
      Memory_Free (buf);

      release_config = true;

      if (bson_iter_init_find (&iter, &config, "concurrency") &&
          BSON_ITER_HOLDS_INT32 (&iter)) {
         gConcurrent = bson_iter_int32 (&iter);
      }

      if (bson_iter_init_find (&iter, &config, "requests") &&
          BSON_ITER_HOLDS_INT32 (&iter)) {
         gRequests = bson_iter_int32 (&iter);
      }

      if (bson_iter_init_find (&iter, &config, "keepalive") &&
          BSON_ITER_HOLDS_BOOL (&iter)) {
         gKeepalive = bson_iter_bool (&iter);
      }

      if (bson_iter_init_find (&iter, &config, "host") &&
          BSON_ITER_HOLDS_UTF8 (&iter)) {
         gHost = (char *)bson_iter_utf8 (&iter, NULL);
      }

      if (bson_iter_init_find (&iter, &config, "port") &&
          BSON_ITER_HOLDS_UTF8 (&iter)) {
         gPort = bson_iter_int32 (&iter);
      }

      if (bson_iter_init_find (&iter, &config, "operations") &&
          BSON_ITER_HOLDS_ARRAY (&iter) &&
          bson_iter_recurse (&iter, &ar)) {
         while (bson_iter_next (&ar)) {
            if (BSON_ITER_HOLDS_DOCUMENT (&ar) &&
                bson_iter_recurse (&ar, &child)) {
               if (Op_Init (&op, &child)) {
                  Array_Append (&gOperations, op);
               }
            }
         }
      }
   }

   gCount = gRequests;
   gActive = gConcurrent;

   PrintHeader ();

   for (i = 0; i < gConcurrent; i++) {
      Task_Create (&task, BenchNet_Client, NULL);
   }

   gBeginTime = TimeSpec_GetMonotonic ();
   Sched_Run ();
   gEndTime = TimeSpec_GetMonotonic ();

   fprintf (stdout, "done.\n\n");

   if (!Completed_Get ()) {
      fprintf (stderr, "Failed to connect to %s.\n\n", gHost);
   } else {
      PrintReport ();
   }

   bson_free (gVersion);

   if (release_config) {
      bson_destroy (&config);
   }

   return EXIT_SUCCESS;
}

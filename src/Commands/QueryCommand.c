/* QueryCommand.c
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


#include <Debug.h>
#include <Endian.h>
#include <Log.h>
#include <QueryCommand.h>


#undef LOG_DOMAIN
#define LOG_DOMAIN "Query"


static __inline__ uint32_t
GetBsonLength (const uint8_t *buf) /* IN */
{
   uint32_t len;
   memcpy (&len, buf, 4);
   return UINT32_FROM_LE (len);
}


static void
QueryCommand_Destroy (Command *command) /* IN */
{
   QueryCommand *q = (QueryCommand *)command;

   ASSERT (q);

   bson_destroy (&q->q);
   bson_destroy (&q->f);
}


static void
QueryCommand_Log (Command *command) /* IN */
{
   QueryCommand *q = (QueryCommand *)command;
   char *qstr;
   char *fstr;

   ASSERT (q);

   qstr = bson_as_json (&q->q, NULL);
   fstr = bson_as_json (&q->f, NULL);
   LOG_MESSAGE ("[%s]: query=%s fields=%s", q->client->name, qstr, fstr);
   bson_free (qstr);
   bson_free (fstr);
}


static bool
QueryCommand_Run (Command *command, /* IN */
                  Error *error)     /* IN */
{
   return false;
}


bool
QueryCommand_Init (QueryCommand *command,      /* OUT */
                   Socket *client,             /* IN */
                   WireProtocolReader *reader, /* IN */
                   void *writer,               /* IN */
                   WireProtocolQuery *query)   /* IN */
{
   bson_t *q;
   bson_t *f;
   size_t err;

   ASSERT (command);
   ASSERT (client);
   ASSERT (reader);
   ASSERT (query);

   command->parent.Run = QueryCommand_Run;
   command->parent.Log = QueryCommand_Log;
   command->parent.Destroy = QueryCommand_Destroy;

   command->client = client;
   command->reader = reader;
   command->writer = writer;
   command->query = query;

   q = &command->q;
   f = &command->f;

   if (!query->query ||
       !bson_init_static (q, query->query, GetBsonLength (query->query)) ||
       !bson_validate (q, (BSON_VALIDATE_UTF8 |
                           BSON_VALIDATE_UTF8_ALLOW_NULL),
                       &err)) {
      return false;
   }

   if (!query->fields ||
       !bson_init_static (f, query->fields, GetBsonLength (query->fields)) ||
       !bson_validate (f, (BSON_VALIDATE_UTF8 |
                           BSON_VALIDATE_UTF8_ALLOW_NULL |
                           BSON_VALIDATE_DOLLAR_KEYS),
                       &err)) {
      bson_init (f);
   }

   return true;
}

/* WireProtocol.c
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
#include <sys/types.h>
#include <sys/uio.h>

#include <Debug.h>
#include <Endian.h>
#include <Log.h>
#include <WireProtocol.h>


#define RPC(name, _code) \
   static __inline__ void \
   WireProtocol##name##_Gather (WireProtocol##name *rpc, \
                                Array *array) \
   { \
      struct iovec iov; \
      ASSERT (rpc); \
      ASSERT (array); \
      rpc->msg_len = 0; \
      _code \
   }
#define INT32_FIELD(_name) \
   iov.iov_base = (void *)&rpc->_name; \
   iov.iov_len = 4; \
   ASSERT (iov.iov_len); \
   rpc->msg_len += (int32_t)iov.iov_len; \
   Array_Append (array, iov);
#define INT64_FIELD(_name) \
   iov.iov_base = (void *)&rpc->_name; \
   iov.iov_len = 8; \
   ASSERT (iov.iov_len); \
   rpc->msg_len += (int32_t)iov.iov_len; \
   Array_Append (array, iov);
#define CSTRING_FIELD(_name) \
   ASSERT (rpc->_name); \
   iov.iov_base = (void *)rpc->_name; \
   iov.iov_len = strlen(rpc->_name) + 1; \
   ASSERT (iov.iov_len); \
   rpc->msg_len += (int32_t)iov.iov_len; \
   Array_Append (array, iov);
#define BSON_FIELD(_name) \
   do { \
      int32_t __l; \
      memcpy(&__l, rpc->_name, 4); \
      __l = UINT32_FROM_LE(__l); \
      iov.iov_base = (void *)rpc->_name; \
      iov.iov_len = __l; \
      ASSERT (iov.iov_len); \
      rpc->msg_len += (int32_t)iov.iov_len; \
      Array_Append (array, iov); \
   } while (0);
#define BSON_OPTIONAL(_check, _code) \
   if (rpc->_check) { _code }
#define BSON_ARRAY_FIELD(_name) \
   if (rpc->_name##_len) { \
      iov.iov_base = (void *)rpc->_name; \
      iov.iov_len = rpc->_name##_len; \
      ASSERT (iov.iov_len); \
      rpc->msg_len += (int32_t)iov.iov_len; \
      Array_Append (array, iov); \
   }
#define IOVEC_ARRAY_FIELD(_name) \
   do { \
      size_t _i; \
      ASSERT (rpc->n_##_name); \
      for (_i = 0; _i < rpc->n_##_name; _i++) { \
         ASSERT (rpc->_name[_i].iov_len); \
         rpc->msg_len += (int32_t)rpc->_name[_i].iov_len; \
         Array_Append (array, rpc->_name[_i]); \
      } \
   } while (0);
#define RAW_BUFFER_FIELD(_name) \
   iov.iov_base = (void *)rpc->_name; \
   iov.iov_len = rpc->_name##_len; \
   ASSERT (iov.iov_len); \
   rpc->msg_len += (int32_t)iov.iov_len; \
   Array_Append (array, iov);
#define INT64_ARRAY_FIELD(_len, _name) \
   iov.iov_base = (void *)&rpc->_len; \
   iov.iov_len = 4; \
   ASSERT (iov.iov_len); \
   rpc->msg_len += (int32_t)iov.iov_len; \
   Array_Append (array, iov); \
   iov.iov_base = (void *)rpc->_name; \
   iov.iov_len = rpc->_len * 8; \
   ASSERT (iov.iov_len); \
   rpc->msg_len += (int32_t)iov.iov_len; \
   Array_Append (array, iov);


#include "WireProtocolDelete.def"
#include "WireProtocolGetmore.def"
#include "WireProtocolInsert.def"
#include "WireProtocolKillCursors.def"
#include "WireProtocolMsg.def"
#include "WireProtocolQuery.def"
#include "WireProtocolReply.def"
#include "WireProtocolUpdate.def"


#undef RPC
#undef INT32_FIELD
#undef INT64_FIELD
#undef INT64_ARRAY_FIELD
#undef CSTRING_FIELD
#undef BSON_FIELD
#undef BSON_ARRAY_FIELD
#undef IOVEC_ARRAY_FIELD
#undef RAW_BUFFER_FIELD
#undef BSON_OPTIONAL


#define RPC(_name, _code) \
   static __inline__ void \
   WireProtocol##_name##_ToLe (WireProtocol##_name *rpc) \
   { \
      ASSERT (rpc); \
      _code \
   }
#define INT32_FIELD(_name) \
   rpc->_name = UINT32_FROM_LE(rpc->_name);
#define INT64_FIELD(_name) \
   rpc->_name = UINT64_FROM_LE(rpc->_name);
#define CSTRING_FIELD(_name)
#define BSON_FIELD(_name)
#define BSON_ARRAY_FIELD(_name)
#define IOVEC_ARRAY_FIELD(_name)
#define BSON_OPTIONAL(_check, _code) \
   if (rpc->_check) { _code }
#define RAW_BUFFER_FIELD(_name)
#define INT64_ARRAY_FIELD(_len, _name) \
   do { \
      size_t i; \
      for (i = 0; i < rpc->_len; i++) { \
         rpc->_name[i] = UINT64_FROM_LE(rpc->_name[i]); \
      } \
      rpc->_len = UINT32_FROM_LE(rpc->_len); \
   } while (0);


#include "WireProtocolDelete.def"
#include "WireProtocolGetmore.def"
#include "WireProtocolInsert.def"
#include "WireProtocolKillCursors.def"
#include "WireProtocolMsg.def"
#include "WireProtocolQuery.def"
#include "WireProtocolReply.def"
#include "WireProtocolUpdate.def"


#undef RPC
#undef INT64_ARRAY_FIELD

#define RPC(_name, _code) \
   static __inline__ void \
   WireProtocol##_name##_FromLe (WireProtocol##_name *rpc) \
   { \
      ASSERT (rpc); \
      _code \
   }
#define INT64_ARRAY_FIELD(_len, _name) \
   do { \
      size_t i; \
      rpc->_len = UINT32_FROM_LE(rpc->_len); \
      for (i = 0; i < rpc->_len; i++) { \
         rpc->_name[i] = UINT64_FROM_LE(rpc->_name[i]); \
      } \
   } while (0);


#include "WireProtocolDelete.def"
#include "WireProtocolGetmore.def"
#include "WireProtocolInsert.def"
#include "WireProtocolKillCursors.def"
#include "WireProtocolMsg.def"
#include "WireProtocolQuery.def"
#include "WireProtocolReply.def"
#include "WireProtocolUpdate.def"


#undef RPC
#undef INT32_FIELD
#undef INT64_FIELD
#undef INT64_ARRAY_FIELD
#undef CSTRING_FIELD
#undef BSON_FIELD
#undef BSON_ARRAY_FIELD
#undef IOVEC_ARRAY_FIELD
#undef BSON_OPTIONAL
#undef RAW_BUFFER_FIELD

#define RPC(_name, _code) \
   static __inline__ void \
   WireProtocol##_name##_Printf (WireProtocol##_name *rpc) \
   { \
      ASSERT (rpc); \
      _code \
   }
#define INT32_FIELD(_name) \
   printf("  "#_name" : %d\n", rpc->_name);
#define INT64_FIELD(_name) \
   printf("  "#_name" : %" PRIi64 "\n", (int64_t)rpc->_name);
#define CSTRING_FIELD(_name) \
   printf("  "#_name" : %s\n", rpc->_name);
#define BSON_FIELD(_name) \
   do { \
      bson_t b; \
      char *s; \
      int32_t __l; \
      memcpy(&__l, rpc->_name, 4); \
      __l = UINT32_FROM_LE(__l); \
      bson_init_static(&b, rpc->_name, __l); \
      s = bson_as_json(&b, NULL); \
      printf("  "#_name" : %s\n", s); \
      bson_free(s); \
      bson_destroy(&b); \
   } while (0);
#define BSON_ARRAY_FIELD(_name) \
   do { \
      bson_reader_t *__r; \
      bool __eof; \
      const bson_t *__b; \
      __r = bson_reader_new_from_data(rpc->_name, rpc->_name##_len); \
      while ((__b = bson_reader_read(__r, &__eof))) { \
         char *s = bson_as_json(__b, NULL); \
         printf("  "#_name" : %s\n", s); \
         bson_free(s); \
      } \
      bson_reader_destroy(__r); \
   } while (0);
#define IOVEC_ARRAY_FIELD(_name) \
   do { \
      size_t _i; \
      size_t _j; \
      for (_i = 0; _i < rpc->n_##_name; _i++) { \
         printf("  "#_name" : "); \
         for (_j = 0; _j < rpc->_name[_i].iov_len; _j++) { \
            uint8_t u; \
            u = ((char *)rpc->_name[_i].iov_base)[_j]; \
            printf(" %02x", u); \
         } \
         printf("\n"); \
      } \
   } while (0);
#define BSON_OPTIONAL(_check, _code) \
   if (rpc->_check) { _code }
#define RAW_BUFFER_FIELD(_name) \
   { \
      size_t __i; \
      printf("  "#_name" :"); \
      for (__i = 0; __i < rpc->_name##_len; __i++) { \
         uint8_t u; \
         u = ((char *)rpc->_name)[__i]; \
         printf(" %02x", u); \
      } \
      printf("\n"); \
   }
#define INT64_ARRAY_FIELD(_len, _name) \
   do { \
      size_t i; \
      for (i = 0; i < rpc->_len; i++) { \
         printf("  "#_name" : %" PRIi64 "\n", (int64_t)rpc->_name[i]); \
      } \
      rpc->_len = UINT32_FROM_LE(rpc->_len); \
   } while (0);


#include "WireProtocolDelete.def"
#include "WireProtocolGetmore.def"
#include "WireProtocolInsert.def"
#include "WireProtocolKillCursors.def"
#include "WireProtocolMsg.def"
#include "WireProtocolQuery.def"
#include "WireProtocolReply.def"
#include "WireProtocolUpdate.def"


#undef RPC
#undef INT32_FIELD
#undef INT64_FIELD
#undef INT64_ARRAY_FIELD
#undef CSTRING_FIELD
#undef BSON_FIELD
#undef BSON_ARRAY_FIELD
#undef IOVEC_ARRAY_FIELD
#undef BSON_OPTIONAL
#undef RAW_BUFFER_FIELD


#define RPC(_name, _code) \
   static bool \
   WireProtocol##_name##_Scatter (WireProtocol##_name *rpc, \
                                  const uint8_t *buf, \
                                  size_t buflen) \
   { \
      ASSERT (rpc); \
      ASSERT (buf); \
      ASSERT (buflen); \
      _code \
      return true; \
   }
#define INT32_FIELD(_name) \
   if (buflen < 4) { \
      return false; \
   } \
   memcpy(&rpc->_name, buf, 4); \
   buflen -= 4; \
   buf += 4;
#define INT64_FIELD(_name) \
   if (buflen < 8) { \
      return false; \
   } \
   memcpy(&rpc->_name, buf, 8); \
   buflen -= 8; \
   buf += 8;
#define INT64_ARRAY_FIELD(_len, _name) \
   do { \
      size_t needed; \
      if (buflen < 4) { \
         return false; \
      } \
      memcpy(&rpc->_len, buf, 4); \
      buflen -= 4; \
      buf += 4; \
      needed = UINT32_FROM_LE(rpc->_len) * 8; \
      if (needed > buflen) { \
         return false; \
      } \
      rpc->_name = (void *)buf; \
      buf += needed; \
      buflen -= needed; \
   } while (0);
#define CSTRING_FIELD(_name) \
   do { \
      size_t __i; \
      bool found = false; \
      for (__i = 0; __i < buflen; __i++) { \
         if (!buf[__i]) { \
            rpc->_name = (const char *)buf; \
            buflen -= __i + 1; \
            buf += __i + 1; \
            found = true; \
            break; \
         } \
      } \
      if (!found) { \
         return false; \
      } \
   } while (0);
#define BSON_FIELD(_name) \
   do { \
      int32_t __l; \
      if (buflen < 4) { \
         return false; \
      } \
      memcpy(&__l, buf, 4); \
      __l = UINT32_FROM_LE(__l); \
      if (__l < 5 || __l > buflen) { \
         return false; \
      } \
      rpc->_name = (uint8_t *)buf; \
      buf += __l; \
      buflen -= __l; \
   } while (0);
#define BSON_ARRAY_FIELD(_name) \
   rpc->_name = (uint8_t *)buf; \
   rpc->_name##_len = (int32_t)buflen; \
   buf = NULL; \
   buflen = 0;
#define BSON_OPTIONAL(_check, _code) \
   if (buflen) { \
      _code \
   }
#define IOVEC_ARRAY_FIELD(_name) \
   rpc->_name##_recv.iov_base = (void *)buf; \
   rpc->_name##_recv.iov_len = buflen; \
   rpc->_name = &rpc->_name##_recv; \
   rpc->n_##_name = 1; \
   buf = NULL; \
   buflen = 0;
#define RAW_BUFFER_FIELD(_name) \
   rpc->_name = (void *)buf; \
   rpc->_name##_len = (int32_t)buflen; \
   buf = NULL; \
   buflen = 0;


#include "WireProtocolDelete.def"
#include "WireProtocolGetmore.def"
#include "WireProtocolHeader.def"
#include "WireProtocolInsert.def"
#include "WireProtocolKillCursors.def"
#include "WireProtocolMsg.def"
#include "WireProtocolQuery.def"
#include "WireProtocolReply.def"
#include "WireProtocolUpdate.def"


#undef RPC
#undef INT32_FIELD
#undef INT64_FIELD
#undef INT64_ARRAY_FIELD
#undef CSTRING_FIELD
#undef BSON_FIELD
#undef BSON_ARRAY_FIELD
#undef IOVEC_ARRAY_FIELD
#undef BSON_OPTIONAL
#undef RAW_BUFFER_FIELD


void
WireProtocolMessage_Gather (WireProtocolMessage *rpc, /* IN */
                            Array *array)             /* IN */
{
   ASSERT (rpc);
   ASSERT (array);

   switch ((WireProtocolOpcode)rpc->header.opcode) {
   case WIRE_PROTOCOL_REPLY:
      WireProtocolReply_Gather (&rpc->reply, array);
      return;
   case WIRE_PROTOCOL_MSG:
      WireProtocolMsg_Gather (&rpc->msg, array);
      return;
   case WIRE_PROTOCOL_UPDATE:
      WireProtocolUpdate_Gather (&rpc->update, array);
      return;
   case WIRE_PROTOCOL_INSERT:
      WireProtocolInsert_Gather (&rpc->insert, array);
      return;
   case WIRE_PROTOCOL_QUERY:
      WireProtocolQuery_Gather (&rpc->query, array);
      return;
   case WIRE_PROTOCOL_GETMORE:
      WireProtocolGetmore_Gather (&rpc->getmore, array);
      return;
   case WIRE_PROTOCOL_DELETE:
      WireProtocolDelete_Gather (&rpc->delete, array);
      return;
   case WIRE_PROTOCOL_KILL_CURSORS:
      WireProtocolKillCursors_Gather (&rpc->kill_cursors, array);
      return;
   default:
      LOG_WARNING ("Unknown rpc type: 0x%08x", rpc->header.opcode);
      break;
   }
}


void
WireProtocolMessage_ToLe (WireProtocolMessage *rpc) /* IN */
{
#if BSON_BYTE_ORDER != BSON_LITTLE_ENDIAN
   WireProtocolOpcode opcode;

   ASSERT(rpc);

   opcode = rpc->header.opcode;

   switch (opcode) {
   case WIRE_PROTOCOL_REPLY:
      WireProtocolReply_ToLe (&rpc->reply);
      break;
   case WIRE_PROTOCOL_MSG:
      WireProtocolMsg_ToLe (&rpc->msg);
      break;
   case WIRE_PROTOCOL_UPDATE:
      WireProtocolUpdate_ToLe (&rpc->update);
      break;
   case WIRE_PROTOCOL_INSERT:
      WireProtocolInsert_ToLe (&rpc->insert);
      break;
   case WIRE_PROTOCOL_QUERY:
      WireProtocolQuery_ToLe (&rpc->query);
      break;
   case WIRE_PROTOCOL_GETMORE:
      WireProtocolGetmore_ToLe (&rpc->getmore);
      break;
   case WIRE_PROTOCOL_DELETE:
      WireProtocolDelete_ToLe (&rpc->delete);
      break;
   case WIRE_PROTOCOL_KILL_CURSORS:
      WireProtocolKillCursors_ToLe (&rpc->kill_cursors);
      break;
   default:
      LOG_WARNING ("Unknown rpc type: 0x%08x", opcode);
      break;
   }
#endif
}


void
WireProtocolMessage_FromLe (WireProtocolMessage *rpc) /* IN */
{
#if BSON_BYTE_ORDER != BSON_LITTLE_ENDIAN
   WireProtocolOpcode opcode;

   ASSERT(rpc);

   opcode = UINT32_FROM_LE(rpc->header.opcode);

   switch (opcode) {
   case WIRE_PROTOCOL_REPLY:
      WireProtocolReply_FromLe (&rpc->reply);
      break;
   case WIRE_PROTOCOL_MSG:
      WireProtocolMsg_FromLe (&rpc->msg);
      break;
   case WIRE_PROTOCOL_UPDATE:
      WireProtocolUpdate_FromLe (&rpc->update);
      break;
   case WIRE_PROTOCOL_INSERT:
      WireProtocolInsert_FromLe (&rpc->insert);
      break;
   case WIRE_PROTOCOL_QUERY:
      WireProtocolQuery_FromLe (&rpc->query);
      break;
   case WIRE_PROTOCOL_GETMORE:
      WireProtocolGetmore_FromLe (&rpc->getmore);
      break;
   case WIRE_PROTOCOL_DELETE:
      WireProtocolDelete_FromLe (&rpc->delete);
      break;
   case WIRE_PROTOCOL_KILL_CURSORS:
      WireProtocolKillCursors_FromLe (&rpc->kill_cursors);
      break;
   default:
      LOG_WARNING ("Unknown rpc type: 0x%08x", rpc->header.opcode);
      break;
   }
#endif
}


void
WireProtocolMessage_Printf (WireProtocolMessage *rpc) /* IN */
{
   ASSERT(rpc);

   switch ((WireProtocolOpcode)rpc->header.opcode) {
   case WIRE_PROTOCOL_REPLY:
      WireProtocolReply_Printf (&rpc->reply);
      break;
   case WIRE_PROTOCOL_MSG:
      WireProtocolMsg_Printf (&rpc->msg);
      break;
   case WIRE_PROTOCOL_UPDATE:
      WireProtocolUpdate_Printf (&rpc->update);
      break;
   case WIRE_PROTOCOL_INSERT:
      WireProtocolInsert_Printf (&rpc->insert);
      break;
   case WIRE_PROTOCOL_QUERY:
      WireProtocolQuery_Printf (&rpc->query);
      break;
   case WIRE_PROTOCOL_GETMORE:
      WireProtocolGetmore_Printf (&rpc->getmore);
      break;
   case WIRE_PROTOCOL_DELETE:
      WireProtocolDelete_Printf (&rpc->delete);
      break;
   case WIRE_PROTOCOL_KILL_CURSORS:
      WireProtocolKillCursors_Printf (&rpc->kill_cursors);
      break;
   default:
      LOG_WARNING ("Unknown rpc type: 0x%08x", rpc->header.opcode);
      break;
   }
}


bool
WireProtocolMessage_Scatter (WireProtocolMessage *rpc, /* IN */
                             const uint8_t *buf,       /* IN */
                             size_t buflen)            /* IN */
{
   WireProtocolOpcode opcode;

   ASSERT (rpc);
   ASSERT (buf);
   ASSERT (buflen);

   memset (rpc, 0, sizeof *rpc);

   if (UNLIKELY (buflen < 16)) {
      return false;
   }

   if (!WireProtocolHeader_Scatter (&rpc->header, buf, 16)) {
      return false;
   }

   opcode = UINT32_FROM_LE (rpc->header.opcode);

   switch (opcode) {
   case WIRE_PROTOCOL_REPLY:
      return WireProtocolReply_Scatter (&rpc->reply, buf, buflen);
   case WIRE_PROTOCOL_MSG:
      return WireProtocolMsg_Scatter (&rpc->msg, buf, buflen);
   case WIRE_PROTOCOL_UPDATE:
      return WireProtocolUpdate_Scatter (&rpc->update, buf, buflen);
   case WIRE_PROTOCOL_INSERT:
      return WireProtocolInsert_Scatter (&rpc->insert, buf, buflen);
   case WIRE_PROTOCOL_QUERY:
      return WireProtocolQuery_Scatter (&rpc->query, buf, buflen);
   case WIRE_PROTOCOL_GETMORE:
      return WireProtocolGetmore_Scatter (&rpc->getmore, buf, buflen);
   case WIRE_PROTOCOL_DELETE:
      return WireProtocolDelete_Scatter (&rpc->delete, buf, buflen);
   case WIRE_PROTOCOL_KILL_CURSORS:
      return WireProtocolKillCursors_Scatter (&rpc->kill_cursors, buf, buflen);
   default:
      LOG_WARNING ("Unknown rpc type: 0x%08x", opcode);
      return false;
   }
}

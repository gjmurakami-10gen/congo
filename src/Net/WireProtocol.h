/* WireProtocol.h
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


#ifndef WIRE_PROTOCOL_H
#define WIRE_PROTOCOL_H


#include <stddef.h>

#include <Array.h>
#include <Debug.h>
#include <Macros.h>
#include <Types.h>


BEGIN_DECLS


typedef enum
{
   WIRE_PROTOCOL_REPLY_NONE               = 0,
   WIRE_PROTOCOL_REPLY_CURSOR_NOT_FOUND   = 1 << 0,
   WIRE_PROTOCOL_REPLY_QUERY_FAILURE      = 1 << 1,
   WIRE_PROTOCOL_REPLY_SHARD_CONFIG_STALE = 1 << 2,
   WIRE_PROTOCOL_REPLY_AWAIT_CAPABLE      = 1 << 3,
} WireProtocolReplyFlags;


typedef enum
{
   WIRE_PROTOCOL_QUERY_NONE              = 0,
   WIRE_PROTOCOL_QUERY_TAILABLE_CURSOR   = 1 << 1,
   WIRE_PROTOCOL_QUERY_SLAVE_OK          = 1 << 2,
   WIRE_PROTOCOL_QUERY_OPLOG_REPLAY      = 1 << 3,
   WIRE_PROTOCOL_QUERY_NO_CURSOR_TIMEOUT = 1 << 4,
   WIRE_PROTOCOL_QUERY_AWAIT_DATA        = 1 << 5,
   WIRE_PROTOCOL_QUERY_EXHAUST           = 1 << 6,
   WIRE_PROTOCOL_QUERY_PARTIAL           = 1 << 7,
} WireProtocolQueryFlags;


typedef enum
{
   WIRE_PROTOCOL_UPDATE_NONE         = 0,
   WIRE_PROTOCOL_UPDATE_UPSERT       = 1 << 0,
   WIRE_PROTOCOL_UPDATE_MULTI_UPDATE = 1 << 1,
} WireProtocolUpdateFlags;


typedef enum
{
   WIRE_PROTOCOL_INSERT_NONE              = 0,
   WIRE_PROTOCOL_INSERT_CONTINUE_ON_ERROR = 1 << 0,
} WireProtocolInsertFlags;


typedef enum
{
   WIRE_PROTOCOL_DELETE_NONE          = 0,
   WIRE_PROTOCOL_DELETE_SINGLE_REMOVE = 1 << 0,
} WireProtocolDeleteFlags;


typedef enum
{
   WIRE_PROTOCOL_REPLY = 1,
   WIRE_PROTOCOL_MSG = 1000,
   WIRE_PROTOCOL_UPDATE = 2001,
   WIRE_PROTOCOL_INSERT = 2002,
   WIRE_PROTOCOL_QUERY = 2004,
   WIRE_PROTOCOL_GETMORE = 2005,
   WIRE_PROTOCOL_DELETE = 2006,
   WIRE_PROTOCOL_KILL_CURSORS = 2007,
} WireProtocolOpcode;


#define RPC(_name, _code)                typedef struct { _code } WireProtocol##_name;
#define INT32_FIELD(_name)               int32_t _name;
#define INT64_FIELD(_name)               int64_t _name;
#define INT64_ARRAY_FIELD(_len, _name)   int32_t _len; int64_t *_name;
#define CSTRING_FIELD(_name)             const char *_name;
#define BSON_FIELD(_name)                const uint8_t *_name;
#define BSON_ARRAY_FIELD(_name)          const uint8_t *_name; int32_t _name##_len;
#define IOVEC_ARRAY_FIELD(_name)         const struct iovec *_name; int32_t n_##_name; struct iovec _name##_recv;
#define RAW_BUFFER_FIELD(_name)          const uint8_t *_name; int32_t _name##_len;
#define BSON_OPTIONAL(_check, _code)     _code


#include "WireProtocolDelete.def"
#include "WireProtocolGetmore.def"
#include "WireProtocolHeader.def"
#include "WireProtocolInsert.def"
#include "WireProtocolKillCursors.def"
#include "WireProtocolMsg.def"
#include "WireProtocolQuery.def"
#include "WireProtocolReply.def"
#include "WireProtocolUpdate.def"


#pragma pack(push, 1)
typedef union
{
   WireProtocolDelete      delete;
   WireProtocolGetmore     getmore;
   WireProtocolHeader      header;
   WireProtocolInsert      insert;
   WireProtocolKillCursors kill_cursors;
   WireProtocolMsg         msg;
   WireProtocolQuery       query;
   WireProtocolReply       reply;
   WireProtocolUpdate      update;
} WireProtocolMessage;
#pragma pack(pop)


STATIC_ASSERT (sizeof (WireProtocolHeader) == 16);
STATIC_ASSERT (offsetof (WireProtocolHeader, opcode) ==
               offsetof (WireProtocolReply, opcode));


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


void WireProtocolMessage_FromLe  (WireProtocolMessage *message);
void WireProtocolMessage_Gather  (WireProtocolMessage *message,
                                  Array *iovecs);
void WireProtocolMessage_Printf  (WireProtocolMessage *message);
bool WireProtocolMessage_Scatter (WireProtocolMessage *message,
                                  const uint8_t *buf,
                                  size_t buflen);
void WireProtocolMessage_ToLe    (WireProtocolMessage *message);


END_DECLS


#endif /* WIRE_PROTOCOL_MESSAGE */

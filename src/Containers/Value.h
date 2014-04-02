/* Value.h
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


#ifndef VALUE_H
#define VALUE_H


#include <Macros.h>
#include <Types.h>
#include <CString.h>


BEGIN_DECLS


typedef struct
{
   int type;
   int __pad;
   union {
      int16_t     i16;
      uint16_t    u16;
      int32_t     i32;
      uint32_t    u32;
      int64_t     i64;
      uint64_t    u64;
      size_t      sz;
      size_t      ssz;
      float       flt4;
      double      flt8;
      bool        bl;
      char       *str;
   } u;
} Value;


#define VALUE_TYPE_INT16  1
#define VALUE_TYPE_UINT16 2
#define VALUE_TYPE_INT32  3
#define VALUE_TYPE_UINT32 4
#define VALUE_TYPE_INT64  5
#define VALUE_TYPE_UINT64 6
#define VALUE_TYPE_FLOAT  7
#define VALUE_TYPE_DOUBLE 8
#define VALUE_TYPE_SIZE   9
#define VALUE_TYPE_SSIZE  10
#define VALUE_TYPE_STRING 11
#define VALUE_TYPE_BOOL   12


#define _Value_Init(_container, _type, _field, _item) \
   do { \
      (_container)->type = _type; \
      (_container)->u._field = _item; \
   } while (0)
#define Value_InitInt16(container, item) \
   _Value_Init(container, VALUE_TYPE_INT16, i16, item)
#define Value_InitUInt16(container, item) \
   _Value_Init(container, VALUE_TYPE_UINT16, ui16, item)
#define Value_InitInt32(container, item) \
   _Value_Init(container, VALUE_TYPE_INT32, i32, item)
#define Value_InitUInt32(container, item) \
   _Value_Init(container, VALUE_TYPE_UINT32, ui32, item)
#define Value_InitInt64(container, item) \
   _Value_Init(container, VALUE_TYPE_INT64, i64, item)
#define Value_InitUInt64(container, item) \
   _Value_Init(container, VALUE_TYPE_UINT64, ui64, item)
#define Value_InitDouble(container, item) \
   _Value_Init(container, VALUE_TYPE_DOUBLE, flt8, item)
#define Value_InitFloat(container, item) \
   _Value_Init(container, VALUE_TYPE_FLOAT, flt4, item)
#define Value_InitSize(container, item) \
   _Value_Init(container, VALUE_TYPE_SIZE, sz, item)
#define Value_InitSSize(container, item) \
   _Value_Init(container, VALUE_TYPE_SSIZE, ssz, item)
#define Value_InitString(container, item) \
   _Value_Init(container, VALUE_TYPE_STRING, str, CString_Dup(item))
#define Value_InitBool(container, item) \
   _Value_Init(container, VALUE_TYPE_BOOL, bl, item)


#define Value_GetInt16(value)  (value)->u.i16
#define Value_GetUInt16(value) (value)->u.u16
#define Value_GetInt32(value)  (value)->u.i32
#define Value_GetUInt32(value) (value)->u.u32
#define Value_GetInt64(value)  (value)->u.i64
#define Value_GetUInt64(value) (value)->u.u64
#define Value_GetDouble(value) (value)->u.flt8
#define Value_GetFloat(value)  (value)->u.flt4
#define Value_GetSize(value)   (value)->u.sz
#define Value_GetSSize(value)  (value)->u.ssz
#define Value_GetString(value) (value)->u.str
#define Value_GetBool(value)   (value)->u.bl


void Value_Copy    (const Value *src,
                    Value *dest);
void Value_Destroy (Value *value);


END_DECLS


#endif /* VALUE_H */

/* Mutable.h
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


#ifndef MUTABLE_H
#define MUTABLE_H


#include <stddef.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <Core/Macros.h>
#include <Core/Endian.h>
#include <Core/Types.h>


BEGIN_DECLS


#define PAGE_SIZE 4096


typedef struct _Mutable Mutable;


struct _Mutable
{
   size_t        iovcnt;
   struct iovec *iov;
};


void Mutable_Init    (Mutable *mutable,
                      struct iovec *iov,
                      size_t iovcnt);
void Mutable_IO      (Mutable *mutable,
                      bool mutate,
                      void *buf,
                      size_t count,
                      off_t offset);
void Mutable_Destroy (Mutable *mutable);


#define Mutable_Read(m,b,c,o)  Mutable_IO(m,false,b,c,o)
#define Mutable_Write(m,b,c,o) Mutable_IO(m,true,b,c,o)


#if defined(__GNUC__)
# define Mutable_GetInt8(m,C,F)    ({int8_t v;Mutable_Read(m,&v,1,offsetof(C,F));v;})
# define Mutable_GetInt16(m,C,F)   ({int16_t v;Mutable_Read(m,&v,2,offsetof(C,F));UINT16_FROM_LE(v);})
# define Mutable_GetInt32(m,C,F)   ({int32_t v;Mutable_Read(m,&v,4,offsetof(C,F));UINT32_FROM_LE(v);})
# define Mutable_GetInt64(m,C,F)   ({int64_t v;Mutable_Read(m,&v,8,offsetof(C,F));UINT64_FROM_LE(v);})
# define Mutable_SetInt8(m,C,F,v)  ({Mutable_Write(m,&v,1,offsetof(C,F));})
# define Mutable_SetInt16(m,C,F,v) ({int16_t v_le=UINT16_TO_LE(v);Mutable_Write(m,&v_le,2,offsetof(C,F));})
# define Mutable_SetInt32(m,C,F,v) ({int32_t v_le=UINT32_TO_LE(v);Mutable_Write(m,&v_le,4,offsetof(C,F));})
# define Mutable_SetInt64(m,C,F,v) ({int64_t v_le=UINT64_TO_LE(v);Mutable_Write(m,&v_le,8,offsetof(C,F));})
#else
# warning "Implement mutables for non-GCC like systems."
#endif


END_DECLS


#endif /* MUTABLE_H */

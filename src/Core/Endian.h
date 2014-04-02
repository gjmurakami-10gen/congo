/* Endian.h
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


#ifndef ENDIAN_H
#define ENDIAN_H


#include <Types.h>


#if defined(__linux__)
# include <endian.h>
#elif defined(__FreeBSD__) || \
      defined(__NetBSD__) || \
      defined(__OpenBSD__) || \
      defined(__DragonFly__)
# include <sys/endian.h>
#elif defined(__APPLE__)
# include <libkern/OSByteOrder.h>
#endif


#if defined(__linux__) || \
    defined(__FreeBSD__) || \
    defined(__NetBSD__) || \
    defined(__OpenBSD__) || \
    defined(__DragonFly__) || \
    defined(__sun)
# define UINT16_FROM_LE(v) ((uint16_t) le16toh((v)))
# define UINT16_TO_LE(v)   ((uint16_t) htole16((v)))
# define UINT16_FROM_BE(v) ((uint16_t) be16toh((v)))
# define UINT16_TO_BE(v)   ((uint16_t) htobe16((v)))
# define UINT32_FROM_LE(v) ((uint32_t) le32toh((v)))
# define UINT32_TO_LE(v)   ((uint32_t) htole32((v)))
# define UINT32_FROM_BE(v) ((uint32_t) be32toh((v)))
# define UINT32_TO_BE(v)   ((uint32_t) htobe32((v)))
# define UINT64_FROM_LE(v) ((uint64_t) le64toh((v)))
# define UINT64_TO_LE(v)   ((uint64_t) htole64((v)))
# define UINT64_FROM_BE(v) ((uint64_t) be64toh((v)))
# define UINT64_TO_BE(v)   ((uint64_t) htobe64((v)))
#elif defined(__APPLE__)
# define UINT16_FROM_LE(v) ((uint16_t) OSSwapLittleToHostInt16((v)))
# define UINT16_TO_LE(v)   ((uint16_t) OSSwapHostToLittleInt16((v)))
# define UINT16_FROM_BE(v) ((uint16_t) OSSwapBigToHostInt16((v)))
# define UINT16_TO_BE(v)   ((uint16_t) OSSwapHostToBigInt16((v)))
# define UINT32_FROM_LE(v) ((uint32_t) OSSwapLittleToHostInt32((v)))
# define UINT32_TO_LE(v)   ((uint32_t) OSSwapHostToLittleInt32((v)))
# define UINT32_FROM_BE(v) ((uint32_t) OSSwapBigToHostInt32((v)))
# define UINT32_TO_BE(v)   ((uint32_t) OSSwapHostToBigInt32((v)))
# define UINT64_FROM_LE(v) ((uint64_t) OSSwapLittleToHostInt64((v)))
# define UINT64_TO_LE(v)   ((uint64_t) OSSwapHostToLittleInt64((v)))
# define UINT64_FROM_BE(v) ((uint64_t) OSSwapBigToHostInt64((v)))
# define UINT64_TO_BE(v)   ((uint64_t) OSSwapHostToBigInt64((v)))
#elif defined(_WIN32) || defined(_WIN64)
# define UINT16_FROM_LE(v) ((uint16_t) (v))
# define UINT16_TO_LE(v)   ((uint16_t) (v))
# define UINT16_FROM_BE(v) ((uint16_t) _byteswap_ushort((v)))
# define UINT16_TO_BE(v)   ((uint16_t) _byteswap_ushort((v)))
# define UINT32_FROM_LE(v) ((uint32_t) (v))
# define UINT32_TO_LE(v)   ((uint32_t) (v))
# define UINT32_FROM_BE(v) ((uint32_t) _byteswap_ulong((v)))
# define UINT32_TO_BE(v)   ((uint32_t) _byteswap_ulong((v)))
# define UINT64_FROM_LE(v) ((uint64_t) (v))
# define UINT64_TO_LE(v)   ((uint64_t) (v))
# define UINT64_FROM_BE(v) ((uint64_t) _byteswap_uint64((v)))
# define UINT64_TO_BE(v)   ((uint64_t) _byteswap_uint64((v)))
#else
# error "You're platform is not yet supported."
#endif


#endif /* ENDIAN_H */

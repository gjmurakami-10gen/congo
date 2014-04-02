/* HashTable.h
 *
 * Copyright (C) 2014 Christian Hergert <christian@hergert.me>
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


#ifndef HASHTABLE_H
#define HASHTABLE_H


#include <Macros.h>
#include <Types.h>


BEGIN_DECLS


typedef struct _HashTable HashTable;


typedef uint32_t (*HashFunc)  (const void *data);
typedef bool     (*EqualFunc) (const void *data1,
                               const void *data2);
typedef void     (*FreeFunc)  (void *data);


HashTable *HashTable_Create    (uint32_t size,
                                HashFunc hash_func,
                                EqualFunc equal_func,
                                FreeFunc key_free_func,
                                FreeFunc value_free_func);
uint32_t   HashTable_CountKeys (HashTable *hashtable);
void       HashTable_Free      (HashTable *hash_table);
void       HashTable_Insert    (HashTable *hash_table,
                                void *key,
                                void *data);
void       HashTable_Remove    (HashTable *hash_table,
                                const void *key);
bool       HashTable_Contains  (HashTable *hash_table,
                                const void *key);
void      *HashTable_Lookup    (HashTable *hash_table,
                                const void *key);


bool       Pointer_Equal       (const void *data1,
                                const void *data2);
uint32_t   Pointer_Hash        (const void *data);


bool       UInt32_Equal        (const void *data1,
                                const void *data2);
uint32_t   UInt32_Hash         (const void *data);


END_DECLS


#endif /* HASHTABLE_H */

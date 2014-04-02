/* HashTable.c
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


#include <Debug.h>
#include <HashTable.h>
#include <Memory.h>


typedef struct _HashTableItem HashTableItem;


struct _HashTableItem
{
   HashTableItem *next;
   void *key;
   void *value;
};


struct _HashTable
{
   uint32_t len;
   uint32_t key_count;
   HashFunc hash_func;
   EqualFunc equal_func;
   FreeFunc key_free_func;
   FreeFunc value_free_func;
   HashTableItem *table[0];
};


/*
 *--------------------------------------------------------------------------
 *
 * HashTable_Create --
 *
 *       Creates a new instance of HashTable.
 *
 * Returns:
 *       HashTable that should be freed with HashTable_Free().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

HashTable *
HashTable_Create (uint32_t size,            /* IN */
                  HashFunc hash_func,       /* IN */
                  EqualFunc equal_func,     /* IN */
                  FreeFunc key_free_func,   /* IN */
                  FreeFunc value_free_func) /* IN */
{
   HashTable *hash_table;
   uint32_t n_bytes;

   ASSERT(size > 0);
   ASSERT(hash_func);
   ASSERT(equal_func);

   n_bytes = sizeof *hash_table + (size * sizeof (HashTableItem *));
   hash_table = Memory_SafeMalloc0 (n_bytes);
   hash_table->len = size;
   hash_table->hash_func = hash_func;
   hash_table->equal_func = equal_func;
   hash_table->key_free_func = key_free_func;
   hash_table->value_free_func = value_free_func;

   return hash_table;
}


/*
 *--------------------------------------------------------------------------
 *
 * HashTable_Lookup --
 *
 *       Attempts to find the first item in the hash table matching key.
 *
 * Returns:
 *       The value stored in the hash table if successful; otherwise NULL.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void *
HashTable_Lookup (HashTable *hash_table, /* IN */
                  const void *key)       /* IN */
{
   HashTableItem *item;
   uint32_t pos;

   ASSERT(hash_table);

   pos = hash_table->hash_func(key) % hash_table->len;

   for (item = hash_table->table[pos]; item; item = item->next) {
      if (hash_table->equal_func(key, item->key)) {
         return item->value;
      }
   }

   return NULL;
}


/*
 *--------------------------------------------------------------------------
 *
 * HashTable_Remove --
 *
 *       Removes the first item matching key from the hash table.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
HashTable_Remove(HashTable *hash_table, /* IN */
                 const void *key)       /* IN */
{
   HashTableItem *item;
   HashTableItem *last = NULL;
   uint32_t pos;

   ASSERT(hash_table);

   pos = hash_table->hash_func(key) % hash_table->len;

   for (item = hash_table->table[pos]; item; item = item->next) {
      if (hash_table->equal_func(key, item->key)) {
         if (last) {
            last->next = item->next;
         } else {
            hash_table->table[pos] = item->next;
         }
         hash_table->key_count--;
         if (hash_table->value_free_func) {
            hash_table->value_free_func(item->value);
         }
         if (hash_table->key_free_func) {
            hash_table->key_free_func(item->key);
         }
         Memory_Free (item);
         break;
      }
      last = item;
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * HashTable_CountKeys --
 *
 *       Returns the number of keys stored in the hash table. This does
 *       include duplicated entries.
 *
 * Returns:
 *       The number of items in the HashTable.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

uint32_t
HashTable_CountKeys(HashTable *hash_table) /* IN */
{
   ASSERT(hash_table);
   return hash_table->key_count;
}


/*
 *--------------------------------------------------------------------------
 *
 * HashTable_Contains --
 *
 *       Checks to see if hash_table contains an item matching key.
 *
 * Returns:
 *       true if an item matches key; otherwise false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

bool
HashTable_Contains(HashTable *hash_table, /* IN */
                        const void *key)            /* IN */
{
   HashTableItem *item;
   uint32_t pos;

   ASSERT(hash_table);

   pos = hash_table->hash_func(key) % hash_table->len;

   for (item = hash_table->table[pos]; item; item = item->next) {
      if (hash_table->equal_func(key, item->key)) {
         return true;
      }
   }

   return false;
}


/*
 *--------------------------------------------------------------------------
 *
 * HashTable_Insert --
 *
 *       Inserts a new item into the hash table.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
HashTable_Insert(HashTable *hash_table, /* IN */
                      void *key,                  /* IN */
                      void *data)                 /* IN */
{
   HashTableItem *item;
   uint32_t pos;

   ASSERT(hash_table);

   pos = hash_table->hash_func(key) % hash_table->len;

   item = Memory_SafeMalloc0 (sizeof *item);
   item->key = key;
   item->value = data;
   item->next = hash_table->table[pos];

   hash_table->table[pos] = item;
   hash_table->key_count++;
}


/*
 *--------------------------------------------------------------------------
 *
 * HashTable_Free --
 *
 *       Destroys the hash table and all the items contained within it.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       Everything.
 *
 *--------------------------------------------------------------------------
 */

void
HashTable_Free(HashTable *hash_table) /* IN */
{
   HashTableItem *item;
   HashTableItem *removed;
   int32_t i;

   ASSERT(hash_table);

   for (i = 0; i < hash_table->len; i++) {
      item = hash_table->table[i];
      while (item) {
         removed = item;
         item = item->next;
         if (hash_table->key_free_func) {
            hash_table->key_free_func(removed->key);
         }
         if (hash_table->value_free_func) {
            hash_table->value_free_func(removed->value);
         }
         Memory_Free(removed);
      }
   }

   Memory_Free(hash_table);
}


uint32_t
UInt32_Hash (const void *data) /* IN */
{
   ASSERT(data);

   return *(uint32_t *)data;
}


bool
UInt32_Equal (const void *data1, /* IN */
              const void *data2) /* IN */
{
   ASSERT(data1);
   ASSERT(data2);

   return (*(uint32_t *)data1) == (*(uint32_t *)data2);
}


/*
 *--------------------------------------------------------------------------
 *
 * Pointer_Hash --
 *
 *       Hash function for a pointer to a pointer.
 *
 * Returns:
 *       A uint32_t containing the hash.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

uint32_t
Pointer_Hash (const void *data) /* IN */
{
   return (uint32_t)(size_t)data;
}


/*
 *--------------------------------------------------------------------------
 *
 * Pointer_Equal --
 *
 *       Function to compare equality on two pointers.
 *
 * Returns:
 *       true if the pointers are the same; otherwise false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

bool
Pointer_Equal (const void *data1, /* IN */
               const void *data2) /* IN */
{
   return (data1 == data2);
}

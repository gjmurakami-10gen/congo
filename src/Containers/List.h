/* List.h
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


#ifndef LIST_H
#define LIST_H


#include <Macros.h>
#include <Memory.h>


BEGIN_DECLS


typedef struct _List  List;


struct _List
{
   void *data;
   List *prev;
   List *next;
};


static __inline__ List *
List_Prepend (List *list, /* IN */
              void *data) /* IN */
{
   List *item = Memory_SafeMalloc (sizeof *list);
   item->data = data;
   item->next = list;
   item->prev = NULL;
   if (list) list->prev = item;
   return item;
}


static __inline__ List *
List_Append (List *list, /* IN */
             void *data) /* IN */
{
   List *tmp;
   List *item = Memory_SafeMalloc (sizeof *list);
   item->data = data;
   item->prev = NULL;
   item->next = NULL;
   if (!list) return item;
   for (tmp = list; tmp && tmp->next; tmp = tmp->next) { }
   tmp->next = item;
   item->prev = tmp;
   return list;
}


static __inline__ List *
List_Last (List *list)
{
   for (; list && list->next; list = list->next) { }
   return list;
}


static __inline__ List *
List_Remove (List *list, /* IN */
             List *item) /* IN */
{
   if (item->prev) {
      item->prev->next = item->next;
   }

   if (item->next) {
      item->next->prev = item->prev;
   }

   if (list == item) {
      list = item->next;
   }

   Memory_Free (item);

   return list;
}


END_DECLS


#endif /* LIST_H */

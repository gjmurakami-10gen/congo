/* Tunable.c
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


#include <Array.h>
#include <CString.h>
#include <Debug.h>
#include <Log.h>
#include <Mutex.h>
#include <ThreadOnce.h>
#include <Tunable.h>


typedef struct
{
   char  *name;
   Value  value;
} TunableInfo;


static Mutex gTunableMutex;
static Array gTunableInfo;


static void
Tunable_Init (void)
{
   Mutex_Init (&gTunableMutex, NULL);
   Array_Init (&gTunableInfo, sizeof (TunableInfo), false);
}


Tunable
Tunable_Register (const char *key,            /* IN */
                  const Value *current_value) /* IN */
{
   static ThreadOnce once = THREAD_ONCE_INIT;
   TunableInfo info = { 0 };
   Tunable tunable;

   ASSERT (key);

   ThreadOnce_Once (&once, Tunable_Init);

   info.name = CString_Dup (key);

   if (current_value) {
      Value_Copy (current_value, &info.value);
   }

   Mutex_Lock (&gTunableMutex);
   tunable = gTunableInfo.len;
   Array_Append (&gTunableInfo, info);
   Mutex_Unlock (&gTunableMutex);

   return tunable;
}


void
Tunable_Get (Tunable tunable, /* IN */
             Value *value)    /* OUT */
{
   TunableInfo *info;

   ASSERT (tunable != TUNABLE_INVALID);
   ASSERT (value);

   if (tunable < gTunableInfo.len) {
      info = &Array_Index (&gTunableInfo, TunableInfo, tunable);
      Value_Copy (&info->value, value);
      return;
   }

   LOG_WARNING ("No such tunable %d", tunable);
   Memory_Zero (value, sizeof *value);
}


void
Tunable_Set (Tunable tunable,    /* IN */
             const Value *value) /* IN */
{
   TunableInfo *info;

   ASSERT (tunable != TUNABLE_INVALID);

   if (tunable < gTunableInfo.len) {
      info = &Array_Index (&gTunableInfo, TunableInfo, tunable);
      Value_Destroy (&info->value);
      Value_Copy (value, &info->value);
      return;
   }

   LOG_WARNING ("No such tunable %d", tunable);
}


Tunable
Tunable_Find (const char *key) /* IN */
{
   const TunableInfo *info;
   int i;

   ASSERT (key);

   for (i = 0; i < gTunableInfo.len; i++) {
      info = &Array_Index (&gTunableInfo, TunableInfo, i);
      if (0 == strcasecmp (key, info->name)) {
         return (Tunable)i;
      }
   }

   return TUNABLE_INVALID;
}

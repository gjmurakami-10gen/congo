/* Random.c
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

#include <stdlib.h>

#include <Debug.h>
#include <File.h>
#include <Log.h>
#include <Random.h>
#include <TimeSpec.h>


static File gRandom = FILE_INVALID;


void
Random_Init (void)
{
   File fd;

   fd = File_Open ("/dev/urandom", O_RDONLY, 0);
   if (fd == FILE_INVALID) {
      LOG_WARNING ("Failed to open /dev/urandom.");
      fd = File_Open ("/dev/random", O_RDONLY, 0);
      if (fd == FILE_INVALID) {
         LOG_WARNING ("Failed to open /dev/random.");
         ASSERT (false);
         return;
      }
   }

   gRandom = fd;
}


int32_t
Random_Int32 (void)
{
   int32_t val;
   ssize_t ret = 0;

   while (ret != 4) {
      ret = File_Read (gRandom, &val, 4);
      if (ret == -1) {
         ASSERT (false);
         return 0;
      }
   }

   return val;
}

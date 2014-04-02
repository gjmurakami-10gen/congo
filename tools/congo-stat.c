/* congo-stat.c
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


#include <errno.h>
#include <limits.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Counters/Counter.h>


static void
usage (const char *prgname)
{
   fprintf (stderr, "usage: %s PID\n", prgname);
}


static void
Counters_ForeachCb (Counter *counter,
                    void    *user_data)
{
   int64_t value;

   value = Counter_Get (counter);
   fprintf (stdout, "%-20s : %-20s : %-30s : %"PRId64"\n",
            counter->category, counter->name, counter->description, value);
}


int
main (int   argc,
      char *argv[])
{
   char *endptr = NULL;
   long lpid;

   if (argc != 2) {
      usage (argv [0]);
      return EXIT_FAILURE;
   } else if (0 == strcmp ("-h", argv [1])) {
      usage (argv [0]);
      return EXIT_SUCCESS;
   }

   lpid = strtol (argv [1], &endptr, 10);

   if (((lpid == 0) && (endptr == argv [1])) ||
       (((lpid == LONG_MIN) || (lpid == LONG_MAX)) && (errno == ERANGE))) {
      usage (argv [0]);
      return EXIT_FAILURE;
   }

   Counters_InitRemote ((pid_t)(int)lpid);

   Counters_Foreach (Counters_ForeachCb, NULL);

   return EXIT_SUCCESS;
}

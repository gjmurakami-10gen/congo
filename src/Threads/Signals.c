/* Signals.c
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

#include <pthread.h>
#include <signal.h>

#include <Log.h>
#include <Signals.h>


static void
Signals_HandleSIGINT (int signum) /* IN */
{
   LOG_WARNING ("SIGINT handled.");
}


static void
Signals_HandleSIGTERM (int signum) /* IN */
{
   LOG_WARNING ("SIGTERM handled.");
}


void
Signals_Init (void)
{
   sigset_t set;
   int r;

   (void)signal (SIGINT,  Signals_HandleSIGINT);
   (void)signal (SIGTERM, Signals_HandleSIGTERM);

   sigemptyset (&set);

   r = pthread_sigmask (SIG_BLOCK, &set, NULL);
   if (r != 0) {
      LOG_WARNING ("Failed to block signals.");
      return;
   }
}

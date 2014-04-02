/* Log.c
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


#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include <Log.h>


static void Log_DefaultLogFunc (LogLevel    level,
                                const char *domain,
                                const char *message,
                                void       *user_data);


static LogFunc  gLogFunc = Log_DefaultLogFunc;
static void    *gLogFuncData;
static int      gLogCorked;


/*
 *--------------------------------------------------------------------------
 *
 * Log_Log --
 *
 *       Format and log a message.
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
Log_Log (LogLevel level,     /* IN */
         const char *domain, /* IN */
         const char *format, /* IN */
         ...)                /* IN */
{
   va_list args;
   char msgbuf [1024];
   int ret;

   if (gLogCorked) {
      return;
   }

   va_start (args, format);

   ret = vsnprintf (msgbuf, sizeof msgbuf, format, args);

   if (ret == -1) {
      fprintf (stderr, "Failed to vsnprintf() \"%s\"", format);
      va_end (args);
      return;
   } else if (ret == sizeof msgbuf) {
      msgbuf [sizeof msgbuf - 2] = '\n';
      msgbuf [sizeof msgbuf - 1] = '\0';
   }

   gLogFunc (level, domain, msgbuf, gLogFuncData);

   va_end (args);
}


/*
 *--------------------------------------------------------------------------
 *
 * LogLevel_ToString --
 *
 *       Return an internal log level string for a given level.
 *
 * Returns:
 *       A const string that should not be modified or freed.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static const char *
LogLevel_ToString (LogLevel level) /* IN */
{
   switch (level) {
   case LOG_LEVEL_ERROR:
      return "ERROR";
   case LOG_LEVEL_CRITICAL:
      return "CRITICAL";
   case LOG_LEVEL_WARNING:
      return "WARNING";
   case LOG_LEVEL_MESSAGE:
      return "MESSAGE";
   case LOG_LEVEL_INFO:
      return "INFO";
   case LOG_LEVEL_DEBUG:
      return "DEBUG";
   case LOG_LEVEL_TRACE:
      return "TRACE";
   default:
      return "UNKNOWN";
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * Log_DefaultLogFunc --
 *
 *       Default Log function that writes to stderr.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static void
Log_DefaultLogFunc (LogLevel level,      /* IN */
                    const char *domain,  /* IN */
                    const char *message, /* IN */
                    void *user_data)     /* IN */
{
   struct timeval tv;
   struct tm tt;
   time_t t;
   char nowstr [32];

   gettimeofday (&tv, NULL);
   t = tv.tv_sec;
   tt = *localtime (&t);

   strftime (nowstr, sizeof nowstr, "%Y/%m/%d %H:%M:%S", &tt);

   fprintf (stderr, "%s.%04ld: %8s: %12s: %s\n",
            nowstr,
            tv.tv_usec / 1000L,
            LogLevel_ToString (level),
            domain,
            message);
}


/*
 *--------------------------------------------------------------------------
 *
 * Log_Cork --
 *
 *       Cork the log, causing incoming log messages to be dropped.
 *
 *       This is useful for testing environments that want to suppress
 *       output unless verbose is enabled.
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
Log_Cork (void)
{
   gLogCorked = 1;
}


/*
 *--------------------------------------------------------------------------
 *
 * Log_Uncork --
 *
 *       Uncorks a log after a call to Log_Cork().
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
Log_Uncork (void)
{
   gLogCorked = 0;
}

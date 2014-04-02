/* Log.h
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


#ifndef LOG_H
#define LOG_H


#include <Macros.h>


BEGIN_DECLS


#ifndef LOG_DOMAIN
# define LOG_DOMAIN "General"
#endif


#define LOG_ERROR(...)    Log_Log(LOG_LEVEL_ERROR,    LOG_DOMAIN, __VA_ARGS__)
#define LOG_CRITICAL(...) Log_Log(LOG_LEVEL_CRITICAL, LOG_DOMAIN, __VA_ARGS__)
#define LOG_WARNING(...)  Log_Log(LOG_LEVEL_WARNING,  LOG_DOMAIN, __VA_ARGS__)
#define LOG_MESSAGE(...)  Log_Log(LOG_LEVEL_MESSAGE,  LOG_DOMAIN, __VA_ARGS__)
#define LOG_INFO(...)     Log_Log(LOG_LEVEL_INFO,     LOG_DOMAIN, __VA_ARGS__)


#if !defined(DISABLE_DEBUG)
# define LOG_DEBUG(...)   Log_Log(LOG_LEVEL_DEBUG,    LOG_DOMAIN, __VA_ARGS__)
#else
# define LOG_DEBUG(...)
#endif


typedef enum
{
   LOG_LEVEL_TRACE,
   LOG_LEVEL_DEBUG,
   LOG_LEVEL_INFO,
   LOG_LEVEL_MESSAGE,
   LOG_LEVEL_WARNING,
   LOG_LEVEL_CRITICAL,
   LOG_LEVEL_ERROR,
} LogLevel;


typedef void (*LogFunc) (LogLevel    level,
                         const char *domain,
                         const char *message,
                         void       *user_data);


void Log_Log    (LogLevel level,
                 const char *domain,
                 const char *format,
                 ...) GNUC_PRINTF (3, 4);
void Log_Trace  (const char *domain,
                 const char *format,
                 ...) GNUC_PRINTF (2, 3);
void Log_Cork   (void);
void Log_Uncork (void);


END_DECLS


#endif /* LOG_H */

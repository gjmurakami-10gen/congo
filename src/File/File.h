/* File.h
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


#ifndef FILE_H
#define FILE_H


#include <fcntl.h>
#include <sys/stat.h>

#include <Error.h>
#include <Macros.h>
#include <Platform.h>
#include <Types.h>


BEGIN_DECLS


#if defined(PLATFORM_POSIX)
typedef int File;
# define FILE_INVALID ((File)-1)
#else
typedef HANDLE File;
# define FILE_INVALID ((File)HANDLE_INVALID_VALUE)
#endif


#define FILE_ERROR      3000
#define FILE_ERROR_ZERO 1


File    File_Open         (const char *path,
                           int flags,
                           mode_t mode);
ssize_t File_Read         (File file,
                           void *buffer,
                           size_t count);
ssize_t File_Write        (File file,
                           void *buffer,
                           size_t count);
bool    File_Close        (File file);
bool    File_Stat         (File file,
                           struct stat *st);
bool    File_ZeroAllocate (File file,
                           size_t size,
                           bool sparse,
                           Error *error);
ssize_t File_GetSize      (File file);
int     File_Sync         (File file);


END_DECLS


#endif /* FILE_H */

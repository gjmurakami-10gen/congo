/* Path.h
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


#ifndef PATH_H
#define PATH_H


#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <File.h>
#include <Macros.h>
#include <Platform.h>


BEGIN_DECLS


#ifdef PLATFORM_WIN32
# define DIR_SEPARATOR   '\\'
# define DIR_SEPARATOR_S "\\"
#else
# define DIR_SEPARATOR   '/'
# define DIR_SEPARATOR_S "/"
#endif


#define Path_Copy(src,dst) (memcpy((dst), (src), sizeof(Path)))


#if !defined(PATH_MAX)
# define PATH_MAX 255
#endif


#define FS_TYPE_NFS    0x6969
#define FS_TYPE_ZFS    0x2fc12fc1
#define FS_TYPE_BTRFS  0x9123683E
#define FS_TYPE_EXT234 0xEF53
#define FS_TYPE_XFS    0x58465342
#define FS_TYPE_JFS    0x3153464a
#define FS_TYPE_HFS    0x4244
#define FS_TYPE_UFS    0x00011954
#define FS_TYPE_NTFS   0x5346544E
#define FS_TYPE_HAMMER 0x3454f8e9

typedef struct
{
   char str [PATH_MAX + 1];
} Path;


int           Path_Build            (Path *path,
                                     const char *patr1,
                                     ...) GNUC_NULL_TERMINATED;
int           Path_Append           (Path *path,
                                     const char *part);
int           Path_AppendPrintf     (Path *path,
                                     const char *format,
                                     ...) GNUC_PRINTF (2, 3);
int           Path_Basename         (const Path *path,
                                     Path *basepath);
bool          Path_Parent           (const Path *path,
                                     Path *dirpath);
bool          Path_Exists           (const Path *path);
bool          Path_IsRelative       (const Path *path);
bool          Path_IsDir            (const Path *path);
bool          Path_IsWritable       (const Path *path);
bool          Path_IsReadable       (const Path *path);
unsigned long Path_FSType           (const Path *path);
bool          Path_IsCopyOnWrite    (const Path *path);
bool          Path_Stat             (const Path *path,
                                     struct stat *st);
bool          Path_MkdirWithParents (const Path *path,
                                     mode_t mode);
File          Path_Open             (const Path *path,
                                     int flags,
                                     mode_t mode);


END_DECLS

#endif /* PATH_H */

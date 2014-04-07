/* Path.c
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

#include <libgen.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_STATFS_H
# include <sys/statfs.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
# include <sys/mount.h>
#endif
#include <sys/types.h>
#include <unistd.h>

#include <CString.h>
#include <Debug.h>
#include <Log.h>
#include <Path.h>
#include <Task.h>


/*
 *--------------------------------------------------------------------------
 *
 * Path_BuildPath --
 *
 *       Build a path string in UTF-8 encoding and store the result in @str.
 *
 *       @size is the size of @str in bytes.
 *
 * Todo:
 *       Do this as a single pass.
 *       Check if we overflow @size.
 *
 * Returns:
 *       1 if successful, otherwise 0.
 *
 * Side effects:
 *       @str is initialized.
 *
 *--------------------------------------------------------------------------
 */

static int
Path_BuildPath (char *str,         /* OUT */
                size_t size,       /* IN */
                const char *part1, /* IN */
                va_list args)      /* IN */
{
   const char *part = part1;
   int need_sep = 0;

   ASSERT (str);

   /*
    * TODO: If we do not rely on CString_*() helpers, we can optimize the
    *       building of paths in one shot. As it is now, we do a lot of
    *       suffix checks which are pretty heavy. But I don't think we will
    *       be building these enough to matter.
    */

   str [0] = '\0';

   do {
      if (need_sep && !CString_HasSuffix (str, DIR_SEPARATOR_S)) {
         CString_Concat (str, size, DIR_SEPARATOR_S);
      }
      CString_Concat (str, size, part);
      need_sep = 1;
   } while ((part = va_arg (args, const char *)));

   return 1;
}


/*
 *--------------------------------------------------------------------------
 *
 * Path_Build --
 *
 *       Initialize @path using the parts provided. Parts should have a
 *       NULL sentinel indicating the end of the path chain.
 *
 * Returns:
 *       1 on success, 0 on failure.
 *
 * Side effects:
 *       @path is initialized.
 *
 *--------------------------------------------------------------------------
 */

int
Path_Build (Path *path,        /* OUT */
            const char *part1, /* IN */
            ...)               /* IN */
{
   va_list args;
   int ret;

   va_start (args, part1);
   ret = Path_BuildPath (path->str, sizeof path->str, part1, args);
   va_end (args);

   return ret;
}


/*
 *--------------------------------------------------------------------------
 *
 * Path_Append --
 *
 *       Append @part onto @path. A path separator may be added between
 *       the two parts if it does not already exist.
 *
 * Returns:
 *       1 if successful, otherwise 0.
 *
 * Side effects:
 *       @path is updated to include @part.
 *
 *--------------------------------------------------------------------------
 */

int
Path_Append (Path *path,       /* IN */
             const char *part) /* IN */
{
   ASSERT (path);
   ASSERT (part);

   if (!CString_HasSuffix (path->str, DIR_SEPARATOR_S)) {
      CString_Concat (path->str, sizeof path->str, DIR_SEPARATOR_S);
   }

   CString_Concat (path->str, sizeof path->str, part);

   return 1;
}


int
Path_AppendPrintf (Path *path,         /* IN */
                   const char *format, /* IN */
                   ...)                /* IN */
{
   va_list args;
   char part [PATH_MAX];

   ASSERT (path);
   ASSERT (format);

   va_start (args, format);
   if (vsnprintf (part, sizeof part, format, args) == sizeof part) {
      /* Too Big! */
      part [sizeof part - 1] = '\0';
      LOG_WARNING ("Cannot append path \"%s\", too big!", part);
      return 0;
   }
   va_end (args);

   return Path_Append (path, part);
}


/*
 *--------------------------------------------------------------------------
 *
 * Path_Basename --
 *
 *       Calculate the basename() of @path and store it as the path
 *       @basepath.
 *
 * Returns:
 *       1 on success, 0 on failure.
 *
 * Side effects:
 *       @basepath is initialized.
 *
 *--------------------------------------------------------------------------
 */

int
Path_Basename (const Path *path, /* IN */
               Path *basepath)   /* OUT */
{
   char *str;

   ASSERT (path);
   ASSERT (basepath);

   memcpy (basepath, path, sizeof *basepath);
   str = basename (basepath->str);
   memmove (basepath->str, str, strlen (str) + 1);

   return 1;
}


bool
Path_IsRelative (const Path *path) /* IN */
{
   ASSERT (path);

   return (path->str [0] != DIR_SEPARATOR);
}


/*
 *--------------------------------------------------------------------------
 *
 * Path_Parent --
 *
 *       Calculate the parent directory of @path and store the result in
 *       @dirpath.
 *
 * Returns:
 *       1 on success, 0 on failure.
 *
 * Side effects:
 *       @dirpath is initialized.
 *
 *--------------------------------------------------------------------------
 */

bool
Path_Parent (const Path *path, /* OUT */
             Path *dirpath)    /* OUT */
{
#if defined(PLATFORM_WIN32)
# warning "Path_Parent() is not yet implemented."
   return false;
#else
   char *str;

   ASSERT (path);
   ASSERT (dirpath);

   if (0 == strcmp (path->str, DIR_SEPARATOR_S)) {
      return false;
   }

   if (strchr (path->str, DIR_SEPARATOR)) {
      memcpy (dirpath, path, sizeof *dirpath);
      str = dirname (dirpath->str);
      memmove (dirpath->str, str, strlen (str) + 1);
      return true;
   }

   Path_Build (dirpath, "..", path->str, NULL);

   return true;
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * Path_Exists --
 *
 *       Check if @path exists on the file-system.
 *
 * Returns:
 *       true if @path exists, otherwise false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

bool
Path_Exists (const Path *path) /* IN */
{
   ASSERT (path);

   return (0 == access (path->str, F_OK));
}


/*
 *--------------------------------------------------------------------------
 *
 * Path_IsDir --
 *
 *       Checks to see if @path is a directory.
 *
 * Returns:
 *       true if @path is a directory, otherwise false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

bool
Path_IsDir (const Path *path) /* IN */
{
   struct stat st;

   ASSERT (path);

   if (0 == stat (path->str, &st)) {
      return S_ISDIR (st.st_mode);
   }

   return false;
}


/*
 *--------------------------------------------------------------------------
 *
 * Path_IsWritable --
 *
 *       Checks if @path is writable. If @path is a directory, it means
 *       that we can create files in the directory.
 *
 * Returns:
 *       true if @path is writable, otherwise false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

bool
Path_IsWritable (const Path *path) /* IN */
{
   ASSERT (path);

   return (0 == access (path->str, W_OK));
}


/*
 *--------------------------------------------------------------------------
 *
 * Path_IsReadable --
 *
 *       Is @path readable? If @path is a directory, then can we read
 *       files within the directory.
 *
 * Returns:
 *       true if @path is readable, otherwise @false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

bool
Path_IsReadable (const Path *path) /* IN */
{
   ASSERT (path);

   return (0 == access (path->str, R_OK));
}


/*
 *--------------------------------------------------------------------------
 *
 * Path_FSType --
 *
 *       Return the type of the underlying file-system. This corresponds
 *       to the magic header of the file-system.
 *
 *       See FS_TYPE_XFS, and others for the superblock magic.
 *
 * Returns:
 *       The fs type (see Path.h for super magic values).
 *       0 on failure or platform does not support this.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

unsigned long
Path_FSType (const Path *path) /* IN */
{
   unsigned long result = 0;
   struct statfs st;
   Path parent;

   ASSERT (path);

   Task_BeginBlockingCall ();

again:

   if (0 == statfs (path->str, &st)) {
#if defined(HAVE_STRUCT_STATFS_F_FSTYPENAME)
      if (0 == strcmp ("ufs", st.f_fstypename)) {
         result = FS_TYPE_UFS;
      } else if (0 == strcmp ("zfs", st.f_fstypename)) {
         result = FS_TYPE_ZFS;
      } else if (0 == strcmp ("hammer", st.f_fstypename)) {
         result = FS_TYPE_HAMMER;
      } else if ((0 == strcmp ("nfs", st.f_fstypename)) ||
                 (0 == strcmp ("oldnfs", st.f_fstypename))) {
         result = FS_TYPE_NFS;
      } else if (0 == strncmp ("ext", st.f_fstypename, 3)) {
         result = FS_TYPE_EXT234;
      } else if (0 == strcmp ("ntfs", st.f_fstypename)) {
         result = FS_TYPE_NTFS;
      }
      LOG_WARNING ("Failed to discover file-system type \"%s\". "
                   "Defaulting to UFS.", st.f_fstypename);
      result = FS_TYPE_UFS;
#elif defined(HAVE_STRUCT_STATFS_F_TYPE)
      result = st.f_type;
#else
# error "Unsupported statfs on target platform."
#endif
   } else {
      if (Path_Parent (path, &parent)) {
         path = &parent;
         goto again;
      }
   }

   Task_EndBlockingCall ();

   return result;
}


#if !defined(PLATFORM_WIN32)
int
Path_Open (const Path *path, /* IN */
           int flags,        /* IN */
           mode_t mode)      /* IN */
{
   ASSERT (path);

   return File_Open (path->str, flags, mode);
}
#endif


bool
Path_Stat (const Path *path, /* IN */
           struct stat *st)  /* OUT */
{
   int ret;

   ASSERT (path);
   ASSERT (st);

   Task_BeginBlockingCall ();
   ret = stat (path->str, st);
   Task_EndBlockingCall ();

   return (ret == 0);
}


bool
Path_MkdirWithParents (const Path *path, /* IN */
                       mode_t mode)      /* IN */
{
   Path ppath;
   char *parent;
   char copy [PATH_MAX];

   ASSERT (path);

   strncpy (copy, path->str, sizeof copy);
   copy [sizeof copy - 1] = '\0';
   parent = dirname (copy);

   if (0 != strcmp (path->str, parent)) {
      if (0 != access (parent, F_OK)) {
         Path_Build (&ppath, parent, NULL);
         if (!Path_MkdirWithParents (&ppath, mode)) {
            return false;
         }
      }
   }

   return (0 == mkdir (path->str, mode));
}


bool
Path_IsCopyOnWrite (const Path *path) /* IN */
{
   unsigned long fstype;

   /* XXX: This isn't well named, these aren't just COW. */

   ASSERT (path);

   fstype = Path_FSType (path);

   switch (fstype) {
   case FS_TYPE_BTRFS:
   case FS_TYPE_NFS:
   case FS_TYPE_ZFS:
   case FS_TYPE_HAMMER:
      return true;
   default:
      return false;
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * Path_Rename --
 *
 *       Renames the file found at @path to @new_path.
 *
 * Returns:
 *       an int functionally the same as the result of rename().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
Path_Rename (const Path *path,     /* IN */
             const Path *new_path) /* IN */
{
   int ret;

   ASSERT (path);
   ASSERT (new_path);

   Task_BeginBlockingCall ();
   ret = rename (path->str, new_path->str);
   Task_EndBlockingCall ();

   return ret;
}


/*
 *--------------------------------------------------------------------------
 *
 * Path_Unlink --
 *
 *       Functionally similar to unlink(path->str)
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
Path_Unlink (const Path *path) /* IN */
{
   int ret;

   ASSERT (path);

   Task_BeginBlockingCall ();
   ret = unlink (path->str);
   Task_EndBlockingCall ();

   return ret;
}

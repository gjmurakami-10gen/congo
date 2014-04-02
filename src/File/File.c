/* File.c
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

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <Debug.h>
#include <File.h>
#include <Log.h>
#include <Task.h>


/*
 *--------------------------------------------------------------------------
 *
 * File_Open --
 *
 *       Wrapper around posix open().
 *
 *       The file will be open using O_NONBLOCK so that other File methods
 *       can provide coroutine semantics.
 *
 * Returns:
 *       FILE_INVALID on failure and errno is set.
 *       Otherwise a valid file descriptor.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

File
File_Open (const char *path, /* IN */
           int flags,        /* IN */
           mode_t mode)      /* IN */
{
   ASSERT (path);

   return open (path, flags | O_NONBLOCK, mode);
}


ssize_t
File_Read (File file,    /* IN */
           void *buffer, /* IN */
           size_t count) /* IN */
{
   ASSERT (file != FILE_INVALID);
   ASSERT (buffer);
   ASSERT (count);

   return Task_Read (file, buffer, count, 0);
}


ssize_t
File_Write (File file,    /* IN */
            void *buffer, /* IN */
            size_t count) /* IN */
{
   ASSERT (file != FILE_INVALID);
   ASSERT (buffer);
   ASSERT (count);

   return Task_Write (file, buffer, count);
}


bool
File_Close (File file) /* IN */
{
   ASSERT (file != FILE_INVALID);

   return (Task_Close (file) == 0);
}


bool
File_Stat (File file,       /* IN */
           struct stat *st) /* OUT */
{
   int ret;

   ASSERT (file != FILE_INVALID);
   ASSERT (st);

   Task_BeginBlockingCall ();
   ret = fstat (file, st);
   Task_EndBlockingCall ();

   return (ret == 0);
}


int
File_Sync (File file) /* IN */
{
   ASSERT (file != FILE_INVALID);

#if defined(F_FULLFSYNC)
   return fcntl (file, F_FULLFSYNC);
#elif defined(HAVE_FDATASYNC)
   return fdatasync (file);
#else
   return fsync (file);
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * File_ZeroFallback --
 *
 *       Write zeroes to a file by manually calling write() with a
 *       zeroed buffer.
 *
 *       This should only be used when none of the other faster methods
 *       are available.
 *
 *       This expects to be called from the thread-pool, meaning that
 *       the task should have already done a Task_BeginBlockingCall().
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       @error may be set.
 *
 *--------------------------------------------------------------------------
 */

static bool
File_ZeroFallback (File fd,  /* IN */
                   size_t size,  /* IN */
                   Error *error) /* IN */
{
   static const char buf[4096] = { 0 };
   ssize_t ret;
   size_t towrite = size;
   bool result = false;

   ASSERT (fd != FILE_INVALID);
   ASSERT (size);

   Task_BeginBlockingCall ();

   if (0 != lseek (fd, 0, SEEK_SET)) {
      Error_Init (error,
                  FILE_ERROR,
                  FILE_ERROR_ZERO,
                  "lseek() failure: %s",
                  strerror (errno));
      goto failure;
   }

   while (towrite) {
      ret = write (fd, buf, MIN (towrite, sizeof buf));
      if ((ret == -1) && ((errno != EAGAIN) || (errno == EINTR))) {
         Error_Init (error,
                     FILE_ERROR,
                     FILE_ERROR_ZERO,
                     "write() failure: %s",
                     strerror (errno));
         goto failure;
      }
      towrite -= ret;
   }

   if (File_Sync (fd) != 0) {
      return false;
   }

   result = true;

failure:
   Task_EndBlockingCall ();

   return result;
}


/*
 *--------------------------------------------------------------------------
 *
 * File_ZeroAllocate --
 *
 *       Zero a file using whatever method available.
 *
 *       @fd is a file descriptor suitable for File.
 *       AsyncFile and File are acceptable.
 *
 *       @size is the target size of the file in bytes.
 *
 *       @sparse means we should try to create a sparse file or fail.
 *       A sparse file means the file-system says the file is of a given
 *       size but does not actually take that many blocks on disk. Reading
 *       from the file past the allocated boundry results in reading
 *       zeroes.
 *
 *       If you are running on a Copy-on-Write file-system such as ZFS,
 *       or Btrfs, you probably want sparse files. Additionally if you
 *       are running on NFS, you probably want sparse files.
 *
 *       Local file-systems such as Ext2, Ext3, Ext4, JFS, or XFS all
 *       want to actually zero out the file to increase the chance that
 *       we will get an allocation of contiguous blocks on disk.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       @error may be set.
 *
 *--------------------------------------------------------------------------
 */

bool
File_ZeroAllocate (File fd,      /* IN */
                   size_t size,  /* IN */
                   bool sparse,  /* IN */
                   Error *error) /* OUT */
{
   bool result = false;

   ASSERT (fd != FILE_INVALID);
   ASSERT (size);

   Task_BeginBlockingCall ();

#if defined(HAVE_POSIX_FADVISE)
   if (0 != posix_fadvise (fd, 0, size, POSIX_FADV_DONTNEED)) {
      LOG_WARNING ("posix_fadvise() failure: %s", strerror (errno));
   }
#endif

   if (sparse) {
#if defined(HAVE_FTRUNCATE)
      if (0 == ftruncate (fd, size)) {
         result = true;
         goto finish;
      }
      Error_Init (error,
                  FILE_ERROR,
                  FILE_ERROR_ZERO,
                  "ftruncate() failure: %s",
                  strerror (errno));
      goto finish;
#else
      Error_Init (error,
                  FILE_ERROR,
                  FILE_ERROR_ZERO,
                  "ftruncate() is not supported on this platform. "
                  "Sparse file cannot be created.");
      goto finish;
#endif
   }

#if defined(HAVE_POSIX_FALLOCATE)
   if (0 == posix_fallocate (fd, 0, size)) {
      result = true;
      goto finish;
   }
#elif defined(HAVE_FALLOCATE)
   if (0 == fallocate (fd, 0, 0, size)) {
      result = true;
      goto finish;
   }
#endif

   result = File_ZeroFallback (fd, size, error);

finish:
   Task_EndBlockingCall ();

   return result;
}


/*
 *--------------------------------------------------------------------------
 *
 * File_GetSize --
 *
 *       Convenience function to call stat and return st_size.
 *
 * Returns:
 *       -1 on failure, the size on success.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

ssize_t
File_GetSize (File file) /* IN */
{
   struct stat st;

   ASSERT (file != FILE_INVALID);

   if (File_Stat (file, &st)) {
      return st.st_size;
   }

   return -1;
}

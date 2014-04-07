/* DBFile.c
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

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>

#include <CString.h>
#include <DBFile.h>
#include <Debug.h>
#include <Log.h>
#include <Memory.h>
#include <Task.h>

/*
 *--------------------------------------------------------------------------
 *
 * DBFile --
 *
 *       DBFile is a an abstraction upon a set of numbered files. Each
 *       numbered file is a DBFileVolume.
 *
 *       Pages exist within these volumes.
 *
 *       This layer is not meant to do caching or read-ahead.
 *       That responsibility is the buffer manager who calls into this.
 *
 * TODO:
 *       - I want to play with libaio and O_DIRECT when the underlying
 *         file-system supports this. Basically, Ext3/4, XFS, JFS.
 *       - We need to handle sparse/cow files properly.
 *       - This makes assumptions that each volume is 2gb.
 *       - We will want an interface to read and write more than one page
 *         at a given time.
 *
 *--------------------------------------------------------------------------
 */


/*
 *--------------------------------------------------------------------------
 *
 * DBFile_SuggestVolumeSize --
 *
 *       Suggest the size for a new volume based on configuration and
 *       the size of the previous volume.
 *
 * Returns:
 *       Cheating for now, just INT_MAX.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static size_t
DBFile_SuggestVolumeSize (DBFile *dbfile, /* IN */
                          int volume)     /* IN */
{
   ASSERT (dbfile);

   return INT_MAX;
}


/*
 *--------------------------------------------------------------------------
 *
 * DBFile_CreateVolume --
 *
 *       Creates a new volume on disk. The file will be zero allocated.
 *
 * Returns:
 *       true if successful; false on failure.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static bool
DBFile_CreateVolume (DBFile *dbfile, /* IN */
                     int volume)     /* IN */
{
   size_t size;
   Error error;
   bool sparse;
   File fd;
   Path path;
   Path dest;

   ASSERT (dbfile);
   ASSERT (volume >= 0);

   Path_Copy (&dbfile->directory, &path);
   Path_AppendPrintf (&path, "%s.%u.tmp", dbfile->name, volume);
   fd = Path_Open (&path, O_CREAT | O_RDWR, 0640);
   if (fd == FILE_INVALID) {
      return false;
   }

   size = DBFile_SuggestVolumeSize (dbfile, volume);
   sparse = Path_IsCopyOnWrite (&path);

   if (!File_ZeroAllocate (fd, size, sparse, &error)) {
      LOG_WARNING ("File_ZeroAllocate: %s", error.message);
      File_Close (fd);
      Path_Unlink (&path);
      return false;
   }

   File_Close (fd);

   Path_Copy (&dbfile->directory, &dest);
   Path_AppendPrintf (&dest, "%s.%u", dbfile->name, volume);

   if (0 != Path_Rename (&path, &dest)) {
      Path_Unlink (&path);
      return false;
   }

   return true;
}


/*
 *--------------------------------------------------------------------------
 *
 * DBFile_OpenVolume --
 *
 *       Open a volume for use by the DBFile.
 *
 * Returns:
 *       true on success, false on failure.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static bool
DBFile_OpenVolume (DBFile *dbfile, /* IN */
                   int volume)     /* IN */
{
   const DBFileVolume invalid = { FILE_INVALID, 0 };
   DBFileVolume *vol;
   struct stat st;
   Path path;
   File fd;

   ASSERT (dbfile);
   ASSERT (volume >= 0);

   Path_Copy (&dbfile->directory, &path);
   Path_AppendPrintf (&path, "%s.%u", dbfile->name, volume);

   /*
    * TODO: Not all file-systems will allow us to open with O_DIRECT.
    *       This means we would be double buffered by default. There
    *       might be some reasons to use alternate I/O models here.
    *       (If we actually care, not totally convinced yet).
    */

   fd = Path_Open (&path, O_RDWR | O_DIRECT, 0640);
   if (fd == FILE_INVALID) {
      LOG_WARNING ("Failed to open %s with O_DIRECT", path.str);
      return false;
   }

   if (0 != File_Stat (fd, &st)) {
      File_Close (fd);
      return false;
   }

   while (dbfile->volumes.len <= volume) {
      Array_Append (&dbfile->volumes, invalid);
   }

   vol = &Array_Index (&dbfile->volumes, DBFileVolume, volume);

   vol->fd = fd;
   vol->size = st.st_size;

   return true;
}


/*
 *--------------------------------------------------------------------------
 *
 * DBFile_IsMember --
 *
 *       Check to see if name is a member of this dbfile.
 *
 *       This is done by checking that the name matches the <db.number>
 *       format and does not contain any extra characters.
 *
 * Returns:
 *       true if @name belongs to @dbfile, otherwise false.
 *
 * Side effects:
 *       @volume is set if true is returned.
 *
 *--------------------------------------------------------------------------
 */

bool
DBFile_IsMember (DBFile *dbfile,   /* IN */
                 const char *name, /* IN */
                 int *volume)      /* OUT */
{
   const char *str;
   long int ret;
   char *endptr = NULL;
   char check [128];

   ASSERT (dbfile);
   ASSERT (name);
   ASSERT (volume);

   *volume = -1;

   str = strrchr (name, '.');
   if (!str) {
      return false;
   }

   ret = strtol (str, &endptr, 10);
   if (*endptr) {
      return false;
   }

   if (((ret == 0) && (errno == EINVAL)) ||
       ((ret == LONG_MIN) && ((errno == ERANGE))) ||
       ((ret == LONG_MAX) && ((errno == ERANGE)))) {
      return false;
   }

   snprintf (check, sizeof check, "%s.%u", dbfile->name, (int)ret);
   check [sizeof check - 1] = '\0';

   if (0 == strcmp (check, name)) {
      *volume = ret;
      return true;
   }

   return false;
}


/*
 *--------------------------------------------------------------------------
 *
 * DBFile_Init --
 *
 *       Initialize the DBFile structure.
 *
 *       A DBFile is a set of numbered files on disk. SO reading from
 *       a particular PageNumber might read from a given numbered
 *       file backing it.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @dbfile is initialized.
 *
 *--------------------------------------------------------------------------
 */

void
DBFile_Init (DBFile *dbfile,        /* IN */
             const Path *directory, /* IN */
             const char *name,      /* IN */
             int flags)             /* IN */
{
   struct dirent *result;
   struct dirent entry;
   DIR *dir;
   int volume;

   ASSERT (dbfile);
   ASSERT (directory);
   ASSERT (name);

   Memory_Zero (dbfile, sizeof *dbfile);

   /*
    * Set defaults.
    */
   Path_Copy (directory, &dbfile->directory);
   CString_Copy (name, dbfile->name, sizeof dbfile->name);
   dbfile->flags = flags;
   Array_Init (&dbfile->volumes, sizeof (DBFileVolume), false);

   /*
    * Open all discovered volumes.
    */
   if ((dir = opendir (name))) {
      while ((0 == readdir_r (dir, &entry, &result)) && result) {
         if (DBFile_IsMember (dbfile, result->d_name, &volume)) {
            DBFile_OpenVolume (dbfile, volume);
         }
      }
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * DBFile_GetVolume --
 *
 *       Retrieves a volume from the DBFile. A volume is a single numbered
 *       file backing the set of files that is a DBFile.
 *
 * Returns:
 *       NULL on failure, or a DBFileVolume.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static DBFileVolume *
DBFile_GetVolume (DBFile *dbfile, /* IN */
                  int volume)     /* IN */
{
   DBFileVolume *ret;

   ASSERT (dbfile);
   ASSERT (volume >= 0);

   if (UNLIKELY (volume <= dbfile->volumes.len)) {
      if (!DBFile_CreateVolume (dbfile, volume)) {
         return NULL;
      }
   }

   ret = &Array_Index (&dbfile->volumes, DBFileVolume, volume);

   return ret;
}


/*
 *--------------------------------------------------------------------------
 *
 * DBFile_GetPageNumberVolumeAndOffset --
 *
 *       This function should convert a PageNumber to the volume number
 *       and the offset within that volume.
 *
 *       This will likely need to be optimized since it will be hit
 *       a lot. If we track sizes for files as we load them, we can
 *       probably binary search based on a cached offset for where
 *       each volume starts.
 *
 * TODO:
 *       Don't assume 2Gb files.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @volume is set.
 *       @offset is set.
 *
 *--------------------------------------------------------------------------
 */

static void
DBFile_GetPageNumberVolumeAndOffset (DBFile *dbfile,     /* IN */
                                     PageNumber pagenum, /* IN */
                                     int32_t *volume,    /* OUT */
                                     int32_t *offset)    /* OUT */
{
   ASSERT (dbfile);
   ASSERT (volume);
   ASSERT (offset);

   /*
    * TODO: Don't assume we have 2Gb files for every one.
    */

   *volume = ((size_t)pagenum) / INT_MAX;
   *offset = ((size_t)pagenum) % INT_MAX;
}


/*
 *--------------------------------------------------------------------------
 *
 * DBFile_ReadPage --
 *
 *       Read a single page determined by PageNumber from the underlying
 *       storage.
 *
 * Returns:
 *       true if successful; otherwise false.
 *
 * Side effects:
 *       @page is initialized with the contents, if true is returned.
 *
 *--------------------------------------------------------------------------
 */

bool
DBFile_ReadPage (DBFile *dbfile,     /* IN */
                 PageNumber pagenum, /* IN */
                 Page *page)         /* OUT */
{
   DBFileVolume *vol;
   int32_t volume = -1;
   int32_t offset = -1;
   ssize_t ret;

   ASSERT (dbfile);
   ASSERT (page);
   ASSERT (((size_t)page % PAGE_SIZE) == 0);

   /*
    * Convert our page number to a location in a volume.
    */
   DBFile_GetPageNumberVolumeAndOffset (dbfile, pagenum, &volume, &offset);
   ASSERT (volume > -1);
   ASSERT (offset > -1);

   /*
    * Get access to the volume, possibly opening or creating it on disk.
    */
   vol = DBFile_GetVolume (dbfile, volume);
   if (!vol) {
      return NULL;
   }

   /*
    * TODO: use libaio with O_DIRECT and block aligned pages. this will
    *       require libaio, io_submit(), and io_getevents() into lthread.
    *       Buf of course, not all file systems will support this. So
    *       will need to be a bit careful.
    */

   Task_BeginBlockingCall ();
   ret = pread (vol->fd, page->data, PAGE_SIZE, offset);
   Task_EndBlockingCall ();

   return (ret == PAGE_SIZE);
}


/*
 *--------------------------------------------------------------------------
 *
 * DBFile_WritePage --
 *
 *       Write @page back to disk.
 *
 * Returns:
 *       true on success, otherwise false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

bool
DBFile_WritePage (DBFile *dbfile,     /* IN */
                  PageNumber pagenum, /* IN */
                  const Page *page)   /* IN */
{
   DBFileVolume *vol;
   int32_t volume = -1;
   int32_t offset = -1;
   ssize_t ret;

   ASSERT (dbfile);
   ASSERT (page);
   ASSERT (((size_t)page % PAGE_SIZE) == 0);

   /*
    * Convert our page number to a location in a volume.
    */
   DBFile_GetPageNumberVolumeAndOffset (dbfile, pagenum, &volume, &offset);
   ASSERT (volume > -1);
   ASSERT (offset > -1);

   /*
    * Get access to the volume, possibly opening or creating it on disk.
    */
   vol = DBFile_GetVolume (dbfile, volume);
   if (!vol) {
      return NULL;
   }

   /*
    * TODO: use libaio.
    */

   Task_BeginBlockingCall ();
   ret = pwrite (vol->fd, page->data, PAGE_SIZE, offset);
   Task_EndBlockingCall ();

   return (ret == PAGE_SIZE);
}


void
DBFile_Destroy (DBFile *dbfile) /* IN */
{
   DBFileVolume *vol;
   int i;

   ASSERT (dbfile);

   for (i = 0; i < dbfile->volumes.len; i++) {
      vol = &Array_Index (&dbfile->volumes, DBFileVolume, i);

      if (vol->fd != FILE_INVALID) {
         File_Close (vol->fd);
         vol->fd = FILE_INVALID;
      }
   }

   Array_Destroy (&dbfile->volumes);
}

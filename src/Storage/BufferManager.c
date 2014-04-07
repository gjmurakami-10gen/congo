/* BufferManager.c
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

#include <BufferManager.h>
#include <Debug.h>
#include <Log.h>
#include <Memory.h>


/*
 *--------------------------------------------------------------------------
 *
 * BufferManager --
 *
 *       Currently, this does no buffering and is just a passthrough to
 *       the I/O layer.
 *
 * TODO:
 *       Everything.
 *
 *--------------------------------------------------------------------------
 */


void
BufferManager_Init (BufferManager *bufmgr, /* OUT */
                    DBFile *dbfile)        /* IN */
{
   ASSERT (bufmgr);
   ASSERT (dbfile);

   Memory_Zero (bufmgr, sizeof *bufmgr);

   bufmgr->dbfile = dbfile;
}


bool
BufferManager_FlushPage (BufferManager *bufmgr,
                         PageNumber pagenum,
                         const Page *page)
{
   ASSERT (bufmgr);
   ASSERT (pagenum >= 0);
   ASSERT (page);

   return DBFile_WritePage (bufmgr->dbfile, pagenum, page);
}


bool
BufferManager_GetAndPinPage (BufferManager *bufmgr,
                             PageNumber pagenum,
                             Page *page)
{
   ASSERT (bufmgr);
   ASSERT (pagenum >= 0);
   ASSERT (page);

   return DBFile_ReadPage (bufmgr->dbfile, pagenum, page);
}


void
BufferManager_ReleasePage (BufferManager *bufmgr,
                           PageNumber pagenum)
{
   ASSERT (bufmgr);
   ASSERT (pagenum >= 0);
}


void
BufferManager_Destroy (BufferManager *bufmgr)
{
   ASSERT (bufmgr);

   bufmgr->dbfile = NULL;
}

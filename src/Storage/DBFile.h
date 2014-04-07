/* DBFile.h
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


#ifndef DB_FILE_H
#define DB_FILE_H


#include <Array.h>
#include <Macros.h>
#include <File.h>
#include <Page.h>
#include <PageNumber.h>
#include <Path.h>
#include <Types.h>


BEGIN_DECLS


typedef struct
{
   Path  directory;
   char  name [256];
   int   flags;
   Array volumes;
} DBFile;


typedef struct
{
   File     fd;
   uint32_t size;
} DBFileVolume;


void DBFile_Init      (DBFile *dbfile,
                       const Path *directory,
                       const char *name,
                       int flags);
bool DBFile_ReadPage  (DBFile *dbfile,
                       PageNumber pagenum,
                       Page *page);
bool DBFile_WritePage (DBFile *dbfile,
                       PageNumber pagenum,
                       const Page *page);
void DBFile_Destroy   (DBFile *dbfile);


END_DECLS


#endif /* DB_FILE_H */

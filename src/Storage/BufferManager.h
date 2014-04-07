/* BufferManager.h
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


#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H


#include <DBFile.h>
#include <Macros.h>
#include <Page.h>
#include <PageNumber.h>
#include <Types.h>


BEGIN_DECLS


typedef struct
{
   DBFile *dbfile;
} BufferManager;


void BufferManager_Init          (BufferManager *bufmgr,
                                  DBFile *dbfile);
bool BufferManager_FlushPage     (BufferManager *bufmgr,
                                  PageNumber pagenum,
                                  const Page *page);
bool BufferManager_GetAndPinPage (BufferManager *bufmgr,
                                  PageNumber pagenum,
                                  Page *page);
void BufferManager_ReleasePage   (BufferManager *bufmgr,
                                  PageNumber pagenum);
void BufferManager_Destroy       (BufferManager *bufmgr);


END_DECLS


#endif /* BUFFER_MANAGER_H */

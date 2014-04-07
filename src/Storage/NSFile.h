/* NSFile.h
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

#ifndef NS_FILE_H
#define NS_FILE_H


#include <Macros.h>
#include <Path.h>
#include <Types.h>


BEGIN_DECLS


typedef struct
{
   File   fd;
   Path   path;
   size_t size;
} NSFile;


typedef struct
{
   void *dummy;
} NSFileIter;


void NSFile_Init     (NSFile *nsfile,
                      const Path *path,
                      int flags);
void NSFile_Destroy  (NSFile *nsfile);
void NSFileIter_Init (NSFileIter *iter,
                      NSFile *nsfile);
bool NSFileIter_Next (NSFileIter *iter);


END_DECLS


#endif /* NS_FILE_H */

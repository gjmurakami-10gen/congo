/* congo-dump-freelist.c
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

#include <stdlib.h>

#include <BufferManager.h>
#include <DBFile.h>
#include <Page.h>
#include <Path.h>


int
main (int argc,     /* IN */
      char *argv[]) /* IN */
{
   BufferManager bufmgr;
   const char *name = "test";
   Path dbpath;
   DBFile dbfile;

   Path_Build (&dbpath, ".", NULL);

   DBFile_Init (&dbfile, &dbpath, name, 0);
   BufferManager_Init (&bufmgr, &dbfile);

   /*
    * TODO: get access to .ns for finding beginning extent.
    */

   BufferManager_Destroy (&bufmgr);
   DBFile_Destroy (&dbfile);

   return EXIT_SUCCESS;
}

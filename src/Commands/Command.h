/* Command.h
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


#ifndef COMMAND_H
#define COMMAND_H


#include <Error.h>
#include <Macros.h>
#include <Types.h>


BEGIN_DECLS


typedef struct _Command Command;


struct _Command
{
   void (*Destroy) (Command *command);
   void (*Log)     (Command *command);
   bool (*Run)     (Command *command,
                    Error *error);
};


#define Command_Destroy(c) (((Command *)c)->Destroy((Command *)(c)))
#define Command_Log(c)     (((Command *)c)->Log((Command *)(c)))
#define Command_Run(c,e)   (((Command *)c)->Run((Command *)(c), (e)))


END_DECLS


#endif /* COMMAND_H */

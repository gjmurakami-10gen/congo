/* Tunable.h
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


#ifndef TUNABLE_H
#define TUNABLE_H


#include <Macros.h>
#include <Value.h>


BEGIN_DECLS


typedef int Tunable;


#define TUNABLE_INVALID (-1)


Tunable Tunable_Register (const char *key,
                          const Value *curval);
Tunable Tunable_Find     (const char *key);
void    Tunable_Set      (Tunable tunable,
                          const Value *value);
void    Tunable_Get      (Tunable tunable,
                          Value *value);


END_DECLS


#endif /* TUNABLE_H */
